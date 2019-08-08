#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester_network.hpp>
#include <eosio/chain/producer_object.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <contracts.hpp>
//#include <relay.token/relay.token.wast.hpp>
//#include <relay.token/relay.token.abi.hpp>
//#include <sys.match/sys.match.wast.hpp>
//#include <sys.match/sys.match.abi.hpp>


#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;

struct sys_match : public TESTER {
   
public:
   sys_match() : sys_match_acc(config::match_account_name), exc_acc("biosbpa"), buyer("buyer"), 
      seller("seller"), escrow(config::match_account_name), rel_token_acc(config::relay_token_account_name),
      pair1_base(4, "BTC"), pair1_quote(2, "USDT"), pair2_base(4, "BTCC"), pair2_quote(2, "USDTC") {
      produce_blocks(2);
      
      // deploy relay.token contract
      create_account(rel_token_acc);
      /*push_action(config::system_account_name, name("freeze"), rel_token_acc, fc::mutable_variant_object()
         ("voter", rel_token_acc)
         ("stake", "10000.0000 SYS"));
      push_action(config::system_account_name, name("vote4ram"), rel_token_acc, fc::mutable_variant_object()
         ("voter", rel_token_acc)
         ("bpname", "codex.bpa")
         ("stake", "10000.0000 SYS"));*/
      set_code(rel_token_acc, contracts::relay_token_wasm());
      set_abi(rel_token_acc, contracts::relay_token_abi().data());
      
      // first, deploy sys.match contract on eosforce contract
      create_account(sys_match_acc);
      /*push_action(config::system_account_name, name("freeze"), sys_match_acc, fc::mutable_variant_object()
         ("voter", sys_match_acc)
         ("stake", "10000.0000 SYS"));
      push_action(config::system_account_name, name("vote4ram"), sys_match_acc, fc::mutable_variant_object()
         ("voter", sys_match_acc)
         ("bpname", "codex.bpa")
         ("stake", "10000.0000 SYS"));*/
      set_code(sys_match_acc, contracts::sys_match_wasm());
      set_abi(sys_match_acc, contracts::sys_match_abi().data());
      
      // second, setfee
      set_fee(sys_match_acc, N(create), asset(100), 100000, 1000000, 1000);
      set_fee(sys_match_acc, N(match), asset(100), 100000, 1000000, 1000);
      set_fee(sys_match_acc, N(cancel), asset(100), 100000, 1000000, 1000);
      set_fee(sys_match_acc, N(done), asset(100), 100000, 1000000, 1000);
      set_fee(rel_token_acc, N(trade), asset(100), 100000, 1000000, 1000);
      set_fee(rel_token_acc, N(create), asset(100), 100000, 1000000, 1000);
      set_fee(rel_token_acc, N(issue), asset(100), 100000, 1000000, 1000);
      set_fee(rel_token_acc, N(transfer), asset(100), 100000, 1000000, 1000);
      
      // third, issue BTC/USDT tokens
      create_account(seller);// sell
      create_account(buyer);// buy
      
      create_account(token1_chain);
      push_action(rel_token_acc, name("create"), token1_chain, fc::mutable_variant_object()
         ("issuer", seller)
         ("chain", token1_chain)
         ("side_account", "relay.token")
         ("side_action", "transfer")
         ("maximum_supply", token1_max_supply)
      );
      create_account(token2_chain);
      push_action(rel_token_acc, name("create"), token2_chain, fc::mutable_variant_object()
         ("issuer", buyer)
         ("chain", token2_chain)
         ("side_account", "relay.token")
         ("side_action", "transfer")
         ("maximum_supply", token2_max_supply)
      );
      
      push_action(rel_token_acc, name("issue"), seller, fc::mutable_variant_object()
         ("chain", token1_chain)
         ("to", seller)
         ("quantity", token1_max_supply)
         ("memo", "issue")
      );
      push_action(rel_token_acc, name("issue"), buyer, fc::mutable_variant_object()
         ("chain", token2_chain)
         ("to", buyer)
         ("quantity", token2_max_supply)
         ("memo", "issue")
      );
         
      // fourth, authorization
      create_account(exc_acc);
      asset transfer_amt(1000000000);
      transfer( N(eosforce), exc_acc, transfer_amt, "create_account", config::token_account_name );
      
      asset stake(400000000);   
      vote4ram2(exc_acc, N(codex.bpa), stake);

      // efc set account permission biosbpa active '{"threshold": 1,"keys": [{"key": "CDX5muUziYrETi5b6G2Ev91dCBrEm3qir7PK4S2qSFqfqcmouyzCr","weight": 1}],"accounts": [{"permission":{"actor":"codex.match","permission":"codex.code"},"weight":1}]}' owner -p biosbpa
      auto auth = authority(get_public_key(exc_acc, "active"), 0);
      auth.accounts.push_back( permission_level_weight{{config::match_account_name, config::eosio_code_name}, 1} );
      push_action(config::system_account_name, updateauth::get_name(), exc_acc, fc::mutable_variant_object()
         ("account", exc_acc)
         ("permission", "active")
         ("parent", "owner")
         ("auth",  auth)
      );
      
      auth = authority(get_public_key(sys_match_acc, "active"), 0);
      auth.accounts.push_back( permission_level_weight{{sys_match_acc, config::eosio_code_name}, 1} );
      push_action(config::system_account_name, updateauth::get_name(), sys_match_acc, fc::mutable_variant_object()
         ("account", sys_match_acc)
         ("permission", "active")
         ("parent", "owner")
         ("auth",  auth)
      );
      

//      efc push action sys.match create '["4,BTC", "btc1", "4,CBTC", "2,USDT", "usdt1", "2,CUSDT", "0", "codex.bpa"]' -p codex.bpa
//efc push action sys.match create '["4,BTCC", "btc1", "4,CBTC", "2,USDT", "usdt1", "2,CUSDT", "0", "codex.bpa"]' -p codex.bpa

      // fifth, create trading pair
      //efc push action codex.token trade '["biosbpa", "codex.match", "1000.0000 CDX", "1", "freeze;regex;0"]' -p biosbpa
      // freeze system token for creating trading pair
      push_action(config::token_account_name, name("trade"), exc_acc, fc::mutable_variant_object()
           ("from", exc_acc)
           ("to", escrow)
           ("quantity", "1000.0000 CDX")
           ("type", 1)
           ("memo", "freeze;regex;0")
      );
      //efc push action codex.match regex '["biosbpa"]' -p biosbpa
      push_action(sys_match_acc, name("regex"), exc_acc, fc::mutable_variant_object()
         ("exc_acc", exc_acc)
      );
      
      push_action(sys_match_acc, name("create"), exc_acc, fc::mutable_variant_object()
         ("base", pair1_base)
         ("base_chain", token1_chain)
         ("base_sym", asset::from_string(string(token1_max_supply)).get_symbol())
         ("quote", pair1_quote)
         ("quote_chain", token2_chain)
         ("quote_sym", asset::from_string(string(token2_max_supply)).get_symbol())
         ("exc_acc", exc_acc)
      );
      
      // open trading pair
      push_action(config::token_account_name, name("trade"), exc_acc, fc::mutable_variant_object()
           ("from", exc_acc)
           ("to", escrow)
           ("quantity", "1000.0000 CDX")
           ("type", 1)
           ("memo", "freeze;open;0")
      );
      
      push_action(sys_match_acc, name("open"), exc_acc, fc::mutable_variant_object()
         ("base_chain", token1_chain)
         ("base_sym", asset::from_string(string(token1_max_supply)).get_symbol())
         ("quote_chain", token2_chain)
         ("quote_sym", asset::from_string(string(token2_max_supply)).get_symbol())
         ("exc_acc", exc_acc)
      );
      
   }
   ~sys_match() {}
   
