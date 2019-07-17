//
// Created by fy on 2019-01-29.
//

#ifndef FORCEIO_CONTRACT_RELAY_TOKEN_HPP
#define FORCEIO_CONTRACT_RELAY_TOKEN_HPP

#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#include "force.relay/force.relay.hpp"
#include "sys.match/sys.match.hpp"

#include "force.system/codex_defines.hpp"
#include "sys.match/match_defines.hpp"

namespace relay {

   using namespace eosio;

   static constexpr int32_t  COIN_REWARD_RECORD_SIZE = 360;
   static constexpr uint32_t BASE_COIN_WEIGHT        = 10000;

   class token : public eosio::contract {
   public:
      using contract::contract;

      token( account_name self ) : contract(self) {}

      struct action {
         account_name from;
         account_name to;
         asset quantity;
         std::string memo;

         EOSLIB_SERIALIZE( action, ( from )( to )( quantity )( memo ) )
      };

      /// @abi action
      void on( name chain, const checksum256 block_id, const force::relay::action& act );

      /// @abi action
      void create( account_name issuer,
                   name chain,
                   account_name side_account,
                   action_name side_action,
                   asset maximum_supply );

      /// @abi action
      void issue( name chain, account_name to, asset quantity, std::string memo );

      /// @abi action
      void destroy( name chain, account_name from, asset quantity, std::string memo );

      /// @abi action
      void transfer( account_name from,
                     account_name to,
                     name chain,
                     asset quantity,
                     std::string memo );

      inline asset get_supply( name chain, symbol_name sym ) const;
      inline asset get_current_balance( account_name owner, name chain, const asset& destcoin ) const;

      /// @abi action
      void trade( account_name from,
                  account_name to,
                  name chain,
                  asset quantity,
                  codex::trade::func_typ type,
                  std::string memo);

      /// @abi action
      void addreward( name chain, asset supply, int32_t reward_now );
      /// @abi action
      void rewardmine( asset quantity );
      /// @abi action
      void claim( name chain, asset quantity, account_name receiver );
      /// @abi action
      void settlemine( account_name system_account );
      /// @abi action
      void activemine( account_name system_account );
      /// @abi action
      void setweight( name chain, asset coin, uint32_t weight );

    private:
      inline static uint128_t get_account_idx( const name& chain, const asset& a ) {
         return ( uint128_t( uint64_t( chain ) ) << 64 ) + uint128_t( a.symbol.name() );
      }

      struct account {
         uint64_t id;
         asset    balance;
         name     chain;
         int128_t mineage               = 0;  // asset.amount * block height
         uint32_t mineage_update_height = 0;
         asset    reward;

         uint64_t  primary_key() const { return id; }
         uint128_t get_index_i128() const { return get_account_idx(chain, balance); }
      };

      struct account_next_id {
         uint64_t     id;
         account_name account;

         uint64_t  primary_key() const { return account; }
      };

      struct reward_mine_info {
         int128_t total_mineage    = 0;
         asset    reward_pool;
         uint32_t reward_block_num = 0;

         uint64_t primary_key() const { return static_cast<uint64_t>(reward_block_num); }
      };

      struct currency_stats {
         asset        supply;
         asset        max_supply;
         account_name issuer;
         name         chain;

         account_name side_account;
         action_name  side_action;

         int128_t     total_mineage               = 0; // asset.amount * block height
         uint32_t     total_mineage_update_height = 0;
         uint64_t     reward_scope;
         int32_t      reward_size = 0;
         uint32_t     coin_weight = 10000;

         uint64_t primary_key() const { return supply.symbol.name(); }
      };
      struct reward_currency {
         uint64_t     id;
         name         chain;
         asset        supply;
         bool         reward_now = true;

         uint64_t primary_key() const { return id; }
         uint128_t get_index_i128() const { return get_account_idx(chain, supply); }
      };

      typedef multi_index<N(accounts), account,
            indexed_by< N(bychain), const_mem_fun<account, uint128_t, &account::get_index_i128 >>> accounts;
      typedef multi_index<N(stat), currency_stats> stats;
      typedef multi_index<N(accountid), account_next_id> account_next_ids ;
      typedef multi_index<N(reward), reward_currency,
         indexed_by< N(bychain), const_mem_fun<reward_currency, uint128_t, &reward_currency::get_index_i128 >>> rewards;
      typedef multi_index<N(minereward), reward_mine_info> reward_mine ;
      

      void sub_balance( uint32_t curr_block_num, account_name owner, name chain, const asset& value );
      void add_balance( uint32_t curr_block_num, account_name owner, name chain, const asset& value, account_name ram_payer );
      void settle_user( uint32_t curr_block_num, account_name owner, name chain, const asset& value );

   public:
      struct transfer_args {
         account_name from;
         account_name to;
         name chain;
         asset quantity;
         std::string memo;
      };
   };

   asset token::get_supply( name chain, symbol_name sym ) const {
      stats statstable( _self, chain );
      const auto& st = statstable.get( sym );
      return st.supply;
   }

   asset token::get_current_balance( account_name owner, name chain, const asset& destcoin ) const {
      accounts from_acnts( _self, owner );
      auto idx = from_acnts.get_index<N( bychain )>();
      auto from = idx.get( get_account_idx( chain, destcoin ), "no balance object found" );
      eosio_assert( from.chain == chain, "symbol chain mismatch" );
      return from.balance;
   }
};

#endif //FORCEIO_CONTRACT_RELAY_TOKEN_HPP
