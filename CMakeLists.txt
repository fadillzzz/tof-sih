cmake_minimum_required(VERSION 3.20)

project(TOFInternal VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()
add_definitions(-DUNICODE -D_UNICODE)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_subdirectory(src)
add_subdirectory(CppSDK)