   account_name sys_match_acc;
   account_name exc_acc;
   account_name buyer;
   account_name seller;
   account_name escrow;
   account_name rel_token_acc;
   const name token1_chain = "btc1";
   const char *token1_max_supply = "1000000000.0000 CBTC";
   const name token2_chain = "usdt1";
   const char *token2_max_supply = "1000000000.0000 CUSDT";
   const symbol pair1_base;
   const symbol pair1_quote;
   const symbol pair2_base;
   const symbol pair2_quote;
   const uint32_t trade_type = 1;
   const uint32_t pair1_fee_rate = 0;
   const uint32_t pair2_fee_rate = 10;
};

BOOST_FIXTURE_TEST_SUITE(sys_match_tests, sys_match)

// partial match / full match when test price equals order price
BOOST_AUTO_TEST_CASE( test1 ) { try {

   produce_blocks(2);
   asset quantity;
   asset seller_before;
   asset seller_now;
   asset buyer_before;
   asset buyer_now;
   asset escrow_before;
   asset escrow_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // make an ask order
   // efc push action relay.token trade '["testa", "codex.match", "btc1", "4.0000 CBTC", "1", "trade;1;4000.00 CUSDT;0;biosbpa;testc;2"]' -p testa
   
   quantity = asset::from_string( "4.0000 CBTC" );
   seller_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), seller);
   escrow_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.00 CUSDT;0;biosbpa;testc;2")
   );
   seller_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), seller);  
   escrow_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL((seller_before - quantity), seller_now);
   BOOST_REQUIRE_EQUAL((escrow_before + quantity), escrow_now);

   // make a bid order
   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "39500.0000 CUSDT", "1", "trade;1;3950.00 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "39500.0000 CUSDT" );
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.00 CUSDT;1;biosbpa;;2")
   );
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_before - quantity, buyer_now);
   BOOST_REQUIRE_EQUAL(escrow_before + quantity, escrow_now);

   // partially match bid orders (price equals buy first price)
   // efc push action relay.token trade '["testa", "sys.match", "btc1", "1.0000 CBTC", "1", "trade;1;3950.00 CUSDT;0;biosbpa;testc;2"]' -p testa
   quantity = asset::from_string( "1.0000 CBTC" );
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.00 CUSDT;0;biosbpa;testc;2")
   );
   seller_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(seller_before + asset::from_string( "3950.0000 CUSDT" ), seller_now);
   BOOST_REQUIRE_EQUAL(buyer_before + quantity, buyer_now);

   // fully match bid orders (price equals buy first price, bid quantity equals order quantity)
   // efc push action relay.token trade '["testa", "sys.match", "btc1", "1.0000 CBTC", "1", "trade;1;3950.00 CUSDT;0;biosbpa;testc;2"]' -p testa
   quantity = asset::from_string( "9.0000 CBTC" );
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.00 CUSDT;0;biosbpa;testc;2")
   );
   seller_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(seller_before + asset::from_string( "35550.0000 CUSDT" ), seller_now);
   BOOST_REQUIRE_EQUAL(buyer_before + quantity, buyer_now);

   // partially match ask order (price equals sell first price)
   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "4000.0000 CUSDT", "1", "trade;1;4000.00 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "4000.0000 CUSDT" );
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), seller);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.00 CUSDT;1;biosbpa;;2")
   );
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   seller_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), seller);
   BOOST_REQUIRE_EQUAL(buyer_before + asset::from_string( "1.0000 CBTC" ), buyer_now);
   BOOST_REQUIRE_EQUAL(seller_before + quantity, seller_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "4000.0000 CUSDT", "1", "trade;1;4000.00 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "12000.0000 CUSDT" );
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), seller);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.00 CUSDT;1;biosbpa;;2")
   );
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   seller_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), seller);
   BOOST_REQUIRE_EQUAL(buyer_before + asset::from_string( "3.0000 CBTC" ), buyer_now);
   BOOST_REQUIRE_EQUAL(seller_before + quantity, seller_now);

   produce_blocks(8);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( test2 ) { try {

   produce_blocks(2);
   asset quantity;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // make an ask order
   // efc push action relay.token trade '["testa", "codex.match", "btc1", "4.0000 CBTC", "1", "trade;0;4000.00 CUSDT;0;biosbpa;testc;2"]' -p testa
   quantity = asset::from_string( "4.0000 CBTC" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), seller);
   escrow_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.00 CUSDT;0;biosbpa;testc;2")
   );
   seller_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), seller);  
   escrow_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL((seller_base_before - quantity), seller_base_now);
   BOOST_REQUIRE_EQUAL((escrow_base_before + quantity), escrow_base_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "39500.0000 CUSDT", "1", "trade;0;3950.00 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "39500.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.00 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);
   BOOST_REQUIRE_EQUAL(escrow_quote_before + quantity, escrow_quote_now);

   // partially match bid orders (price less than buy first price)
   // efc push action relay.token trade '["testa", "codex.match", "btc1", "1.0000 CBTC", "1", "trade;0;3900.00 CUSDT;0;biosbpa;testc;2"]' -p testa
   quantity = asset::from_string( "1.0000 CBTC" );
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3900.00 CUSDT;0;biosbpa;testc;2")
   );
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(seller_quote_before + asset::from_string( "3950.0000 CUSDT" ), seller_quote_now);
   BOOST_REQUIRE_EQUAL(buyer_base_before + quantity, buyer_base_now);

   // fully match bid orders (price equals buy first price, bid quantity equals order quantity)
   // efc push action relay.token trade '["testa", "codex.match", "btc1", "9.0000 CBTC", "1", "trade;0;3950.00 CUSDT;0;biosbpa;testc;2"]' -p testa
   quantity = asset::from_string( "9.0000 CBTC" );
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.00 CUSDT;0;biosbpa;testc;2")
   );
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(seller_quote_before + asset::from_string( "35550.0000 CUSDT" ), seller_quote_now);
   BOOST_REQUIRE_EQUAL(buyer_base_before + quantity, buyer_base_now);

   // partially match ask order (price equals sell first price)
   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "4100.0000 CUSDT", "1", "trade;0;4100.00 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "4100.0000 CUSDT" );
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4100.00 CUSDT;1;biosbpa;;2")
   );
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   BOOST_REQUIRE_EQUAL(seller_quote_before + asset::from_string( "4000.0000 CUSDT" ), seller_quote_now);
   BOOST_REQUIRE_EQUAL(buyer_base_before + asset::from_string( "1.0000 CBTC" ), buyer_base_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "12000.0000 CUSDT", "1", "trade;0;4000.00 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "12000.0000 CUSDT" );
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), seller);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.00 CUSDT;1;biosbpa;;2")
   );
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), seller);
   BOOST_REQUIRE_EQUAL(buyer_base_before + asset::from_string( "3.0000 CBTC" ), buyer_base_now);
   BOOST_REQUIRE_EQUAL(seller_quote_before + quantity, seller_quote_now);

   produce_blocks(8);

} FC_LOG_AND_RETHROW() }

