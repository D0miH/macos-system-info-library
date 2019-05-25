project(SMCKitCPP)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 14)

# get IOKit
find_library(IOKit IOKit)
if(NOT IOKit)
    message(FATAL_ERROR "IOKit not found")
endif()

ADD_LIBRARY(SMCKit SMCKit.h SMCKit.cpp)

add_executable(SMCKitCPP main.cpp)

target_link_libraries(SMCKitCPP SMCKit ${IOKit})