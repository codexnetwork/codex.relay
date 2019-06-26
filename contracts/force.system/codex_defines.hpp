#pragma once

#include <eosiolib/types.hpp>
#include <eosiolib/contract_config.hpp>

namespace codex {

   // FIXME: Delete all DEFINEs to const var
   #define CORE_SYMBOL_PRECISION 10000
   
   static constexpr int NUM_OF_TOP_BPS = CONTRACT_NUM_OF_TOP_BPS;//23;
#ifdef BEFORE_ONLINE_TEST 
   static constexpr uint32_t CYCLE_PREHOUR = 10;
   static constexpr uint32_t CYCLE_PREBP_BLOCK = 3;
   static constexpr uint32_t CYCLE_PREDAY = 5;//5;//275;
   static constexpr uint32_t STABLE_DAY = 10;//2;//60;
   static constexpr uint64_t PRE_BLOCK_REWARDS = 58.6*CORE_SYMBOL_PRECISION;
   static constexpr uint64_t STABLE_BLOCK_REWARDS = 126*CORE_SYMBOL_PRECISION;
   static constexpr uint32_t FROZEN_DELAY = 3*24*20*60; 
#else
   static constexpr uint32_t CYCLE_PREHOUR = 12;
   static constexpr uint32_t CYCLE_PREBP_BLOCK = 15;
   
   static constexpr uint32_t CYCLE_PREDAY = 275;//5;//275;
   static constexpr uint32_t STABLE_DAY = 60;//2;//60;
   static constexpr uint64_t STABLE_BLOCK_REWARDS = 630*CORE_SYMBOL_PRECISION;
   static constexpr uint64_t PRE_BLOCK_REWARDS = 143*CORE_SYMBOL_PRECISION;
   static constexpr uint32_t FROZEN_DELAY = CONTRACT_FROZEN_DELAY; // 3 * 24 * 60 * 20; //3*24*60*20*3s;
#endif
   //static constexpr uint32_t STABLE_BLOCK_HEIGHT = UPDATE_CYCLE * CYCLE_PREDAY * STABLE_DAY;
   static constexpr uint32_t PRE_GRADIENT = 10250;
   static constexpr uint32_t STABLE_GRADIENT = 10010;
   //static constexpr uint32_t REWARD_MODIFY_COUNT = UPDATE_CYCLE * CYCLE_PREDAY;
   static constexpr uint32_t UPDATE_CYCLE =CYCLE_PREBP_BLOCK * NUM_OF_TOP_BPS;

   static constexpr uint64_t REWARD_ID = 1;
   static constexpr uint64_t BLOCK_OUT_WEIGHT = 1000;
   static constexpr uint64_t MORTGAGE = 8228;

   #define REWARD_RATIO_PRECISION 10000
  // static constexpr uint32_t REWARD_DEVELOP = 900;
   static constexpr uint32_t REWARD_BP = 300;
   static constexpr uint32_t REWARD_FUND = 100;
   static constexpr uint32_t REWARD_MINE = REWARD_RATIO_PRECISION - REWARD_BP;

   static constexpr uint64_t OTHER_COIN_WEIGHT = 500;

   #define LACKMORTGAGE_FREEZE UPDATE_CYCLE * CYCLE_PREHOUR
   #define PUNISH_BP_FEE   asset(100*CORE_SYMBOL_PRECISION)
   #define BAIL_PUNISH_FEE   asset(10*CORE_SYMBOL_PRECISION)

   #define  REWARD_RECORD_SIZE   2000
   #define  BP_REWARD_RECORD_SIZE  360

   // FIXME: Use a Static Name for genesis bps, donot need to make a genesis bp table
   struct creation_producer {
      account_name bp_name;
      int64_t      total_staked    = 0;
      int64_t      mortgage = 0;
   };

   inline bool is_genesis_bp( const account_name bpname ) {
      const auto genesis_bp_per = "codex.bp";
      return eosio::name{static_cast<uint64_t>(bpname)}.to_string().find( genesis_bp_per ) != 0;
   }

   static constexpr creation_producer CREATION_BP[26] = {
      {N(codex.bpa),400000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpb),400000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpc),400000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpd),400000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpe),600000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpf),600000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpg),600000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bph),600000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpi),1300000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpj),1300000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpk),1300000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpl),2100000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpm),2100000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpn),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpo),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpp),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpq),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpr),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bps),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpt),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpu),10000000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpv),100000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpw),100000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpx),100000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpy),100000,200000*CORE_SYMBOL_PRECISION},
      {N(codex.bpz),100000,200000*CORE_SYMBOL_PRECISION}
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
