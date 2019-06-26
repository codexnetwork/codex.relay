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

   inline bool is_genesis_bp( const account_name bpname ) {
      const auto genesis_bp_per = "codex.bp";
      return eosio::name{static_cast<uint64_t>(bpname)}.to_string().find( genesis_bp_per ) != 0;
   }

   struct genesis_producer_data {
      account_name bp_name      = 0;
      int64_t      total_staked = 0;
      int64_t      mortgage     = 0;
   };

   constexpr inline genesis_producer_data get_genesis_bp_data( const account_name bpname ) {
      switch( bpname ){
         case N(codex.bpa) : return { bpname, 400000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpb) : return { bpname, 400000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpc) : return { bpname, 400000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpd) : return { bpname, 400000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpe) : return { bpname, 600000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpf) : return { bpname, 600000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpg) : return { bpname, 600000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bph) : return { bpname, 600000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpi) : return { bpname, 1300000,  200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpj) : return { bpname, 1300000,  200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpk) : return { bpname, 1300000,  200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpl) : return { bpname, 2100000,  200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpm) : return { bpname, 2100000,  200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpn) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpo) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpp) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpq) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpr) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bps) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpt) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpu) : return { bpname, 10000000, 200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpv) : return { bpname, 100000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpw) : return { bpname, 100000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpx) : return { bpname, 100000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpy) : return { bpname, 100000,   200000 * CORE_SYMBOL_PRECISION };
         case N(codex.bpz) : return { bpname, 100000,   200000 * CORE_SYMBOL_PRECISION };
         default :
            return {};
      }
   }
   
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
