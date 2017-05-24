#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <functional>
#include <algorithm>
#include <random>
#include <climits>
#include <cstring>

#include <curl/curl.h>
#include "fcgiapp.h"

extern std::string webserverLocation;
extern std::string fastcgiServer;
extern std::string fastcgiPort;

///////////// hello world //////////////////

static std::atomic<bool> SUCCESS {false};
static std::atomic<unsigned> VALUE {0};

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

bool simpleQuery() {

	std::thread server_thread(server);
	std::thread client_thread(client);
	server_thread.join();
	client_thread.join();

	return SUCCESS.load();
}

///////////////// long query //////////////////////

static constexpr const size_t N=10000; // long query
static std::mutex mutex_;

// long query request
static unsigned char request_buffer[N];
char request_str[N];

// long query response
static unsigned char response_buffer[N];
char response_str[N];

void handle_long_request(FCGX_Request *request) {
	char *input;
	FCGX_FPrintF(request->out, "Content-Type: text/plain\r\n\r\n");
	if ((input = FCGX_GetParam("QUERY_STRING", request->envp)) != NULL) {
		char value[N];
		FCGX_FPrintF(request->out, input); // manual test in a browser
		{
		    std::lock_guard<std::mutex> lock(mutex_);
		    if( 1 == sscanf(input, "long=%s", &response_str) ) { ; }
		}
	}
	FCGX_FPrintF(request->out, "\n");
}

bool check_long_response(const char* const query, const CURLcode expected) {
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

void long_server() {
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

void long_client() {

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if( check_response((webserverLocation + "?value=1").c_str(), CURLE_OK) ) {
		BOOST_TEST_MESSAGE( "Seems OK");
		SUCCESS.store(true);
	} else {
		BOOST_TEST( "Failed FCGX" );
		SUCCESS.store(false);
	}
}

bool longQuery() {

	// random long request & response
	std::random_device r;
	std::seed_seq seed0{ r(), r(), r(), r(), r(), r(), r(), r()};
	std::generate_n(request_buffer, N, std::bind(std::uniform_int_distribution<>(0, UCHAR_MAX), std::mt19937(seed0)));
	std::seed_seq seed1{ r(), r(), r(), r(), r(), r(), r(), r()};
	std::generate_n(response_buffer, N, std::bind(std::uniform_int_distribution<>(0, UCHAR_MAX), std::mt19937(seed1)));

	return (0 != std::memcmp(request_buffer, response_buffer, N));
}
