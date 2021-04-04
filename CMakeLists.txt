cmake_minimum_required(VERSION 3.7)
project(dxfconvert)

set(CMAKE_CXX_STANDARD 14)
FILE(GLOB_RECURSE LIB_SOURCE lib/*.cpp)
FILE(GLOB_RECURSE LIB_HEADER lib/*.h)

set(
        SOURCE_FILES
        src/main.cpp
        src/DxfConverter.cpp
        src/DxfConverter.h)

set (CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")
add_executable(dxfconvert ${LIB_HEADER} ${LIB_SOURCE} ${SOURCE_FILES})
include_directories(dxfconvert "${CMAKE_CURRENT_SOURCE_DIR}/lib")
include_directories(dxfconvert "${CMAKE_CURRENT_SOURCE_DIR}/src")