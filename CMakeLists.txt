cmake_minimum_required(VERSION 3.12)
project(Assignment_3_Client)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(BOOST_ROOT "/path_to_boost_1_57_0")

find_package(Boost 1.57.0)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem)
add_executable(Assignment_3_Client src/connectionHandler.cpp src/BGSclient.cpp src/SocketReader.cpp include/SocketReader.h)
target_link_libraries(Assignment_3_Client boost_system)