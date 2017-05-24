#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE NgnixUnitTest

#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <regex>
#include <map>

#include <curl/curl.h>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

#include "fcgiapp.h"

// globalish public vars out of lazyness
static std::string nginxLocation {"http://0.0.0.0/testingFCGX"};
static std::string fastcgiServer {"0.0.0.0"};
static std::string fastcgiPort {"2006"};
static std::atomic<bool> SUCCESS {false};
static std::atomic<unsigned> VALUE {0};

// command line info
struct GlobalInit {

  using Param = std::map<std::string, std::string>;

  GlobalInit() : argc(boost::unit_test_framework::framework::master_test_suite().argc),
		 argv(boost::unit_test_framework::framework::master_test_suite().argv)
  {
    Param param {commandline(argc, argv)};

    if(param["help"] == "help") { BOOST_TEST_MESSAGE("\n\tUsage " << param["PROGRAM"] <<
R"( [--log_level=message] -- [--help] [--nginx_location=NGINX configured location at nginx.conf] [--fastcgi_server=ip for testing server] [--fastcgi_port=port for testing server]

	All those commandline arguments are optional provided NGINX is properly configured and running:

	  By default, --nginx_location=http://0.0.0.0/testingFCGX

	  By default, --fastcgi_server=0.0.0.0

	  By default, --fastcgi_port=2006

	Example of NGINX configuration:

	    location /testingFCGX  {

		    fastcgi_pass   0.0.0.0:2006;
		    fastcgi_connect_timeout 5h;
		    fastcgi_read_timeout 5h;

		    fastcgi_param  QUERY_STRING       $query_string;
		    fastcgi_param  REQUEST_METHOD     $request_method;
		    fastcgi_param  CONTENT_TYPE       $content_type;
		    fastcgi_param  CONTENT_LENGTH     $content_length;
		    fastcgi_param  REQUEST            $request;
		    fastcgi_param  REQUEST_BODY       $request_body;
		    fastcgi_param  REQUEST_URI        $request_uri;
		    fastcgi_param  DOCUMENT_URI       $document_uri;
		    fastcgi_param  DOCUMENT_ROOT      $document_root;
		    fastcgi_param  SERVER_PROTOCOL    $server_protocol;
		    fastcgi_param  REMOTE_ADDR        $remote_addr;
		    fastcgi_param  REMOTE_PORT        $remote_port;
		    fastcgi_param  SERVER_ADDR        $server_addr;
		    fastcgi_param  SERVER_PORT        $server_port;
		    fastcgi_param HTTP_REFERER        $http_referer;
		    fastcgi_param SCHEME              $scheme;
	      }

)"); exit(0); }

    if( param.cend() != param.find("nginx_location") ) { nginxLocation = param["nginx_location"]; }
    if( param.cend() != param.find("fastcgi_server") ) { fastcgiServer = param["fastcgi_server"]; }
    if( param.cend() != param.find("fastcgi_port") ) { fastcgiPort = param["fastcgi_port"]; }

    BOOST_TEST_MESSAGE( "\nNGINX Location: " << nginxLocation);
    BOOST_TEST_MESSAGE( "FastCGI Server: " << fastcgiServer);
    BOOST_TEST_MESSAGE( "FastCGI Port:   " << fastcgiPort << "\n");
  }

private:
  int argc {};
  char **argv {nullptr};

  Param commandline(const int argc = 0, char** argv = nullptr) const
  {
	  Param result{};
	  if(argc > 0) { result.emplace("PROGRAM", argv[0]); }

	  static const std::regex param_pattern{"^-[-]?(.*)[ ]*=[ ]*(.*)$"};
	  static const std::regex help_pattern{"^-[-]?help$"};
	  for(int i=1; i<argc; ++i) {
		 std::string candidate{argv[i]};
		 std::smatch param_match;
		 if(std::regex_match(candidate, param_match, help_pattern)) {
			 result.emplace("help", "help");
			 continue;
		 }
		 if(std::regex_match(candidate, param_match, param_pattern)) {
			 if( param_match.size() == 3) {
				 result.emplace(param_match[1].str(), param_match[2].str());
			 }
		 }
	  }

	  return result;
  }

};
BOOST_GLOBAL_FIXTURE( GlobalInit );

void handle_request(FCGX_Request *request) {
	char *input;
	FCGX_FPrintF(request->out, "Content-Type: text/plain\r\n\r\n");
	if ((input = FCGX_GetParam("QUERY_STRING", request->envp)) != NULL) {
		unsigned value;
		FCGX_FPrintF(request->out, input); // manual test in a browser
		if( 1 == sscanf(input, "value=%u", &value) ) { VALUE.store(value); }
	}
	FCGX_FPrintF(request->out, "\n");
}

bool check_response(const char* const query, const CURLcode expected) {
 bool success {false};
 CURL *curl = curl_easy_init();
 if(curl) {
  CURLcode res;
  curl_easy_setopt(curl, CURLOPT_URL, query);
  res = curl_easy_perform(curl);
  success = (res == expected) && (VALUE.load() == 1);
  curl_easy_cleanup(curl);
 }
 return success;
}

void server() {
	int sock;
	FCGX_Request request;

	FCGX_Init(); 
	sock = FCGX_OpenSocket((fastcgiServer + ":" + fastcgiPort).c_str(), 5);
	FCGX_InitRequest(&request, sock, 0);

	// just in case 
	VALUE.store(0);

	while (FCGX_Accept_r(&request) >= 0) {
		handle_request(&request);
		FCGX_Finish_r(&request);
		if( VALUE.load() == 1 ) { break; }
	}
}

void client() {

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if( check_response((nginxLocation + "?value=1").c_str(), CURLE_OK) ) {
 		BOOST_TEST_MESSAGE( "Seems OK");
		SUCCESS.store(true);
	} else { 
		BOOST_TEST( "Failed FCGX" ); 
		SUCCESS.store(false);
	} 
}

BOOST_AUTO_TEST_CASE( test001 ) {

 BOOST_TEST_MESSAGE( "\ntest001: Just a hello world library check");

 std::thread server_thread(server);
 std::thread client_thread(client);
 server_thread.join();
 client_thread.join();

 BOOST_CHECK( SUCCESS.load() );
}