// fully match bid orders and has remaining
BOOST_AUTO_TEST_CASE( test3 ) { try {
   asset quantity, matched;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // efc push action relay.token trade '["testb", "codex.match", "eosforce", "8600.0000 CUSDT", "1", "trade;0;4300.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "8600.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4300.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testa", "codex.match", "btc1", "1.0000 CBTC", "1", "trade;1;4100.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "1.0000 CBTC" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4100.00 CUSDT;0;biosbpa;;2")
   );
   seller_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(seller_base_before - quantity, seller_base_now);
   BOOST_REQUIRE_EQUAL(seller_quote_before + asset::from_string( "4300.0000 CUSDT" ), seller_quote_now);
   BOOST_REQUIRE_EQUAL(buyer_base_before + quantity, buyer_base_now);
   BOOST_REQUIRE_EQUAL(escrow_base_before, escrow_base_now); // quote toeken: buyer -> seller,  escrow do not change

   // efc push action relay.token trade '["testa", "codex.match", "btc1", "2.0000 CBTC", "1", "trade;0;4200.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "2.0000 CBTC" );
   matched  = asset::from_string( "1.0000 CBTC" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4200.00 CUSDT;0;biosbpa;;2")
   );
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   escrow_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   BOOST_REQUIRE_EQUAL(buyer_base_before + matched, buyer_base_now);
   BOOST_REQUIRE_EQUAL(escrow_base_before + asset::from_string( "1.0000 CBTC" ), escrow_base_now);
} FC_LOG_AND_RETHROW() }

