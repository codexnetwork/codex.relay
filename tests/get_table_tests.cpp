#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/wasm_eosio_constraints.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/chain/config.hpp>

#include <asserter/asserter.wast.hpp>
#include <asserter/asserter.abi.hpp>

#include <force.token/force.token.wast.hpp>
#include <force.token/force.token.abi.hpp>

#include <fc/io/fstream.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>

#include <array>
#include <utility>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

BOOST_AUTO_TEST_SUITE(get_table_tests)

BOOST_FIXTURE_TEST_CASE( get_scope_test, TESTER ) try {
   produce_blocks(2);

   create_accounts({ config::token_account_name, N(eosio.ram), N(eosio.ramfee), N(eosio.stake),
      N(eosio.bpay), N(eosio.vpay), N(eosio.saving), N(eosio.names) });

   std::vector<account_name> accs{N(inita), N(initb), N(initc), N(initd)};
   create_accounts(accs);
   produce_block();

   set_code( config::token_account_name, force_token_wast );
   set_abi( config::token_account_name, force_token_abi );
   produce_blocks(1);

   // create currency
   auto act = mutable_variant_object()
         ("issuer",       "eosio")
         ("maximum_supply", eosio::chain::asset::from_string("1000000000.0000 SYS"));
   push_action(config::token_account_name, N(create), config::token_account_name, act );

   // issue
   for (account_name a: accs) {
      push_action( config::token_account_name, N(issue), "eosio", mutable_variant_object()
                  ("to",      name(a) )
                  ("quantity", eosio::chain::asset::from_string("999.0000 SYS") )
                  ("memo", "")
                  );
   }
   produce_blocks(1);

   // iterate over scope
   eosio::chain_apis::read_only plugin(*(this->control), fc::microseconds(INT_MAX));
   eosio::chain_apis::read_only::get_table_by_scope_params param{config::token_account_name, N(accounts), "inita", "", 10};
   eosio::chain_apis::read_only::get_table_by_scope_result result = plugin.read_only::get_table_by_scope(param);

   BOOST_REQUIRE_EQUAL(4, result.rows.size());
   BOOST_REQUIRE_EQUAL("", result.more);
   if (result.rows.size() >= 4) {
      BOOST_REQUIRE_EQUAL(name(config::token_account_name), result.rows[0].code);
      BOOST_REQUIRE_EQUAL(name(N(inita)), result.rows[0].scope);
      BOOST_REQUIRE_EQUAL(name(N(accounts)), result.rows[0].table);
      BOOST_REQUIRE_EQUAL(name(N(eosio)), result.rows[0].payer);
      BOOST_REQUIRE_EQUAL(1, result.rows[0].count);

      BOOST_REQUIRE_EQUAL(name(N(initb)), result.rows[1].scope);
      BOOST_REQUIRE_EQUAL(name(N(initc)), result.rows[2].scope);
      BOOST_REQUIRE_EQUAL(name(N(initd)), result.rows[3].scope);
   }

   param.lower_bound = "initb";
   param.upper_bound = "initc";
   result = plugin.read_only::get_table_by_scope(param);
   BOOST_REQUIRE_EQUAL(2, result.rows.size());
   BOOST_REQUIRE_EQUAL("", result.more);
   if (result.rows.size() >= 2) {
      BOOST_REQUIRE_EQUAL(name(N(initb)), result.rows[0].scope);
      BOOST_REQUIRE_EQUAL(name(N(initc)), result.rows[1].scope);
   }

   param.limit = 1;
   result = plugin.read_only::get_table_by_scope(param);
   BOOST_REQUIRE_EQUAL(1, result.rows.size());
   BOOST_REQUIRE_EQUAL("initc", result.more);

   param.table = name(0);
   result = plugin.read_only::get_table_by_scope(param);
   BOOST_REQUIRE_EQUAL(1, result.rows.size());
   BOOST_REQUIRE_EQUAL("initc", result.more);

   param.table = N(invalid);
   result = plugin.read_only::get_table_by_scope(param);
   BOOST_REQUIRE_EQUAL(0, result.rows.size());
   BOOST_REQUIRE_EQUAL("", result.more);

} FC_LOG_AND_RETHROW() /// get_scope_test

