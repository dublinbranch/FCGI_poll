#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE NgnixUnitTest

#include <thread>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

#include "nginx_cmdline.hpp"
#include "common.hpp"

// globalish public vars out of lazyness
static std::string webserverLocation {"http://0.0.0.0/testingFCGX"};
static std::string fastcgiServer {"0.0.0.0"};
static std::string fastcgiPort {"2006"};

// basically commandline arguments for previous variables
BOOST_GLOBAL_FIXTURE( GlobalInit );

BOOST_AUTO_TEST_CASE( test001 ) {

 BOOST_TEST_MESSAGE( "\ntest001: Just a hello world library check");

 std::thread server_thread(server);
 std::thread client_thread(client);
 server_thread.join();
 client_thread.join();

 BOOST_CHECK( SUCCESS.load() );
}

