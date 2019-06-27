/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <boost/algorithm/string.hpp>

#include "force.token.hpp"
#include "sys.match/sys.match.hpp"

#include "sys.match/match_defines.hpp"

namespace eosio {

void token::create( account_name issuer,
                    asset        maximum_supply ) {
    require_auth( issuer );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");
    eosio_assert( sym != CORE_SYMBOL, "not allow create core symbol token by token contract");

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    eosio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}


void token::issue( account_name to, asset quantity, std::string memo ) {
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    eosio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, 0, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    }
}

void token::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      std::string  memo ) {
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    sub_balance( from, quantity );
    add_balance( to, quantity, from );
}

inline uint32_t calc_castfinish_block_num( const uint32_t current_block_num ) {
   const auto weaken_num =  static_cast<uint32_t>( static_cast<double>(current_block_num) / WEAKEN_CAST_NUM );
   // if PRE_CAST_NUM - weaken_num < STABLE_CAST_NUM
   // mean weaken_num > PRE_CAST_NUM - STABLE_CAST_NUM
   static_assert( PRE_CAST_NUM >= STABLE_CAST_NUM, "PRE_CAST_NUM >= STABLE_CAST_NUM" );

   auto cast_num = STABLE_CAST_NUM;
   if ( weaken_num <= ( PRE_CAST_NUM - STABLE_CAST_NUM ) ) {
      cast_num = PRE_CAST_NUM - weaken_num;
   }

   return current_block_num + cast_num;
}

void token::castcoin( account_name from,
                      account_name to,
                      asset        quantity )
{
   eosio_assert( from == ::config::reward_account_name, "only the account force.reward can cast coin to others" );
   require_auth( from );

   eosio_assert( is_account( to ), "to account does not exist");
   coincasts coincast_table( _self, to );

   const auto finish_block = calc_castfinish_block_num( current_block_num() );
   const auto cc = coincast_table.find( static_cast<uint64_t>(finish_block) );

   require_recipient( from );
   require_recipient( to );

   eosio_assert( quantity.is_valid(), "invalid quantity" );
   eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
   eosio_assert( cc != coincast_table.end(), "the cast is not opened" );
   eosio_assert( quantity.symbol == cc->balance.symbol, "symbol precision mismatch" );

   sub_balance( from, quantity );

   coincast_table.modify( cc, to, [&]( auto& a ) {
      a.balance += quantity;
   });
}

void token::takecoin( account_name to ) {
   require_auth( to );
   coincasts coincast_table( _self, to );

   const auto current_block = current_block_num();

   vector<uint32_t> finish_block;
   asset finish_coin = asset{0};

   finish_block.reserve(128);
   for( const auto& coincast : coincast_table ) {
      if( coincast.finish_block < current_block ) {
         finish_block.push_back(coincast.finish_block);
         finish_coin += coincast.balance;
      }
   }

   add_balance( to, finish_coin, to );

   for( const auto& val : finish_block ) {
      const auto cc = coincast_table.find( static_cast<uint64_t>(val) );
      if( cc != coincast_table.end() ) {
         coincast_table.erase(cc);
      }
   }
}

void token::opencast(account_name to) {
   require_auth( to );

   eosio_assert( is_account( to ), "to account does not exist");
   coincasts coincast_table( _self, to );

   const auto finish_block = calc_castfinish_block_num( current_block_num() );
   const auto cc = coincast_table.find( static_cast<uint64_t>(finish_block) );

   eosio_assert(cc == coincast_table.end(),"the cast is been opened");
   coincast_table.emplace( to, [&]( auto& a ){
         a.balance      = asset{0};
         a.finish_block = finish_block;
      });
}

void token::closecast( account_name to, int32_t finish_block ) {
   require_auth( to );

   eosio_assert( is_account( to ), "to account does not exist");
   coincasts coincast_table( _self, to );
   const auto cc = coincast_table.find( static_cast<uint64_t>(finish_block) );
   eosio_assert(cc != coincast_table.end(), "the cast is not exist");
   eosio_assert(cc->balance == asset(0), "the cast can not be closed");

   coincast_table.erase(cc);
}

void token::fee( account_name payer, asset quantity ){
   require_auth( payer );

   auto sym = quantity.symbol.name();
   stats statstable( _self, sym );
   const auto& st = statstable.get( sym );

   eosio_assert( quantity.is_valid(), "invalid quantity" );
   eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
   eosio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

   sub_balance( payer, quantity );
   add_balance( config::fee_account_name, quantity, payer );
}

void token::sub_balance( account_name owner, asset value ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name(), "no balance object found" );
   eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
   } else {
      from_acnts.modify( from, owner, [&]( auto& a ) {
          a.balance -= value;
      });
   }
}

void token::add_balance( account_name owner, asset value, account_name ram_payer )
{
   accounts to_acnts( _self, owner );
   
   auto to = to_acnts.find( value.symbol.name() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, 0, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void token::trade(account_name from,
                  account_name to,
                  asset quantity,
                  codex::trade::func_typ type,
                  std::string memo ) {
   constexpr auto chain_name = name{ N(self) };
   switch( type ) {
      case codex::trade::func_typ::bridge_addmortgage : {
         transfer( from, to, quantity, memo );
         eosio_assert( to == config::bridge_account_name, "to account should be bridge account" );
         codex::trade::sys_bridge_addmort{ memo }.done( chain_name, from, quantity );
         break;
      }
      case codex::trade::func_typ::bridge_exchange : {
         transfer( from, to, quantity, memo );
         eosio_assert( to == config::bridge_account_name, "to account should be bridge account" );
         codex::trade::sys_bridge_exchange{ memo }.done( chain_name, from, quantity );
         break;
      }
      case codex::trade::func_typ::match : {
         eosio_assert( to == config::match_account_name, "to account should be match account" );
         SEND_INLINE_ACTION(*this, transfer, { from, N(active) }, { from, to, quantity, memo });
         break;
      }
      default:
         eosio_assert(false, "invalid trade type");
   }
}

} /// namespace eosio

EOSIO_ABI( eosio::token, (create)(issue)(transfer)(fee)(trade)(castcoin)(takecoin)(opencast)(closecast) )
