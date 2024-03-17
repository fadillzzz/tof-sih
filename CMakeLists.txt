cmake_minimum_required(VERSION 3.20)

project(TOFInternal VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(src)
add_subdirectory(CppSDK)