BOOST_FIXTURE_TEST_CASE( get_table_test, TESTER ) try {
   produce_blocks(2);

   create_accounts({ config::token_account_name, N(eosio.ram), N(eosio.ramfee), N(eosio.stake),
      N(eosio.bpay), N(eosio.vpay), N(eosio.saving), N(eosio.names) });

   std::vector<account_name> accs{N(inita), N(initb)};
   create_accounts(accs);
   produce_block();

   set_code( config::token_account_name, force_token_wast );
   set_abi( config::token_account_name, force_token_abi );
   produce_blocks(1);

   // create currency
   auto act = mutable_variant_object()
         ("issuer",       "eosio")
         ("maximum_supply", eosio::chain::asset::from_string("1000000000.0000 SYS"));
   push_action(config::token_account_name, N(create), config::token_account_name, act );

   // issue
   for (account_name a: accs) {
      push_action( config::token_account_name, N(issue), "eosio", mutable_variant_object()
                  ("to",      name(a) )
                  ("quantity", eosio::chain::asset::from_string("10000.0000 SYS") )
                  ("memo", "")
                  );
   }
   produce_blocks(1);

   // create currency 2
   act = mutable_variant_object()
         ("issuer",       "eosio")
         ("maximum_supply", eosio::chain::asset::from_string("1000000000.0000 AAA"));
   push_action(config::token_account_name, N(create), config::token_account_name, act );
   // issue
   for (account_name a: accs) {
      push_action( config::token_account_name, N(issue), "eosio", mutable_variant_object()
                  ("to",      name(a) )
                  ("quantity", eosio::chain::asset::from_string("9999.0000 AAA") )
                  ("memo", "")
                  );
   }
   produce_blocks(1);

   // create currency 3
   act = mutable_variant_object()
         ("issuer",       "eosio")
         ("maximum_supply", eosio::chain::asset::from_string("1000000000.0000 CCC"));
   push_action(config::token_account_name, N(create), config::token_account_name, act );
   // issue
   for (account_name a: accs) {
      push_action( config::token_account_name, N(issue), "eosio", mutable_variant_object()
                  ("to",      name(a) )
                  ("quantity", eosio::chain::asset::from_string("7777.0000 CCC") )
                  ("memo", "")
                  );
   }
   produce_blocks(1);

   // create currency 3
   act = mutable_variant_object()
         ("issuer",       "eosio")
         ("maximum_supply", eosio::chain::asset::from_string("1000000000.0000 BBB"));
   push_action(config::token_account_name, N(create), config::token_account_name, act );
   // issue
   for (account_name a: accs) {
      push_action( config::token_account_name, N(issue), "eosio", mutable_variant_object()
                  ("to",      name(a) )
                  ("quantity", eosio::chain::asset::from_string("8888.0000 BBB") )
                  ("memo", "")
                  );
   }
   produce_blocks(1);

   // get table: normal case
   eosio::chain_apis::read_only plugin(*(this->control), fc::microseconds(INT_MAX));
   eosio::chain_apis::read_only::get_table_rows_params p;
   p.code = config::token_account_name;
   p.scope = "inita";
   p.table = N(accounts);
   p.json = true;
   p.index_position = "primary";
   eosio::chain_apis::read_only::get_table_rows_result result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(4, result.rows.size());
   BOOST_REQUIRE_EQUAL(false, result.more);
   if (result.rows.size() >= 4) {
      BOOST_REQUIRE_EQUAL("9999.0000 AAA", result.rows[0]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("8888.0000 BBB", result.rows[1]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("7777.0000 CCC", result.rows[2]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("10000.0000 SYS", result.rows[3]["balance"].as_string());
   }

   // get table: reverse ordered
   p.reverse = true;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(4, result.rows.size());
   BOOST_REQUIRE_EQUAL(false, result.more);
   if (result.rows.size() >= 4) {
      BOOST_REQUIRE_EQUAL("9999.0000 AAA", result.rows[3]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("8888.0000 BBB", result.rows[2]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("7777.0000 CCC", result.rows[1]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("10000.0000 SYS", result.rows[0]["balance"].as_string());
   }

   // get table: reverse ordered, with ram payer
   p.reverse = true;
   p.show_payer = true;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(4, result.rows.size());
   BOOST_REQUIRE_EQUAL(false, result.more);
   if (result.rows.size() >= 4) {
      BOOST_REQUIRE_EQUAL("9999.0000 AAA", result.rows[3]["data"]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("8888.0000 BBB", result.rows[2]["data"]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("7777.0000 CCC", result.rows[1]["data"]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("10000.0000 SYS", result.rows[0]["data"]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("eosio", result.rows[0]["payer"].as_string());
      BOOST_REQUIRE_EQUAL("eosio", result.rows[1]["payer"].as_string());
      BOOST_REQUIRE_EQUAL("eosio", result.rows[2]["payer"].as_string());
      BOOST_REQUIRE_EQUAL("eosio", result.rows[3]["payer"].as_string());
   }
   p.show_payer = false;

   // get table: normal case, with bound
   p.lower_bound = "BBB";
   p.upper_bound = "CCC";
   p.reverse = false;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(2, result.rows.size());
   BOOST_REQUIRE_EQUAL(false, result.more);
   if (result.rows.size() >= 2) {
      BOOST_REQUIRE_EQUAL("8888.0000 BBB", result.rows[0]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("7777.0000 CCC", result.rows[1]["balance"].as_string());
   }

   // get table: reverse case, with bound
   p.lower_bound = "BBB";
   p.upper_bound = "CCC";
   p.reverse = true;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(2, result.rows.size());
   BOOST_REQUIRE_EQUAL(false, result.more);
   if (result.rows.size() >= 2) {
      BOOST_REQUIRE_EQUAL("8888.0000 BBB", result.rows[1]["balance"].as_string());
      BOOST_REQUIRE_EQUAL("7777.0000 CCC", result.rows[0]["balance"].as_string());
   }

   // get table: normal case, with limit
   p.lower_bound = p.upper_bound = "";
   p.limit = 1;
   p.reverse = false;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(1, result.rows.size());
   BOOST_REQUIRE_EQUAL(true, result.more);
   if (result.rows.size() >= 1) {
      BOOST_REQUIRE_EQUAL("9999.0000 AAA", result.rows[0]["balance"].as_string());
   }

   // get table: reverse case, with limit
   p.lower_bound = p.upper_bound = "";
   p.limit = 1;
   p.reverse = true;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(1, result.rows.size());
   BOOST_REQUIRE_EQUAL(true, result.more);
   if (result.rows.size() >= 1) {
      BOOST_REQUIRE_EQUAL("10000.0000 SYS", result.rows[0]["balance"].as_string());
   }

   // get table: normal case, with bound & limit
   p.lower_bound = "BBB";
   p.upper_bound = "CCC";
   p.limit = 1;
   p.reverse = false;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(1, result.rows.size());
   BOOST_REQUIRE_EQUAL(true, result.more);
   if (result.rows.size() >= 1) {
      BOOST_REQUIRE_EQUAL("8888.0000 BBB", result.rows[0]["balance"].as_string());
   }

   // get table: reverse case, with bound & limit
   p.lower_bound = "BBB";
   p.upper_bound = "CCC";
   p.limit = 1;
   p.reverse = true;
   result = plugin.read_only::get_table_rows(p);
   BOOST_REQUIRE_EQUAL(1, result.rows.size());
   BOOST_REQUIRE_EQUAL(true, result.more);
   if (result.rows.size() >= 1) {
      BOOST_REQUIRE_EQUAL("7777.0000 CCC", result.rows[0]["balance"].as_string());
   }

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( get_table_by_seckey_test, TESTER ) try {
   produce_blocks(2);

   create_accounts({ config::token_account_name, N(eosio.ram), N(eosio.ramfee), N(eosio.stake),
      N(eosio.bpay), N(eosio.vpay), N(eosio.saving), N(eosio.names) });

   std::vector<account_name> accs{N(inita), N(initb), N(initc), N(initd)};
   create_accounts(accs);
   produce_block();

   set_code( config::token_account_name, force_token_wast );
   set_abi( config::token_account_name, force_token_abi );
   produce_blocks(1);

   // create currency
   auto act = mutable_variant_object()
         ("issuer",       "eosio")
         ("maximum_supply", eosio::chain::asset::from_string("1000000000.0000 SYS"));
   push_action(config::token_account_name, N(create), config::token_account_name, act );

   // issue
   for (account_name a: accs) {
      push_action( config::token_account_name, N(issue), "eosio", mutable_variant_object()
                  ("to",      name(a) )
                  ("quantity", eosio::chain::asset::from_string("10000.0000 SYS") )
                  ("memo", "")
                  );
   }
   produce_blocks(1);

} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
