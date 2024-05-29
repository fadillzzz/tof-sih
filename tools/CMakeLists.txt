add_executable(aad aad.cpp)
add_library(log SHARED log.cpp)
target_link_libraries(log minhook ntdll)
target_include_directories(log PRIVATE ../kiero/minhook/include)