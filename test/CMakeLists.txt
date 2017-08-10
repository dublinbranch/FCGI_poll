##############################
# tests using boost framwork #
##############################

### boost ####
message(STATUS "Using boost at ${BOOST_ROOT} with link_dir=${Boost_LIBRARY_DIRS}")
find_package( Boost 1.54.0 REQUIRED COMPONENTS unit_test_framework )
if(Boost_FOUND)

  # avoid to compile again and again boost unit test framework info
  set(UNIT_TEST_OBJECT_LIBRARY "${LOCAL_CMAKE_PROJECT_NAME}_unit_test_object")
  add_library(${UNIT_TEST_OBJECT_LIBRARY} OBJECT common.cpp) 

  if(${LOCAL_CMAKE_PROJECT_NAME}_NGINX)

	set(TEST_NAME "nginx_fastcgi_test")

	# main test based on NGINX
	file(GLOB TEST_SOURCES nginx_test.cpp)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
	link_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	add_executable(${TEST_NAME} ${TEST_SOURCES} $<TARGET_OBJECTS:${OBJECT_LIBRARY}> $<TARGET_OBJECTS:${UNIT_TEST_OBJECT_LIBRARY}>)
	add_dependencies(${TEST_NAME} ${OBJECT_LIBRARY} ${UNIT_TEST_OBJECT_LIBRARY})
	target_link_libraries(${TEST_NAME} curl)
	add_test(NginxUnitTest ${TEST_NAME})

  else(${LOCAL_CMAKE_PROJECT_NAME}_NGINX)

	# Configuration for golang fastcgi testing server: https://github.com/beberlei/fastcgi-serve
	#
	# The following settings are available:
	#
	#    --document-root - The document root to serve files from (default: current working directory)
	#    --listen - The webserver bind address to listen to (default:127.0.0.1)
	#    --server - The FastCGI server to listen to
	#    --server-port The FastCGI server port to listen to
	#    --index The default script to call when request path cannot be served with an existing file

	find_program(LOCAL_GO_COMPILER go HINTS /home /usr)
	message(STATUS "Go compiler: ${LOCAL_GO_COMPILER}")

	if(EXISTS ${LOCAL_GO_COMPILER})

		set(TEST_NAME "standalone_fastcgi_test")

		# Check out libraries needed
		add_custom_target(${TEST_NAME}_libs
			COMMAND ${LOCAL_GO_COMPILER} get "github.com/beberlei/fastcgi-serve"
		)

		# main test based on fastcgi-serve
		file(GLOB TEST_SOURCES standalone_test.cpp)
		include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
		link_directories(${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
		add_executable(${TEST_NAME} ${TEST_SOURCES} $<TARGET_OBJECTS:${OBJECT_LIBRARY}> $<TARGET_OBJECTS:${UNIT_TEST_OBJECT_LIBRARY}>)
		add_dependencies(${TEST_NAME} ${OBJECT_LIBRARY} ${UNIT_TEST_OBJECT_LIBRARY} ${TEST_NAME}_libs)
		target_link_libraries(${TEST_NAME} curl)
		add_test(StandAloneUnitTest ${TEST_NAME})

	else(EXISTS ${LOCAL_GO_COMPILER})

	     message(STATUS "No golang compiler means no ${TEST_NAME}")

	endif(EXISTS ${LOCAL_GO_COMPILER})

  endif(${LOCAL_CMAKE_PROJECT_NAME}_NGINX)

else()
  message(STATUS "Missing required Boost UNIT TEST framework library. Test building skipped")
endif(Boost_FOUND)

