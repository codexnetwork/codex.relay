#ifndef CODEX_INCLUED_SYS_MATCH_DEFINES_HPP_
#define CODEX_INCLUED_SYS_MATCH_DEFINES_HPP_
#pragma once

#include <string>
#include <vector>

#include <eosiolib/types.hpp>
#include <eosiolib/eosio.hpp>

namespace codex {

namespace utils{
  constexpr inline int64_t precision( uint64_t decimals ) {
      constexpr uint64_t res_size = 18;
      constexpr int64_t res[res_size] = 
         {  1, 10, 100, 1000, 10000, 
            100000, 
            1000000, 
            10000000,
            100000000,
            1000000000,
            10000000000,
            100000000000,
            1000000000000,
            10000000000000,
            100000000000000,
            1000000000000000,
            10000000000000000,
            100000000000000000 };

      if( decimals < res_size ) {
         return res[decimals];
      } else {
         auto p10 = res[res_size - 1];
         for( auto p = static_cast<int64_t>(decimals - res_size + 1); 
              p > 0; --p ) {
            p10 *= 10;
         }
         return p10;
      }
   }
}

namespace trade {

   namespace __details {
      
      // TODO: Need refactor split func
      void splitMemo( std::vector<std::string>& results, const std::string& memo, char separator ) {
         auto start = memo.cbegin();
         auto end = memo.cend();

         for( auto it = start; it != end; ++it ) {
            if( *it == separator ) {
               results.emplace_back(start, it);
               start = it + 1;
            }
         }
         if (start != end) results.emplace_back(start, end);
      }
   }

   enum class func_typ : uint64_t {
      match              = 1,
      bridge_addmortgage = 2,
      bridge_exchange    = 3,
      trade_type_count   = 4
   };

   struct sys_bridge_addmort {
      eosio::name  trade_name;
      account_name trade_maker = 0;
      uint64_t     type        = 0;

      explicit sys_bridge_addmort( const std::string& memo ) {
         std::vector<std::string> memoParts;
         memoParts.reserve( 8 );
         __details::splitMemo( memoParts, memo, ';' );

         eosio_assert( memoParts.size() == 3, "memo is not adapted with bridge_addmortgage" );

         this->trade_name.value = ::eosio::string_to_name( memoParts[0].c_str() );
         this->trade_maker = ::eosio::string_to_name( memoParts[1].c_str() );
         this->type = atoi( memoParts[2].c_str() );

         // FIXME: should use enum
         eosio_assert( this->type == 1 || this->type == 2,
                       "type is not adapted with bridge_addmortgage" );
      }

      void done( const eosio::name& chain, 
                 account_name from, 
                 const eosio::asset& quantity ) const {
         eosio::action(
            {{ config::bridge_account_name, N(active) }},
            config::bridge_account_name,
            N(addmortgage),
            std::make_tuple(
                  trade_name.value,
                  trade_maker,
                  from,
                  chain,
                  quantity,
                  type
            )
         ).send();
      }
   };

   struct sys_bridge_exchange {
      eosio::name  trade_name;
      account_name trade_maker = 0;
      account_name recv        = 0;
      uint64_t     type;

      explicit sys_bridge_exchange( const std::string& memo ) {
         std::vector<std::string> memoParts;
         memoParts.reserve( 8 );
         __details::splitMemo( memoParts, memo, ';' );

         eosio_assert( memoParts.size() == 4, "memo is not adapted with bridge_addmortgage" );

         this->trade_name.value = ::eosio::string_to_name( memoParts[0].c_str() );
         this->trade_maker = ::eosio::string_to_name( memoParts[1].c_str() );
         this->recv = ::eosio::string_to_name( memoParts[2].c_str() );
         this->type = atoi( memoParts[3].c_str() );

         // FIXME: should use enum
         eosio_assert( this->type == 1 || this->type == 2,
                       "type is not adapted with bridge_addmortgage" );
      }

      void done( const eosio::name& chain, 
                 account_name from, 
                 const eosio::asset& quantity ) const {
         eosio::action(
            {{ config::bridge_account_name,N(active) }},
            config::bridge_account_name,
            N(exchange),
            std::make_tuple(
                  trade_name.value,
                  trade_maker,
                  from,
                  recv,
                  chain,
                  quantity,
                  type
            )
         ).send();
      }
   };
   
} // namespace trade
} // namespace codex

#endif // CODEX_INCLUED_SYS_MATCH_DEFINES_HPP_