// fully match ask orders and has remaining
BOOST_AUTO_TEST_CASE( test4 ) { try {
   asset quantity, matched;
   asset seller_before, seller_now;
   asset buyer_before, buyer_now;
   asset escrow_before, escrow_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   //efc push action relay.token trade '["testa", "codex.match", "eosforce", "2.0000 CBTC", "1", "trade;0;3900.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "2.0000 CBTC" );
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3900.00 CUSDT;0;biosbpa;;2")
   );
   seller_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(seller_before, seller_now);
   BOOST_REQUIRE_EQUAL(buyer_before, buyer_now);

   //efc push action relay.token trade '["testb", "codex.match", "eosforce", "4000.0000 CUSDT", "1", "trade;0;4000.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "4000.0000 CUSDT" );
   matched  = asset::from_string( "3900.0000 CUSDT" );
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_before - matched, buyer_now);
   BOOST_REQUIRE_EQUAL(escrow_before, escrow_now); // quote toeken: buyer -> seller,  escrow do not change

   //efc push action relay.token trade '["testb", "codex.match", "eosforce", "7900.0000 CUSDT", "1", "trade;0;3950.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "7900.0000 CUSDT" );
   matched  = asset::from_string( "3900.0000 CUSDT" );
   seller_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_before - asset::from_string( "3950.0000 CUSDT" ) - matched, buyer_now);
   BOOST_REQUIRE_EQUAL(escrow_before + asset::from_string( "3950.0000 CUSDT" ), escrow_now);
} FC_LOG_AND_RETHROW() }

