#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE StandAloneUnitTest

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
R"( [--log_level=message] -- [--help] [--webserver0_location=http webserver automatically launched by this test] [--webserver1_location=http webserver automatically launched by this test] [--fastcgi_server=ip for testing server] [--fastcgi0_port=port for testing server] [--fastcgi1_port=port for testing server] [--tool=fastcgi-serve full path if needed]

	All those commandline arguments are optional provided golang testing binary fastcgi-serve tool is properly installed (done by cmake or by 'go get github.com/beberlei/fastcgi-serve'):

	  By default, --websever0_location=0.0.0.0:9000

	  By default, --websever1_location=0.0.0.0:9001

	  By default, --fastcgi_server=0.0.0.0

	  By default, --fastcgi0_port=2006

	  By default, --fastcgi1_port=2007

	  By default, --tool=fastcgi-serve

	So fastcgi-serve internally usage invoked by this test will be:

	    fastcgi-serve --listen=0.0.0.0:9000 --server=0.0.0.0 --server-port=2006

	Further info at https://github.com/beberlei/fastcgi-serve

)"); exit(0); }

    if( param.cend() != param.find("webserver0_location") ) { webserver0Location = param["webserver0_location"]; }
    if( param.cend() != param.find("webserver1_location") ) { webserver1Location = param["webserver1_location"]; }
    if( param.cend() != param.find("fastcgi_server") ) { fastcgiServer = param["fastcgi_server"]; }
    if( param.cend() != param.find("fastcgi0_port") ) { fastcgi0Port = param["fastcgi0_port"]; }
    if( param.cend() != param.find("fastcgi1_port") ) { fastcgi1Port = param["fastcgi1_port"]; }

    BOOST_TEST_MESSAGE( "\nWebServer0 Location: " << webserver0Location);
    BOOST_TEST_MESSAGE( "\nWebServer1 Location: " << webserver1Location);
    BOOST_TEST_MESSAGE( "FastCGI Server: " << fastcgiServer);
    BOOST_TEST_MESSAGE( "FastCGI Port0:   " << fastcgi0Port << "\n");
    BOOST_TEST_MESSAGE( "FastCGI Port1:   " << fastcgi1Port << "\n");

    std::string tool{TOOL};
    if( param.cend() != param.find("tool") ) { tool = param["tool"]; }

    // exists that tool??
    std::string found = exec(std::string{"which " + tool}.c_str());
    if( std::string::npos != found.find("which: no ") ) {
	BOOST_TEST_MESSAGE( "Not found " << tool << ". Resorting to hardcoded alternative " << ALTERNATIVE_PATH << "\n" );
	tool = ALTERNATIVE_PATH + TOOL;
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
    std::string cmd = tool + " --listen=" + webserver0Location + " --server=" + fastcgiServer + " --server-port=" + fastcgi0Port;
    std::string background = cmd + " &";
    if( 0 != std::system(background.c_str())) {
	      BOOST_TEST_MESSAGE( "Unable to launch " << cmd << ". Aboring execution\n" );
	      BOOST_FAIL( true );
	      exit(1);
    } else {
	      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time to start
	      std::string process ="ps -edf | grep \"" + cmd+ "\" | grep -v grep | awk '{print $2}'";
	      pid0 = exec(process.c_str());
	      pid0.erase(pid0.find_last_not_of(" \n\r\t")+1);
	      BOOST_TEST_MESSAGE( "\nUsing " << cmd << " [" << pid0 << "]\n" );
    }

    // launch that tool to be killed by the destructor
    // fastcgi-serve --listen=0.0.0.0:9001 --server=0.0.0.0 --server-port=2007
    cmd = tool + " --listen=" + webserver1Location + " --server=" + fastcgiServer + " --server-port=" + fastcgi1Port;
    background = cmd + " &";
    if( 0 != std::system(background.c_str())) {
	      BOOST_TEST_MESSAGE( "Unable to launch " << cmd << ". Aboring execution\n" );
	      BOOST_FAIL( true );
	      exit(1);
    } else {
	      std::this_thread::sleep_for(std::chrono::milliseconds(100)); // give some time to start
	      std::string process ="ps -edf | grep \"" + cmd + "\" | grep -v grep | awk '{print $2}'";
	      pid1 = exec(process.c_str());
	      pid1.erase(pid1.find_last_not_of(" \n\r\t")+1);
	      BOOST_TEST_MESSAGE( "\nUsing " << cmd << " [" << pid1 << "]\n" );
    }

  }

~GlobalInit() {
     if( not pid0.empty() ) {
	std::string cmd = "kill -9 " + pid0;
	std::string kill = exec(cmd.c_str());
	BOOST_TEST_MESSAGE( "\nKilled " << TOOL << " process " << pid0 << ": " << kill << "\n");

	cmd = "kill -9 " + pid1;
	kill = exec(cmd.c_str());
	BOOST_TEST_MESSAGE( "\nKilled " << TOOL << " process " << pid1 << ": " << kill << "\n");
     }
}

private:
  int argc {};
  char **argv {nullptr};
  std::string pid0 {""};
  std::string pid1 {""};
  const std::string TOOL{"fastcgi-serve"};
  const std::string ALTERNATIVE_PATH{"/mnt/megaswap/compilationDep/tools/fastcgi_serve_tool/"};

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

