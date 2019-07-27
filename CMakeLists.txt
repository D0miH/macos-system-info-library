project(macos-syste-info-library)
cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 14)

# get IOKit
find_library(IOKit IOKit)
if(NOT IOKit)
    message(FATAL_ERROR "IOKit not found")
endif()

find_library(CoreFoundation CoreFoundation)
if(NOT CoreFoundation)
    message(FATAL_ERROR "CoreFoundation not found")
endif()

ADD_LIBRARY(SystemLib System.h System.cpp Utils.h)

add_executable(SystemMain main.cpp)

target_link_libraries(SystemMain SystemLib ${IOKit} ${CoreFoundation})