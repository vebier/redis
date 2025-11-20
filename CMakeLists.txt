cmake_minimum_required(VERSION 3.31)

project(redis)

set(CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp hashtable.cpp)

add_executable(client client.cpp)

add_executable(test test.cpp hashtable.cpp)
