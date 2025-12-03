cmake_minimum_required(VERSION 3.31)
project(redis)
set(CMAKE_CXX_STANDARD 20)

# 设置包含目录
set(INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/network
        ${CMAKE_SOURCE_DIR}/include/protocol
        ${CMAKE_SOURCE_DIR}/include/server
        ${CMAKE_SOURCE_DIR}/include/storage
)

file(GLOB_RECURSE SRC "src/*.cpp")

set(ALL_SOURCES
        ${SRC}
        include/core/singleton.h
)

# 服务端
add_executable(main main.cpp ${ALL_SOURCES})
target_include_directories(main PUBLIC ${INCLUDE_DIRS})

# 客户端
add_executable(client client.cpp ${NETWORK_SOURCES})
target_include_directories(client PUBLIC ${INCLUDE_DIRS})

