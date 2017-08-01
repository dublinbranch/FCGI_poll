cmake_minimum_required(VERSION 3.0)
set(LOCAL_CMAKE_PROJECT_NAME FCGI_poll)
project(${LOCAL_CMAKE_PROJECT_NAME} CXX)

#########################################################
# Helper to choose libraries when several are available #
#########################################################

function(set_ar)
  if(AR)
     set(CMAKE_AR "${AR}")
  else()
     ### try to define ar tool from gcc name ###
     get_filename_component(GCC_DIRECTORY "${CMAKE_C_COMPILER}" DIRECTORY)
     get_filename_component(GCC_NAME "${CMAKE_C_COMPILER}" NAME)
     string(REGEX MATCH "-" GCC_WITH_DASH "${GCC_NAME}")
     if(GCC_WITH_DASH)
        message(STATUS "Trying to automatically define AR utility")
        string(REPLACE "-" "-ar-" SPECIAL_AR "${GCC_NAME}")
        set(SPECIAL_AR "${GCC_DIRECTORY}/${SPECIAL_AR}")
        find_program(SPECIAL_AR ${SPECIAL_AR} HINTS ${SPECIAL_AR})
        set(CMAKE_AR "${SPECIAL_AR}" PARENT_SCOPE)
        if(NOT EXISTS "${CMAKE_AR}")
          message(STATUS "AR utility not found at ${CMAKE_AR}")
          set(CMAKE_AR "gcc-ar" PARENT_SCOPE)
          message(WARNING "AR utility unable to be automatically set. Resorting to ${CMAKE_AR}")
          message(STATUS "AR utility could be defined at environment variable AR")
        endif(NOT EXISTS "${CMAKE_AR}")
     else()
        set(CMAKE_AR "gcc-ar" PARENT_SCOPE)
        message(WARNING "AR utility unable to be automatically set. Resorting to ${CMAKE_AR}")
        message(STATUS "AR utility could be defined at environment variable AR")
     endif(GCC_WITH_DASH)
  endif(AR)
endfunction(set_ar)

####################################
# build/install/pack instructions  #
####################################

### Needed for special build ###
if($ENV{AR})
  set(AR "${ENV{AR}")
endif()

### This can be placed in a specif CMake config file for each machine and use "find_package" ###
if(APPLE)

   message(FATAL_ERROR "macOS is not supported for Gaisensha yet")

elseif(UNIX)

 find_program(LSB_RELEASE lsb_release)
 execute_process(COMMAND ${LSB_RELEASE} -is OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
 message(STATUS "Linux distro: ${LSB_RELEASE_ID_SHORT}")

 if("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")
 
  ### gcc should be good enough for c++1z ###
  if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0)
    message(FATAL_ERROR "Require at least g++ 6.0")
  endif() 

  ### Needed for special build ###
  set_ar()                                                                                                                                                                                                                                                                     
                                                                                                                                                                                                                                                                               
  if("${LSB_RELEASE_ID_SHORT}" MATCHES "openSUSE") # Supposed be at work, opensuse, where default g++ is old 4.x                                                                                                                                                               
   message(STATUS "Officially supported GNU linux build")                                                                                                                                                                                                                      
    else() # Supposed to use the default one there                                                                                                                                                                                                                             
   message(WARNING "Non officially supported GNU linux build")                                                                                                                                                                                                                 
  endif("${LSB_RELEASE_ID_SHORT}" MATCHES "openSUSE")                                                                                                                                                                                                                          
 else()                                                                                                                                                                                                                                                                        
   message(WARNING "Non officially supported ${CMAKE_C_COMPILER_ID} linux build")                                                                                                                                                                                              
 endif("${CMAKE_C_COMPILER_ID}" MATCHES "GNU")                                                                                                                                                                                                                                 
                                                                                                                                                                                                                                                                               
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC -std=gnu++1z -fno-omit-frame-pointer -ftls-model=initial-exec")                                                                                                                                                        
 set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -pthread -fPIC -std=gnu++1zi -fno-omit-frame-pointer -ftls-model=initial-exec")                                                                                                                                                 
                                                                                                                                                                                                                                                                               
endif(APPLE)      

##########################################
# CXXFLAGS/LFLAGS external configuration #
##########################################

if(CXXFLAGS_RELEASE_FLAG)
    set(CMAKE_BUILD_TYPE "RELEASE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    message(STATUS "${LOCAL_CMAKE_PROJECT_NAME}:  release CXXFLAGS [-O2]")
elseif(CXXFLAGS_NATIVE_FLAG)
    set(CMAKE_BUILD_TYPE "NATIVE")
    if(APPLE)
        message(FATAL_ERROR "CXXFLAGS_NATIVE_FLAG, linking time optimization, not configured for macOS")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -march=native")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_FINISH   true)
    message(STATUS "${LOCAL_CMAKE_PROJECT_NAME}:  native CXXFLAGS [-O2 -g -march=native]")
elseif(CXXFLAGS_OPTIMIZED_FLAG)
    set(CMAKE_BUILD_TYPE "OPTIMIZED")
    if(APPLE)
        message(FATAL_ERROR "CXXFLAGS_OPTIMIZED_FLAG, linking time optimization, not configured for macOS")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=haswell -flto -fuse-ld=gold -fuse-linker-plugin")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_FINISH   true) 
    message(STATUS "${LOCAL_CMAKE_PROJECT_NAME}:  optimized CXXFLAGS [-O2 -march=haswell -flto -fuse-ld=gold -fuse-linker-plugin]")
else() # default --> standard
    set(CMAKE_BUILD_TYPE "STANDARD")
    set(CXXFLAGS_STANDARD_FLAG true)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0  -g -Wunused")
    message(STATUS "${LOCAL_CMAKE_PROJECT_NAME}:  standard CXXFLAGS [-O0 -g -Wunused]")
endif()
if(SANITIZE_ADDRESS_FLAG)
    set(CMAKE_BUILD_TYPE "SANITIZE_ADDRESS ${CMAKE_BUILD_TYPE}")
    set(CMAKE_COMPILER_FLAGS "${CMAKE_COMPILER_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
elseif(SANITIZE_UNDEFINED_FLAG)
    set(CMAKE_BUILD_TYPE "SANITIZE_UNDEFINED ${CMAKE_BUILD_TYPE}")
    set(CMAKE_COMPILER_FLAGS "${CMAKE_COMPILER_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS}  -fsanitize=undefined")
endif()

message(STATUS "${LOCAL_CMAKE_PROJECT_NAME} AR utility  : ${CMAKE_AR}")
message(STATUS "${LOCAL_CMAKE_PROJECT_NAME} C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "${LOCAL_CMAKE_PROJECT_NAME} C++ flags   : ${CMAKE_CXX_FLAGS}")
message(STATUS "${LOCAL_CMAKE_PROJECT_NAME} linker flags: ${CMAKE_LINKER_FLAGS}")
message(STATUS "${LOCAL_CMAKE_PROJECT_NAME} shared library linker flags: ${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}")

######################
# Create the library #
######################

set(STATIC_LIB_NAME "fcgi")
file(GLOB MODIFIED_FASTCGI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libfcgi/*.c*)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(${STATIC_LIB_NAME} ${MODIFIED_FASTCGI_SOURCES})

###########
# Testing #
###########
# TODO: include testing at http://s2.chameleon.ad:3000/Chameleon/FCGI_poll/src/master/test
