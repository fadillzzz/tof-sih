add_library(ImGui
    ../../../imgui/imgui.cpp
    ../../../imgui/imgui_draw.cpp
    ../../../imgui/imgui_widgets.cpp
    ../../../imgui/imgui_tables.cpp
    ../../../imgui/backends/imgui_impl_win32.cpp
    ../../../imgui/backends/imgui_impl_dx12.cpp
    ../../../imgui/backends/imgui_impl_dx11.cpp
    ../../../imgui/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(ImGui PRIVATE ../../../imgui)