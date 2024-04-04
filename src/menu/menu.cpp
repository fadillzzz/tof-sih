#include "kiero.h"

#include "dx11_impl.hpp"
#include "dx12_impl.hpp"

#include "../logger/logger.hpp"
#include "layout/layout.hpp"
#include "menu.hpp"

namespace Menu {
    bool isUsingDx12 = false;
    bool showMenu = false;
    bool initialized = false;

    void init() {
        if (DX12::init()) {
            isUsingDx12 = true;
        } else {
            DX11::init();
        }
    }

    void shutdown() {
        if (isUsingDx12) {
            DX12::shutdown();
        } else {
            DX11::shutdown();
        }
    }

    void render() {
        ImGui::GetIO().MouseDrawCursor = showMenu;

        if (initialized) {
            if (GetAsyncKeyState(VK_INSERT) & 1) {
                showMenu = !showMenu;
            }
        }

        if (showMenu) {
            ImGui::Begin("TOF-SIH", &showMenu);

            Menu::Layout::render();

            ImGui::End();
        }
    }

} // namespace Menu