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

extern std::string webserver0Location;
extern std::string webserver1Location;
extern std::string fastcgiServer;
extern std::string fastcgi0Port;
extern std::string fastcgi1Port;
extern const size_t N;

static constexpr const size_t DELAY {100};

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
	sock = FCGX_OpenSocket((fastcgiServer + ":" + fastcgi0Port).c_str(), 5);
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

	std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

	if( check_response((webserver0Location + "?value=1").c_str(), CURLE_OK) ) {
		SUCCESS.store(true);
	} else { 
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

static std::atomic<bool> LONG_SUCCESS {false};
static std::atomic<bool> LONG_VALUE {false};

// long query request
static char request_buffer[USHRT_MAX]; // 65535

void handle_long_request(FCGX_Request *request) {
	char *input;
	FCGX_FPrintF(request->out, "Content-Type: text/plain\r\n\r\n");
	if ((input = FCGX_GetParam("QUERY_STRING", request->envp)) != NULL) {
		char value[N];
		FCGX_FPrintF(request->out, input); // manual test in a browser
		{
		    if( 1 == sscanf(input, "long=%s", &value) ) { LONG_VALUE.store(std::string{request_buffer} == std::string{value}); }
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
  success = ((res == expected) && LONG_VALUE.load());
  curl_easy_cleanup(curl);
 }
 return success;
}

void long_server() {
	int sock;
	FCGX_Request request;

	FCGX_Init();
	sock = FCGX_OpenSocket((fastcgiServer + ":" + fastcgi1Port).c_str(), 5);
	FCGX_InitRequest(&request, sock, 0);

	// just in case
	LONG_VALUE.store(false);

	while (FCGX_Accept_r(&request) >= 0) {
		handle_long_request(&request);
		FCGX_Finish_r(&request);
		if( LONG_VALUE.load() ) { break; }
	}
}

void long_client() {

	std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));

	if( check_long_response((webserver1Location + "?long=" + std::string(request_buffer)).c_str(), CURLE_OK) ) {
		LONG_SUCCESS.store(true);
	} else {
		LONG_SUCCESS.store(false);
	}
}

bool longQuery() {

	// random long request & response
	std::random_device r;
	std::seed_seq seed{ r(), r(), r(), r(), r(), r(), r(), r()};
	std::generate_n(request_buffer, N, std::bind(std::uniform_int_distribution<>('A', 'Z'), std::mt19937(seed)));

	std::thread server_thread(long_server);
	std::thread client_thread(long_client);
	server_thread.join();
	client_thread.join();

	return LONG_SUCCESS.load();

	//return true;
}