// when ask price is greater than bid order price, eats bid  orders  continuously
BOOST_AUTO_TEST_CASE( test5 ) { try {
   asset quantity, matched;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // efc push action relay.token trade '["testb", "codex.match", "eosforce", "4000.0000 CUSDT", "1", "trade;0;4000.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "4000.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testb", "codex.match", "eosforce", "7900.0000 CUSDT", "1", "trade;0;3950.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "7900.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3950.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);
   BOOST_REQUIRE_EQUAL(escrow_quote_before + quantity, escrow_quote_now); // quote toeken: buyer -> seller,  escrow do not change

   // efc push action relay.token trade '["testa", "codex.match", "eosforce", "2.0000 CBTC", "1", "trade;0;10.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "2.0000 CBTC" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;10.00 CUSDT;0;biosbpa;;2")
   );
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   escrow_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   BOOST_REQUIRE_EQUAL(buyer_base_before + quantity, buyer_base_now);
   BOOST_REQUIRE_EQUAL(escrow_base_before, escrow_base_now);
} FC_LOG_AND_RETHROW() }

// eat ask orders continuously (bid price equals last deal price)
BOOST_AUTO_TEST_CASE( test6 ) { try {
   asset quantity, matched;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // efc push action relay.token trade '["testa", "codex.match", "eosforce", "1.0000 CBTC", "1", "trade;0;4100.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "1.0000 CBTC" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4100.00 CUSDT;0;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before, buyer_quote_now);

   // efc push action relay.token trade '["testa", "codex.match", "btc1", "2.0000 CBTC", "1", "trade;0;4200.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "2.0000 CBTC" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4200.00 CUSDT;0;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_quote_before, buyer_quote_now);
   BOOST_REQUIRE_EQUAL(escrow_quote_before, escrow_quote_now); // quote toeken: buyer -> seller,  escrow do not change

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "8400.0000 CUSDT", "1", "trade;0;4200.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "8400.0000 CUSDT" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4200.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   escrow_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   BOOST_REQUIRE_EQUAL(buyer_base_before + asset::from_string( "2.0000 CBTC" ), buyer_base_now);
   BOOST_REQUIRE_EQUAL(escrow_base_before - asset::from_string( "2.0000 CBTC" ), escrow_base_now);
} FC_LOG_AND_RETHROW() }

// eat ask orders continuously (bid price is greater than last deal price)
BOOST_AUTO_TEST_CASE( test7 ) { try {
   asset quantity, matched;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // efc push action relay.token trade '["testa", "codex.match", "eosforce", "1.0000 CBTC", "1", "trade;0;4100.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "1.0000 CBTC" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4100.00 CUSDT;0;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before, buyer_quote_now);

   // efc push action relay.token trade '["testa", "codex.match", "btc1", "2.0000 CBTC", "1", "trade;0;4200.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "2.0000 CBTC" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4200.00 CUSDT;0;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   BOOST_REQUIRE_EQUAL(buyer_quote_before, buyer_quote_now);
   BOOST_REQUIRE_EQUAL(escrow_quote_before, escrow_quote_now); // quote toeken: buyer -> seller,  escrow do not change

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "8600.0000 CUSDT", "1", "trade;0;4300.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "8600.0000 CUSDT" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4300.0000 CUSDT;1;biosbpa;;2")
   );
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   escrow_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   BOOST_REQUIRE_EQUAL(seller_quote_before + asset::from_string( "8300.0000 CUSDT" ), seller_quote_now);
   BOOST_REQUIRE_EQUAL(buyer_base_before + asset::from_string( "2.0000 CBTC" ), buyer_base_now);
   BOOST_REQUIRE_EQUAL(escrow_base_before - asset::from_string( "2.0000 CBTC" ), escrow_base_now);
} FC_LOG_AND_RETHROW() }

