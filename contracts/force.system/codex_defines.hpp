#pragma once

#include <eosiolib/types.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract_config.hpp>

namespace codex {

   // FIXME: Delete all DEFINEs to const var
   static constexpr int64_t CORE_SYMBOL_PRECISION = 10000;

   // TODO: in next version core_asset will be constexpr
   inline eosio::asset core_asset( const int64_t value ){
      return eosio::asset{ value * CORE_SYMBOL_PRECISION };
   }

   static constexpr int NUM_OF_TOP_BPS = CONTRACT_NUM_OF_TOP_BPS;

#ifdef BEFORE_ONLINE_TEST 
   static constexpr uint32_t CYCLE_PREHOUR        = 10;
   static constexpr uint32_t CYCLE_PREBP_BLOCK    = 3;
   static constexpr uint32_t CYCLE_PREDAY         = 5;
   static constexpr uint32_t STABLE_DAY           = 10;
   static constexpr int64_t  PRE_BLOCK_REWARDS    = 586 * CORE_SYMBOL_PRECISION / 10;
   static constexpr int64_t  STABLE_BLOCK_REWARDS = 126 * CORE_SYMBOL_PRECISION;
   static constexpr uint32_t FROZEN_DELAY         = 3 * 24 * 20 * 60;
#else
   static constexpr uint32_t CYCLE_PREHOUR        = 12;
   static constexpr uint32_t CYCLE_PREBP_BLOCK    = 15;
   static constexpr uint32_t CYCLE_PREDAY         = 275;
   static constexpr uint32_t STABLE_DAY           = 60;
   static constexpr int64_t  PRE_BLOCK_REWARDS    = 143 * CORE_SYMBOL_PRECISION;
   static constexpr int64_t  STABLE_BLOCK_REWARDS = 630 * CORE_SYMBOL_PRECISION;
   static constexpr uint32_t FROZEN_DELAY         = CONTRACT_FROZEN_DELAY;
#endif

   static constexpr uint32_t PRE_GRADIENT    = 10250;
   static constexpr uint32_t STABLE_GRADIENT = 10010;
   static constexpr uint32_t UPDATE_CYCLE    = CYCLE_PREBP_BLOCK * NUM_OF_TOP_BPS;

   static constexpr uint64_t REWARD_ID        = 1;
   static constexpr uint64_t BLOCK_OUT_WEIGHT = 1000;
   static constexpr uint64_t MORTGAGE         = 8228;

   static constexpr uint32_t REWARD_RATIO_PRECISION = 10000;
   static constexpr uint32_t REWARD_BP              = 300;
   static constexpr uint32_t REWARD_FUND            = 100;
   static constexpr uint32_t REWARD_MINE            = REWARD_RATIO_PRECISION - REWARD_BP;

   static constexpr uint64_t OTHER_COIN_WEIGHT = 500;

   static constexpr uint32_t LACKMORTGAGE_FREEZE = UPDATE_CYCLE * CYCLE_PREHOUR;

   // TODO make asset constexpr
   static constexpr int64_t PUNISH_BP_FEE   = 100 * CORE_SYMBOL_PRECISION;
   static constexpr int64_t BAIL_PUNISH_FEE = 10 * CORE_SYMBOL_PRECISION;

   static constexpr uint32_t REWARD_RECORD_SIZE    = 2000;
   static constexpr uint32_t BP_REWARD_RECORD_SIZE = 360;

   // FIXME: Use a Static Name for genesis bps, donot need to make a genesis bp table
   struct creation_producer {
      account_name bp_name;
      int64_t      total_staked    = 0;
      int64_t      mortgage = 0;
   };

   static constexpr creation_producer CREATION_BP[26] = {
      {N(biosbpa),400000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpb),400000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpc),400000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpd),400000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpe),600000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpf),600000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpg),600000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbph),600000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpi),1300000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpj),1300000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpk),1300000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpl),2100000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpm),2100000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpn),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpo),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpp),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpq),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpr),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbps),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpt),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpu),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpv),100000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpw),100000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpx),100000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpy),100000,200000*CORE_SYMBOL_PRECISION},
      {N(biosbpz),100000,200000*CORE_SYMBOL_PRECISION}
   };
   
   // FIXME: By FanYang Need refactor the code
   
   // bp active_type
   // normal -- error when producer block, after bps muit-sign --> punish -- after two days --> wait_no_punish --> bp do action --> normal
   //  |
   //  -- check no mortgage in onblock --> lack_mortgage -- wait bp mortgage --> wait_normal_from_lack_mortgage -- after a hour --> normal
   // |
   // -- bps muit-sign --> removed
   // FIXME: Need refactor the code manager state in bp class
   enum active_type : int32_t { 
      Normal       = 0,   // normal
      Punish       = 1,   // punish : bp no reward and cannot producer block in 2 days
      LackMortgage = 2,   // lack_mortgage : no mortgage for a bp, after a hour will to normal
      Removed      = 3    // removed : bp has been removed, can acitve
   };
}
