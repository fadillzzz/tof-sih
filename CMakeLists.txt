cmake_minimum_required(VERSION 3.20)

project(TOFInternal VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()
add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(src)
add_subdirectory(CppSDK)
add_subdirectory(injector)
