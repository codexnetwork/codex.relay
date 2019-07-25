
#include <string>
#include <stdlib.h>

#include <boost/algorithm/string.hpp>
#include <eosiolib/action.hpp>
#include <bits/stdint.h>

#include "force.relay/force.relay.hpp"
#include "relay.token.hpp"

#include "sys.match/match_defines.hpp"

namespace relay {

using codex::utils::precision;

// just a test version by contract
void token::on( name chain, const checksum256 block_id, const force::relay::action& act ) {
   require_auth(config::relay_account_name); // TODO use config

   // TODO this ACTION should no err

   const auto data = unpack<token::action>(act.data);
   print("map ", name{ data.from }, " ", data.quantity, " ", data.memo, "\n");

   auto sym = data.quantity.symbol;
   stats statstable(_self, chain);
   auto st = statstable.find(sym.name());
   if( st == statstable.end() ){
      // TODO param err processing
      print("no token err");
      return;
   }

   if(    ( !sym.is_valid() )
       || ( data.memo.size() > 256 )
       || ( !data.quantity.is_valid() )
       || ( data.quantity.amount <= 0 )
       || ( data.quantity.symbol != st->supply.symbol )
       || ( data.quantity.amount > st->max_supply.amount - st->supply.amount )
       ) {
      // TODO param err processing
      print("token err");
      return;
   }

   if( data.memo.empty() || data.memo.size() >= 13 ){
      // TODO param err processing
      print("data.memo err");
      return;
   }
   const auto to = ::eosio::string_to_name(data.memo.c_str());
   if( !is_account(to) ) {
      // TODO param err processing
      print("to is no account");
      return;
   }

   SEND_INLINE_ACTION(*this, issue,
         { chain, N(active) }, { chain, to, data.quantity, "from chain" });
}

void token::create( account_name issuer,
                    name chain,
                    account_name side_account,
                    action_name side_action,
                    asset maximum_supply ) {
   require_auth(chain); // TODO if need

   auto sym = maximum_supply.symbol;
   eosio_assert(sym.is_valid(), "invalid symbol name");
   eosio_assert(maximum_supply.is_valid(), "invalid supply");
   eosio_assert(maximum_supply.amount > 0, "max-supply must be positive");

   stats statstable(_self, chain);
   auto existing = statstable.find(sym.name());
   eosio_assert(existing == statstable.end(), "token with symbol already exists");

   statstable.emplace(_self, [&]( auto& s ) {
      s.supply.symbol = maximum_supply.symbol;
      s.max_supply = maximum_supply;
      s.issuer = issuer;
      s.chain = chain;
      s.side_account = side_account;
      s.side_action = side_action;
      s.total_mineage = 0;
      s.total_mineage_update_height = current_block_num();
      s.reward_scope = asset::max_amount;
      s.reward_size = 0;
      s.coin_weight = BASE_COIN_WEIGHT;
   });
}

void token::issue( name chain, account_name to, asset quantity, std::string memo ) {
   auto sym = quantity.symbol;
   eosio_assert(sym.is_valid(), "invalid symbol name");
   eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

   auto sym_name = sym.name();
   stats statstable(_self, chain);
   auto existing = statstable.find(sym.name());
   eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
   const auto& st = *existing;

   require_auth(st.issuer);

   eosio_assert(quantity.is_valid(), "invalid quantity");
   eosio_assert(quantity.amount > 0, "must issue positive quantity");

   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   eosio_assert(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   const auto current_block = current_block_num();
   statstable.modify(st, 0, [&]( auto& s ) {
      s.total_mineage += ( s.supply.amount * (current_block - s.total_mineage_update_height) );
      s.total_mineage_update_height = current_block;
      s.supply += quantity;
   });

   add_balance(current_block, st.issuer, chain, quantity, st.issuer);

   if( to != st.issuer ) {
      SEND_INLINE_ACTION(*this, transfer, { st.issuer, N(active) }, { st.issuer, to, chain, quantity, memo });
   }
}

void token::destroy( name chain, account_name from, asset quantity, std::string memo ) {
   require_auth(from);

   auto sym = quantity.symbol;
   eosio_assert(sym.is_valid(), "invalid symbol name");
   eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

   auto sym_name = sym.name();
   stats statstable(_self, chain);
   auto existing = statstable.find(sym_name);
   eosio_assert(existing != statstable.end(), "token with symbol does not exist, create token before issue");
   const auto& st = *existing;

   eosio_assert(quantity.is_valid(), "invalid quantity");
   eosio_assert(quantity.amount > 0, "must issue positive quantity");

   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   eosio_assert(quantity.amount <= st.supply.amount, "quantity exceeds available supply");

   const auto curr_block_num = current_block_num();
   statstable.modify(st, 0, [&]( auto& s ) {
      s.total_mineage += s.supply.amount * (curr_block_num - s.total_mineage_update_height);
      s.total_mineage_update_height = curr_block_num;
      s.supply -= quantity;
   });

   sub_balance(curr_block_num, from, chain, quantity);
}

void token::transfer( account_name from,
                      account_name to,
                      name chain,
                      asset quantity,
                      std::string memo ) {
   eosio_assert(from != to, "cannot transfer to self");
   require_auth(from);
   eosio_assert(is_account(to), "to account does not exist");

   auto sym = quantity.symbol.name();
   stats statstable(_self, chain);
   const auto& st = statstable.get(sym);

   const auto curr_block_num = current_block_num();

   require_recipient(from);
   require_recipient(to);

   eosio_assert(quantity.is_valid(), "invalid quantity");
   eosio_assert(quantity.amount > 0, "must transfer positive quantity");
   eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
   eosio_assert(chain == st.chain, "symbol chain mismatch");
   eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

   sub_balance(curr_block_num, from, chain, quantity);
   add_balance(curr_block_num, to, chain, quantity, from);
}

void token::sub_balance( uint32_t curr_block_num, account_name owner, name chain, const asset& value ) {
   settle_user( curr_block_num, owner, chain, value );
   accounts from_acnts( _self, owner );
   auto idx = from_acnts.get_index<N( bychain )>();
   const auto from = idx.get( get_account_idx( chain, value ), "no balance object found" );

   eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );
   eosio_assert( from.chain == chain, "symbol chain mismatch" );

   from_acnts.modify( from, owner, [&]( auto& a ) { 
      a.balance -= value; 
   } );
}

void token::add_balance( uint32_t curr_block_num, account_name owner, name chain, const asset& value, account_name ram_payer ) {
   accounts to_acnts( _self, owner );
   account_next_ids acntids( _self, owner );

   auto idx = to_acnts.get_index<N( bychain )>();

   auto to = idx.find( get_account_idx( chain, value ) );
   if( to == idx.end() ) {
      uint64_t id = 1;
      const auto ids = acntids.find( owner );
      if( ids == acntids.end() ) {
         acntids.emplace( ram_payer, [&]( auto& a ) {
            a.id = 2;
            a.account = owner;
         } );
      } else {
         id = ids->id;
         acntids.modify( ids, 0, [&]( auto& a ) { 
            a.id++;
         } );
      }

      to_acnts.emplace( ram_payer, [&]( auto& a ) {
         a.id = id;
         a.balance = value;
         a.chain = chain;
         a.mineage = 0;
         a.mineage_update_height = curr_block_num;
      } );
   } else {
      settle_user( curr_block_num, owner, chain, value );
      accounts to_acnts_temp( _self, owner );
      idx = to_acnts_temp.get_index<N( bychain )>();
      to = idx.find( get_account_idx( chain, value ) );
      idx.modify( to, 0, [&]( auto& a ) { 
         a.balance += value; 
      } );
   }
}

void token::trade( account_name from,
                   account_name to,
                   name chain,
                   asset quantity,
                   codex::trade::func_typ type,
                   std::string memo ) {
   switch( type ) {
      case codex::trade::func_typ::bridge_addmortgage : {
         transfer( from, to, chain, quantity, memo );
         eosio_assert( to == config::bridge_account_name, "to account should be bridge account" );
         codex::trade::sys_bridge_addmort{ memo }.done( chain, from, quantity );
         break;
      }
      case codex::trade::func_typ::bridge_exchange : {
         transfer( from, to, chain, quantity, memo );
         eosio_assert( to == config::bridge_account_name, "to account should be bridge account" );
         codex::trade::sys_bridge_exchange{ memo }.done( chain, from, quantity );
         break;
      }
      case codex::trade::func_typ::match : {
         eosio_assert( to == config::match_account_name, "to account should be match account" );
         SEND_INLINE_ACTION(*this, transfer, { from, N(active) }, { from, to, chain, quantity, memo });
         break;
      }
      default:
         eosio_assert(false, "invalid trade type");
   }
}

void token::addreward(name chain,asset supply,int32_t reward_now) {
   require_auth( config::system_account_name );

   const auto sym = supply.symbol;
   eosio_assert( sym.is_valid(), "invalid symbol name" );

   stats statstable( _self, chain );
   auto existing = statstable.find( supply.symbol.name() );
   eosio_assert( existing != statstable.end(), "token with symbol do not exists" );

   rewards rewardtable( _self, _self );
   auto idx = rewardtable.get_index<N( bychain )>();
   auto con = idx.find( get_account_idx( chain, supply ) );

   eosio_assert( con == idx.end(), "token with symbol already exists" );

   auto reward_id = rewardtable.available_primary_key();
   rewardtable.emplace( _self, [&]( auto& s ) {
      s.id         = reward_id;
      s.supply     = supply;
      s.chain      = chain;
      s.reward_now = ( reward_now == 1 );
   } );

   statstable.modify( *existing, 0, [&]( auto& s ) {
      s.reward_scope = reward_id;
      s.reward_size  = 1;
   } );

   const auto current_block = current_block_num();
   reward_mine reward_inf( _self, reward_id );
   auto reward_info = reward_inf.find( current_block );
   eosio_assert( reward_info == reward_inf.end(), "the reward info already exist" );

   reward_inf.emplace( _self, [&]( auto& s ) {
      s.total_mineage    = 0;
      s.reward_pool      = asset( 0 );
      s.reward_block_num = current_block;
   } );
}

void token::rewardmine(asset quantity) {
   require_auth( config::system_account_name );

   const auto current_block = current_block_num();

   rewards rewardtable( _self, _self );
   exchange::exchange t( config::match_account_name );

   uint128_t total_power = 0;
   for( const auto& reward : rewardtable ) {
      stats statstable( _self, reward.chain );
      const auto existing = statstable.find( reward.supply.symbol.name() );
      eosio_assert( existing != statstable.end(), "token with symbol already exists" );
      const auto price =
        t.get_avg_price( current_block, existing->chain, existing->supply.symbol ).amount;
   
      reward_mine reward_inf( _self, existing->reward_scope );
      const auto reward_info = reward_inf.find( current_block );

      // TODO: Need a more clear calculation process
      total_power += ( reward_info->total_mineage * price /
                       precision( existing->supply.symbol.precision() ) * existing->coin_weight /
                       BASE_COIN_WEIGHT );
   }

   if( total_power == 0 ) {
      return;
   }

   for( const auto& reward : rewardtable ) {
      stats statstable( _self, reward.chain );
      const auto existing = statstable.find( reward.supply.symbol.name() );
      eosio_assert( existing != statstable.end(), "token with symbol do not exists" );
      const auto price =
        t.get_avg_price( current_block, existing->chain, existing->supply.symbol ).amount;

      reward_mine reward_inf( _self, existing->reward_scope );
      const auto reward_info = reward_inf.find( current_block );

      // FIXME: This will error
      const auto devide_amount = reward_info->total_mineage * price /
                                precision( existing->supply.symbol.precision() ) * quantity.amount /
                                total_power * existing->coin_weight / BASE_COIN_WEIGHT;
      reward_inf.modify( reward_info, 0, [&]( auto& s ) { 
           s.reward_pool = asset( devide_amount ); 
      } );
   }
}

void token::setweight(name chain,asset coin,uint32_t weight) {
   require_auth( config::system_account_name );
   stats statstable( _self, chain );
   auto existing = statstable.find( coin.symbol.name() );
   eosio_assert( existing != statstable.end(), "token with symbol do not exists" );
   statstable.modify( *existing, 0, [&]( auto& s ) { 
      s.coin_weight = weight; 
   } );
}

void token::settlemine( account_name system_account ) {
   require_auth( config::system_account_name );
   const auto current_block = current_block_num();

   rewards rewardtable( _self, _self );

   for( const auto& reward : rewardtable ) {
      stats statstable( _self, reward.chain );
      const auto existing = statstable.find( reward.supply.symbol.name() );
      if( existing == statstable.end() ) {
         continue;
      }

      reward_mine reward_inf( _self, existing->reward_scope );
      const auto reward_info = reward_inf.find( current_block );
      eosio_assert( reward_info == reward_inf.end(), "reward info already exist" );

      reward_inf.emplace( _self, [&]( auto& s ) {
         s.total_mineage =
           existing->total_mineage 
           + ( static_cast<int128_t>( existing->supply.amount ) 
               * ( current_block - existing->total_mineage_update_height ) );
         s.reward_pool = asset{0};
         s.reward_block_num = current_block;
      } );

      if( existing->reward_size == COIN_REWARD_RECORD_SIZE ) {
         auto reward_second = ++(reward_inf.begin());
         reward_inf.modify( *reward_second, 0, [&]( auto& s ) { 
            s.reward_pool = asset{0};
         } );

         reward_inf.erase( reward_inf.begin() );
      }

      statstable.modify( *existing, 0, [&]( auto& s ) {
         s.total_mineage = 0;
         s.total_mineage_update_height = current_block;
         if( s.reward_size != COIN_REWARD_RECORD_SIZE ) {
            s.reward_size += 1;
         }
      } );
   }
}

void token::activemine( account_name system_account ) {
   require_auth( config::system_account_name );
   rewards rewardtable( _self, _self );
   for( const auto& reward : rewardtable ) {
      rewardtable.modify( reward, 0, [&]( auto& s ) { 
         s.reward_now = true; 
      } );
   }
}

void token::claim( name chain, asset quantity, account_name receiver ) {
   require_auth( receiver );
   auto sym = quantity.symbol;
   eosio_assert( sym.is_valid(), "invalid symbol name" );

   const auto current_block = current_block_num();
   const auto account_idx   = get_account_idx( chain, quantity );

   rewards rewardtable( _self, _self );
   auto idx = rewardtable.get_index<N( bychain )>();
   auto con = idx.find( account_idx );
   eosio_assert( con != idx.end(), "token with symbol donot participate in dividends " );

   stats statstable( _self, chain );
   auto existing = statstable.find( quantity.symbol.name() );
   eosio_assert( existing != statstable.end(), "token with symbol already exists" );

   settle_user( current_block, receiver, chain, quantity );

   accounts to_acnts( _self, receiver );
   auto idx_to_acnts = to_acnts.get_index<N( bychain )>();
   const auto& to = idx_to_acnts.get( account_idx, "no balance object found" );
   eosio_assert( to.chain == chain, "symbol chain mismatch" );

   const auto total_reward = to.reward;

   to_acnts.modify( to, receiver, [&]( auto& a ) { a.reward = asset( 0 ); } );

   eosio_assert( total_reward > asset( 10 * 10000 ), "claim amount must > 10" );
   eosio::action( { { config::reward_account_name, N( active ) } },
                  config::token_account_name,
                  N( castcoin ),
                  std::make_tuple( config::reward_account_name, receiver, total_reward ) )
     .send();
}

void token::settle_user( uint32_t curr_block_num, account_name owner, name chain, const asset& value ) {
   accounts from_acnts(_self, owner);
   auto idx = from_acnts.get_index<N(bychain)>();
   const auto& from = idx.get(get_account_idx(chain, value), "no balance object found");
   eosio_assert(from.chain == chain, "symbol chain mismatch");

   stats statstable(_self, chain);
   auto existing = statstable.find(value.symbol.name());
   eosio_assert(existing != statstable.end(), "settle wrong can not find stat");

   reward_mine reward_inf(_self,existing->reward_scope);
   auto last_update_height = from.mineage_update_height;
   auto last_mineage = from.mineage;
   auto total_reward = asset(0);
   bool cross_day = false;
   for(auto it = reward_inf.begin();it != reward_inf.end();++it) {
      if (last_update_height < it->reward_block_num && it->total_mineage > 0) {
         auto mineage = last_mineage + from.balance.amount * (it->reward_block_num - last_update_height);
         auto reward = asset(static_cast<int128_t>(it->reward_pool.amount) * mineage / it->total_mineage);

         total_reward += reward;
         reward_inf.modify(*it,0,[&]( auto& s ) {
            s.total_mineage -= mineage;
            s.reward_pool -= reward;
         });
         last_update_height = it->reward_block_num;
         last_mineage = 0;
         cross_day = true;
      }
   }

   from_acnts.modify( from, 0, [&]( auto& a ) {
      a.reward += total_reward;
      const auto mineage_add = a.balance.amount * ( curr_block_num - last_update_height );
      if( cross_day ) {
         a.mineage = mineage_add;
      } else {
         a.mineage += mineage_add;
      }

      a.mineage_update_height = curr_block_num;
   } );
}

};

EOSIO_ABI( relay::token, 
   (on)(create)(issue)(destroy)(transfer)
   (trade)(rewardmine)(addreward)(claim)
   (settlemine)(activemine)(setweight) )
