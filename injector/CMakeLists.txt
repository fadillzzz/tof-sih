cmake_minimum_required(VERSION 3.20)

project(TOFInjector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_definitions(-DUNICODE -D_UNICODE)

add_executable(Injector main.cpp)
add_library(_aux SHARED _aux.cpp)
add_subdirectory(../cfg ${CMAKE_BINARY_DIR}/cfg)
add_subdirectory(../kiero/minhook ${CMAKE_BINARY_DIR}/kiero/minhook)
target_precompile_headers(Injector PRIVATE pch.hpp)
target_include_directories(Injector PRIVATE
    ../cfg
)
target_include_directories(_aux PRIVATE
    ../kiero/minhook/include
)
target_link_libraries(Injector Config ntdll.lib)
target_link_libraries(_aux minhook ntdll.lib)