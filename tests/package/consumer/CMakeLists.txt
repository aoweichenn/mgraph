cmake_minimum_required(VERSION 3.24)
project(mgraph_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(mgraph 0.1 CONFIG REQUIRED)

add_executable(mgraph_consumer main.cpp)
target_link_libraries(mgraph_consumer PRIVATE mgraph::mgraph)
