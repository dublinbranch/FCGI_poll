#pragma once

#include<string>

extern std::string webserverLocation;
extern std::string fastcgiServer;
extern std::string fastcgiPort;

static std::atomic<bool> SUCCESS {false};
static std::atomic<unsigned> VALUE {0};

#include <curl/curl.h>
#include "fcgiapp.h"

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
