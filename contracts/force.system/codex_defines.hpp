#pragma once

#include <eosiolib/types.hpp>
#include <eosiolib/contract_config.hpp>

namespace codex {

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
   enum  class active_type:int32_t {
      Normal=0,
      Punish,
      LackMortgage,
      Removed
   };
}