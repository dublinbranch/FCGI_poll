#pragma once

#include <string>
#include <regex>
#include <map>

// just logging something ( --log_level=message )
#include <boost/test/unit_test.hpp>

// globalish public vars out of lazyness
extern std::string webserver0Location;
extern std::string webserver1Location;
extern std::string fastcgiServer;
extern std::string fastcgi0Port;
extern std::string fastcgi1Port;

// command line info
struct GlobalInit {

  using Param = std::map<std::string, std::string>;

  GlobalInit() : argc(boost::unit_test_framework::framework::master_test_suite().argc),
		 argv(boost::unit_test_framework::framework::master_test_suite().argv)
  {
    Param param {commandline(argc, argv)};

    if(param["help"] == "help") { BOOST_TEST_MESSAGE("\n\tUsage " << param["PROGRAM"] <<
R"( [--log_level=message] -- [--help] [--nginx0_location=NGINX configured location at nginx.conf] [--nginx1_location=NGINX configured location at nginx.conf] [--fastcgi_server=ip for testing server] [--fastcgi0_port=port for testing server] [--fastcgi1_port=port for testing server]


	All those commandline arguments are optional provided NGINX is properly configured and running:

	  By default, --nginx0_location=http://0.0.0.0/testingFCGX0

	  By default, --nginx1_location=http://0.0.0.0/testingFCGX1

	  By default, --fastcgi_server=0.0.0.0

	  By default, --fastcgi0_port=2006

	  By default, --fastcgi1_port=2007

	Example of NGINX configuration:

	    location /testingFCGX0  {

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

	     location /testingFCGX1  {

		    fastcgi_pass   0.0.0.0:2007;
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

    if( param.cend() != param.find("nginx0_location") ) { webserver0Location = param["nginx0_location"]; }
    if( param.cend() != param.find("nginx1_location") ) { webserver1Location = param["nginx1_location"]; }
    if( param.cend() != param.find("fastcgi_server") ) { fastcgiServer = param["fastcgi_server"]; }
    if( param.cend() != param.find("fastcgi0_port") ) { fastcgi0Port = param["fastcgi0_port"]; }
    if( param.cend() != param.find("fastcgi1_port") ) { fastcgi1Port = param["fastcgi1_port"]; }

    BOOST_TEST_MESSAGE( "\nNGINX0 Location: " << webserver0Location);
    BOOST_TEST_MESSAGE( "\nNGINX1 Location: " << webserver1Location);
    BOOST_TEST_MESSAGE( "FastCGI Server: " << fastcgiServer);
    BOOST_TEST_MESSAGE( "FastCGI0 Port:   " << fastcgi0Port << "\n");
    BOOST_TEST_MESSAGE( "FastCGI1 Port:   " << fastcgi1Port << "\n");
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

