cmake_minimum_required(VERSION 3.0)
set(LOCAL_CMAKE_PROJECT_NAME ModifiedFastCGI)
project(${LOCAL_CMAKE_PROJECT_NAME} NONE)

############################
# compilation dependencies #
############################
if(EXISTS $ENV{COMPILATIONDEP})
    set(COMPILATIONDEP "$ENV{COMPILATIONDEP}")
    message(STATUS "Using environment variable COMPILATIONDEP=${COMPILATIONDEP} as compilationDep")
elseif(EXISTS "/mnt/swap/compilationDep")
    set(COMPILATIONDEP "/mnt/swap/compilationDep")
    message(STATUS "Using default mounted folder ${COMPILATIONDEP} as compilationDep")
elseif(EXISTS "/mnt/megaswap/compilationDep")
    set(COMPILATIONDEP "/mnt/megaswap/compilationDep")
    message(STATUS "Using default mounted folder ${COMPILATIONDEP} as compilationDep")
elseif(EXISTS "/opt/compilationDep")
    set(COMPILATIONDEP "/opt/compilationDep")
    message(STATUS "Using default local folder ${COMPILATIONDEP} as compilationDep")
else()
    message(STATUS "Missing compilation dependencies folder, i.e. /mnt/swap/compilationDep or /mnt/megaswap/compilationDep")
endif()

####################################
# build/install/pack instructions  #
####################################
if(APPLE)
   set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
   project(${LOCAL_CMAKE_PROJECT_NAME} C CXX)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -g")
   set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -std=c++1z -g")
elseif(UNIX)
  find_program(LSB_RELEASE lsb_release)
  execute_process(COMMAND ${LSB_RELEASE} -is OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
  if("${LSB_RELEASE_ID_SHORT}" MATCHES "Debian") # Supposed be at home so default g++ is already version 6
   set(STATIC_BOOST "${COMPILATIONDEP}/builtBoost/debianGCC6")
   set(SPECIAL_AR "gcc-ar")
   project(${LOCAL_CMAKE_PROJECT_NAME} C CXX)
  else() # Supposed be at work, opensuse, where default g++ is old 4.x
   set(CMAKE_CXX_COMPILER "/usr/bin/g++-6")
   set(SPECIAL_AR "gcc-ar-6")
   set(STATIC_BOOST "${COMPILATIONDEP}/builtBoost/openSuseGCC6")
   project(${LOCAL_CMAKE_PROJECT_NAME} C CXX)
  endif("${LSB_RELEASE_ID_SHORT}" MATCHES "Debian")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC -std=gnu++1z -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fPIC -std=gnu++1z -g")
endif(APPLE)

##################################
# building static lib under test #
##################################
set(STATIC_LIB_NAME "fgci")
file(GLOB MODIFIED_FASTCGI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../libfcgi/*.c*)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_library(${STATIC_LIB_NAME} ${MODIFIED_FASTCGI_SOURCES})

##############################
# tests using boost framwork #
##############################
if(EXISTS ${STATIC_BOOST})

  # static compilation for builtBoost libs
  set(Boost_NO_SYSTEM_PATHS TRUE)
  set(BOOST_ROOT "${STATIC_BOOST}")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)

else()

  # dynamic compilation for local libs
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_TEST_DYN_LINK")

endif(EXISTS ${STATIC_BOOST})

### boost ####
message(STATUS "Using boost at ${BOOST_ROOT} with link_dir=${Boost_LIBRARY_DIRS}")
find_package( Boost 1.54.0 REQUIRED COMPONENTS unit_test_framework )
if(Boost_FOUND)
 
  # enable CMake tests
  enable_testing()
 
  # main test
  set(TEST_NAME "fastcgi_test")
  file(GLOB TEST_SOURCES test.cpp)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
  link_directories(${CMAKE_CURRENT_BINARY_DIR})
  add_executable(${TEST_NAME} ${TEST_SOURCES})
  add_dependencies(${TEST_NAME} ${STATIC_LIB_NAME})
  target_link_libraries(${TEST_NAME} ${STATIC_LIB_NAME} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  add_test(UnitTest ${TEST_NAME})

else()
  message(STATUS "Missing required Boost UNIT TEST framework library. Test building skipped")
endif(Boost_FOUND)

