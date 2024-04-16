add_library(TOFInternal SHARED
    main.cpp
    globals.cpp
    hooks.cpp
    logger/logger.cpp
    logger/chain/chain.cpp
    menu/menu.cpp
    menu/layout/layout.cpp
    menu/dx12_impl.cpp
    menu/dx11_impl.cpp
    feats/anti_anti_cheat.cpp
    feats/move_speed.cpp
    feats/fov.cpp
    feats/inf_jump.cpp
    feats/login.cpp
    feats/teleport_nucleus.cpp
    feats/quest.cpp
    feats/teleport_anywhere.cpp
    feats/chain_logging.cpp
    feats/no_clip.cpp
    feats/uid_edit.cpp
    feats/about.cpp
    feats/hotkey.cpp
    feats/ping.cpp
)
target_precompile_headers(TOFInternal PRIVATE pch.hpp)
add_subdirectory(ext/imgui)
add_subdirectory(ext/kiero)
target_link_libraries(TOFInternal CppSDK ImGui Kiero Config)
target_include_directories(TOFInternal PRIVATE ../imgui ../kiero ../cfg)