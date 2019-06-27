#pragma once

#include <string>

#include <bits/stdint.h>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#include "sys.match/match_defines.hpp"

namespace eosio {
   #ifdef BEFORE_ONLINE_TEST   
   static constexpr uint32_t PRE_CAST_NUM = 28800;
   static constexpr uint32_t STABLE_CAST_NUM = 7200;
   static constexpr double WEAKEN_CAST_NUM = 2.5; // per block
   #else
   static constexpr uint32_t PRE_CAST_NUM = 2592000;
   static constexpr uint32_t STABLE_CAST_NUM = 604800;
   static constexpr double WEAKEN_CAST_NUM = 2.5;
   #endif

   // TODO by CODEREVIEW need unity force.token and relay.token
   class token : public contract {
      public:
         token( account_name self ) : contract( self ) {}
 
         void create( account_name issuer,
                      asset        maximum_supply );

         void issue( account_name to, asset quantity, std::string memo );

         void transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        std::string  memo );

         void fee( account_name payer, asset quantity );

         inline asset get_supply( symbol_name sym ) const;
         inline asset get_balance( account_name owner, symbol_name sym ) const;

         void trade( account_name           from,
                     account_name           to,
                     asset                  quantity,
                     codex::trade::func_typ type,
                     std::string            memo );

         void castcoin( account_name from, account_name to, asset quantity );
         void takecoin( account_name to );

         void opencast( account_name to );
         void closecast( account_name to, int32_t finish_block );

       private:
         struct account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.name(); }
         };

         struct currency_stats {
            asset        supply;
            asset        max_supply;
            account_name issuer;

            uint64_t primary_key()const { return supply.symbol.name(); }
         };

         struct coin_cast {
            asset    balance;
            uint32_t finish_block = 0;

            uint64_t primary_key()const { return static_cast<uint64_t>(finish_block); }
         };

         typedef eosio::multi_index<N(accounts), account> accounts;
         typedef eosio::multi_index<N(stat), currency_stats> stats;
         typedef eosio::multi_index<N(coincast), coin_cast> coincasts;

         void sub_balance( account_name owner, asset value );
         void add_balance( account_name owner, asset value, account_name ram_payer );

      public:
         struct transfer_args {
            account_name  from;
            account_name  to;
            asset         quantity;
            std::string   memo;
         };

         static symbol_type get_symbol( const account_name& contract, const symbol_name& sym ) {
            stats statstable( contract, sym );
            return statstable.get( sym, "symbol no found" ).supply.symbol;
         }
   };

   asset token::get_supply( symbol_name sym ) const {
      stats statstable( _self, sym );
      const auto& st = statstable.get( sym );
      return st.supply;
   }

   asset token::get_balance( account_name owner, symbol_name sym ) const {
      accounts accountstable( _self, owner );
      const auto& ac = accountstable.get( sym );
      return ac.balance;
   }

}

