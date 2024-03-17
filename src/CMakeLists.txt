add_library(TOFInternal SHARED
    main.cpp
)
target_precompile_headers(TOFInternal PUBLIC pch.hpp)
add_subdirectory(ext/imgui)
target_link_libraries(TOFInternal CppSDK)
target_link_libraries(TOFInternal ImGui)
target_include_directories(TOFInternal PUBLIC ../imgui)