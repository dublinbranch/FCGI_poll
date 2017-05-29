#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE NgnixUnitTest

#include <thread>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

#include "nginx_cmdline.hpp"
#include "common.hpp"

// globalish public vars out of lazyness
std::string webserver0Location {"http://0.0.0.0/testingFCGX"};
std::string webserver1Location {"http://0.0.0.0/testingFCGX"};
std::string fastcgiServer {"0.0.0.0"};
std::string fastcgi0Port {"2006"};
std::string fastcgi1Port {"2007"};

// basically commandline arguments for previous variables
BOOST_GLOBAL_FIXTURE( GlobalInit );

BOOST_AUTO_TEST_CASE( test000 ) {

 BOOST_TEST_MESSAGE( "\ntest000: Just a hello world library check");
 BOOST_CHECK( simpleQuery() );
}

/*
BOOST_AUTO_TEST_CASE( test001 ) {

 BOOST_TEST_MESSAGE( "\ntest001: Long query");
 BOOST_CHECK( longQuery() );
}
*/
