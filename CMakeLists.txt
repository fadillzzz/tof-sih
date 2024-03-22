cmake_minimum_required(VERSION 3.20)

project(TOFInternal VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()
add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(src)
add_subdirectory(CppSDK)
