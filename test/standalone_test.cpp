#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE StandAloneUnitTest

#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <regex>
#include <map>
#include <stdexcept>
#include <array>
#include <curl/curl.h>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

#include "fcgiapp.h"

// globalish public vars out of lazyness
static std::string webserverLocation {"0.0.0.0:9000"};
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
R"( [--log_level=message] -- [--help] [--fast==webserver_location=http webserver automatically launched by this test] [--fastcgi_server=ip for testing server] [--fastcgi_port=port for testing server] [--tool=fastcgi-serve full path if needed]

	All those commandline arguments are optional provided golang testing binary fastcgi-serve tool is properly installed (done by cmake or by 'go get github.com/beberlei/fastcgi-serve'):

	  By default, --websever_location=0.0.0.0:9000

	  By default, --fastcgi_server=0.0.0.0

	  By default, --fastcgi_port=2006

	  By default, --tool=fastcgi-serve

	So fastcgi-serve internally usage invoked by this test will be:

	    fastcgi-serve --listen=0.0.0.0:9000 --server=0.0.0.0 --server-port=2006

	Further info at https://github.com/beberlei/fastcgi-serve

)"); exit(0); }

    if( param.cend() != param.find("webserver_location") ) { webserverLocation = param["webserver_location"]; }
    if( param.cend() != param.find("fastcgi_server") ) { fastcgiServer = param["fastcgi_server"]; }
    if( param.cend() != param.find("fastcgi_port") ) { fastcgiPort = param["fastcgi_port"]; }

    BOOST_TEST_MESSAGE( "\nWebServer Location: " << webserverLocation);
    BOOST_TEST_MESSAGE( "FastCGI Server: " << fastcgiServer);
    BOOST_TEST_MESSAGE( "FastCGI Port:   " << fastcgiPort << "\n");

    std::string tool{TOOL};
    if( param.cend() != param.find("tool") ) { tool = param["tool"]; }

    // exists that tool??
    std::string found = exec(std::string{"which " + tool}.c_str());
    if( std::string::npos != found.find("which: no ") ) {
	BOOST_TEST_MESSAGE( "Not found " << tool << ". Resorting to hardcoded alternative " << ALTERNATIVE_PATH << "\n" );
	tool = ALTERNATIVE_PATH;
	found = exec(std::string{"which " + tool}.c_str());
	if( std::string::npos != found.find("which: no ") ) {
		BOOST_TEST_MESSAGE( "Not found alternative " << tool << " either. Aborting execution\n" );
		BOOST_FAIL( true );
		exit(1);
	}
    }
    BOOST_TEST_MESSAGE( "Using tool " << tool << "\n");

    // launch that tool to be killed by the destructor
    // fastcgi-serve --listen=0.0.0.0:9000 --server=0.0.0.0 --server-port=2006
    std::string cmd = tool + "  --listen=" + webserverLocation + " --server=" + fastcgiServer + " --server-port=" + fastcgiPort + " &";
    if( 0 != std::system(cmd.c_str())) {
	      BOOST_TEST_MESSAGE( "Unable to launch " << cmd << ". Aboring execution\n" );
	      BOOST_FAIL( true );
	      exit(1);
    } else {
	      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time to start
	      std::string process ="ps -edf | grep " + tool + " | grep -v grep | awk '{print $2}'";
	      pid = exec(process.c_str());
	      pid.erase(pid.find_last_not_of(" \n\r\t")+1);
	      BOOST_TEST_MESSAGE( "\nUsing " << tool << " [" << pid << "]\n" );
    }
  }

~GlobalInit() {
     if( not pid.empty() ) {
	std::string cmd = "kill -9 " + pid;
	std::string kill = exec(cmd.c_str());
	BOOST_TEST_MESSAGE( "\nKilled " << TOOL << " process " << pid << "\n");
     }
}

private:
  int argc {};
  char **argv {nullptr};
  std::string pid {""};
  const std::string TOOL{"fastcgi-serve"};
  const std::string ALTERNATIVE_PATH{"/mnt/megaswap/compilationDep/tools/fastcgi_serve_tool/fastcgi-serve"};

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

  std::string exec(const char* cmd) {
     std::array<char, 128> buffer;
     std::string result;
     std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
     if (!pipe) throw std::runtime_error("popen() failed!");
     while (!feof(pipe.get())) {
	 if (fgets(buffer.data(), 128, pipe.get()) != NULL)
	     result += buffer.data();
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

	if( check_response((webserverLocation + "?value=1").c_str(), CURLE_OK) ) {
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