// eat bid orders continuously (eat all)
BOOST_AUTO_TEST_CASE( test8 ) { try {
   asset quantity, matched;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "394.0000 CUSDT", "1", "trade;0;3940.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "394.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3940.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "393.0000 CUSDT", "1", "trade;0;3930.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "393.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3930.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);
   
   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "392.0000 CUSDT", "1", "trade;0;3920.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "392.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3920.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "391.0000 CUSDT", "1", "trade;0;3910.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "391.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3910.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "390.0000 CUSDT", "1", "trade;0;3900.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "390.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3900.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "389.0000 CUSDT", "1", "trade;0;3890.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "389.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3890.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);
   
   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "388.0000 CUSDT", "1", "trade;0;3880.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "388.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3880.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "387.0000 CUSDT", "1", "trade;0;3870.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "387.0000 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3870.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - quantity, buyer_quote_now);

   // efc push action relay.token trade '["testa", "codex.match", "btc1", "2.0000 CBTC", "1", "trade;0;3800.00 CUSDT;0;biosbpa;;2"]' -p testa
   quantity = asset::from_string( "2.0000 CBTC" );
   seller_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, seller);
   seller_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_base_before = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), escrow);
   push_action(rel_token_acc, name("trade"), seller, fc::mutable_variant_object()
           ("from", seller)
           ("to", escrow)
           ("chain", token1_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;3800.00 CUSDT;0;biosbpa;;2")
   );
   seller_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, seller);
   buyer_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, buyer);
   escrow_base_now = get_relay_token_currency_balance(rel_token_acc, token1_chain, pair1_base_sym, escrow);
   BOOST_REQUIRE_EQUAL(seller_quote_before + asset::from_string( "3124.0000 CUSDT" ), seller_quote_now);
   BOOST_REQUIRE_EQUAL(buyer_base_before + asset::from_string( "0.8000 CBTC" ), buyer_base_now);
   BOOST_REQUIRE_EQUAL(escrow_base_before + asset::from_string( "1.2000 CBTC" ), escrow_base_now);
} FC_LOG_AND_RETHROW() }

// eat bid orders continuously (eat all)
BOOST_AUTO_TEST_CASE( test9 ) { try {
   asset quantity, matched;
   asset seller_base_before, seller_base_now, seller_quote_before, seller_quote_now;
   asset buyer_base_before, buyer_base_now, buyer_quote_before, buyer_quote_now;
   asset escrow_base_before, escrow_base_now, escrow_quote_before, escrow_quote_now;
   symbol pair1_base_sym = asset::from_string(string(token1_max_supply)).get_symbol();
   symbol pair1_quote_sym = asset::from_string(string(token2_max_supply)).get_symbol();

   // efc push action relay.token trade '["testb", "codex.match", "usdt1", "4000.0001 CUSDT", "1", "trade;0;4000.0000 CUSDT;1;biosbpa;;2"]' -p testb
   quantity = asset::from_string( "4000.0001 CUSDT" );
   buyer_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_before = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, escrow);
   push_action(rel_token_acc, name("trade"), buyer, fc::mutable_variant_object()
           ("from", buyer)
           ("to", escrow)
           ("chain", token2_chain)
           ("quantity", quantity)
           ("type", trade_type)
           ("memo", "trade;0;4000.0000 CUSDT;1;biosbpa;;2")
   );
   buyer_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, quantity.get_symbol(), buyer);
   escrow_quote_now = get_relay_token_currency_balance(rel_token_acc, token2_chain, pair1_quote_sym, escrow);
   BOOST_REQUIRE_EQUAL(buyer_quote_before - asset::from_string( "4000.0000 CUSDT" ), buyer_quote_now);
   BOOST_REQUIRE_EQUAL(escrow_quote_before + asset::from_string( "4000.0000 CUSDT" ), escrow_quote_now);
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
