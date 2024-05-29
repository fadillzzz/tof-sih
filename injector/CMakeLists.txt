add_executable(Injector main.cpp)
target_precompile_headers(Injector PRIVATE pch.hpp)
target_include_directories(Injector PRIVATE
    ../cfg
)
target_link_libraries(Injector Config ntdll.lib)
