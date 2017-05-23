#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE NgnixUnitTest

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

#include "fcgiapp.h"

#define printf(...) FCGX_FPrintF(request->out, __VA_ARGS__)
#define get_param(KEY) FCGX_GetParam(KEY, request->envp)

// global out of lazyness
static std::atomic<bool> SUCCESS {false};
static std::atomic<unsigned> VALUE {0};

void handle_request(FCGX_Request *request) {
	char *input;
	unsigned value;
	char buffer[1024];
	std::memset(buffer, '\0', 1024);

	printf("Content-Type: text/plain\r\n\r\n");
	if ((input = get_param("QUERY_STRING")) != NULL) {
		std::memcpy(buffer, input, 16);
		printf(buffer); // manual test in a browser
		if( 1 == sprintf(buffer, "?value=%u", &value) ) { VALUE.store(value); }
	}
	printf("\n");
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

	std::this_thread::sleep_for(std::chrono::seconds(1));

	FCGX_Init(); 
	sock = FCGX_OpenSocket(":2005", 5);
	FCGX_InitRequest(&request, sock, 0);

	// just in case 
	VALUE.store(0);

	while ( (FCGX_Accept_r(&request) >= 0) && (VALUE.load() != 1)) {
		handle_request(&request);
		FCGX_Finish_r(&request);
	}
}

void client() {

	std::this_thread::sleep_for(std::chrono::seconds(2));

	if( check_response("http://0.0.0.0/testingFCGX?value=1", CURLE_OK) ) { 
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

