add_library(TOFInternal SHARED
    main.cpp
    menu/menu.cpp
)
target_precompile_headers(TOFInternal PUBLIC pch.hpp)
add_subdirectory(ext/imgui)
add_subdirectory(ext/kiero)
target_link_libraries(TOFInternal CppSDK)
target_link_libraries(TOFInternal ImGui)
target_link_libraries(TOFInternal Kiero)
target_include_directories(TOFInternal PUBLIC ../imgui)
target_include_directories(TOFInternal PUBLIC ../kiero)