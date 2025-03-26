add_executable(Injector main.cpp ../injector/inject.cpp)
add_library(_aux SHARED _aux.cpp ../injector/inject.cpp)
target_precompile_headers(Injector PRIVATE pch.hpp)
target_include_directories(Injector PRIVATE
    ../cfg
)
target_include_directories(_aux PRIVATE
    ../kiero/minhook/include
)
target_link_libraries(Injector Config ntdll.lib)
target_link_libraries(_aux minhook ntdll.lib)
