#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE StandAloneUnitTest

#include <thread>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

#include "standalone_cmdline.hpp"
#include "common.hpp"

// globalish public vars out of lazyness
std::string webserver0Location {"0.0.0.0:9000"};
std::string webserver1Location {"0.0.0.0:9001"};
std::string fastcgiServer {"0.0.0.0"};
std::string fastcgi0Port {"2006"};
std::string fastcgi1Port {"2007"};

const size_t N = 10000;

// basically commandline arguments for previous variables
BOOST_GLOBAL_FIXTURE( GlobalInit );

BOOST_AUTO_TEST_CASE( test000 ) {

 BOOST_TEST_MESSAGE( "\ntest000: Just a hello world library check");
 BOOST_CHECK( simpleQuery() );
}

BOOST_AUTO_TEST_CASE( test001 ) {

 BOOST_TEST_MESSAGE( "\ntest001: Long query");
 BOOST_CHECK( longQuery() );
}

