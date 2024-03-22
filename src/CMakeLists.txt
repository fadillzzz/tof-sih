add_library(TOFInternal SHARED
    main.cpp
    globals.cpp
    menu/menu.cpp
    menu/dx12_impl.cpp
    menu/dx11_impl.cpp
    feats/move_speed.cpp
    feats/fov.cpp
    feats/inf_jump.cpp
    feats/login.cpp
    feats/teleport_nucleus.cpp
    feats/quest.cpp
)
target_precompile_headers(TOFInternal PRIVATE pch.hpp)
add_subdirectory(ext/imgui)
add_subdirectory(ext/kiero)
target_link_libraries(TOFInternal CppSDK)
target_link_libraries(TOFInternal ImGui)
target_link_libraries(TOFInternal Kiero)
target_include_directories(TOFInternal PRIVATE ../imgui)
target_include_directories(TOFInternal PRIVATE ../kiero)
