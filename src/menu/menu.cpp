#include "kiero.h"

#include "dx11_impl.hpp"
#include "dx12_impl.hpp"

#include "../logger/logger.hpp"
#include "layout/layout.hpp"
#include "menu.hpp"
#include "roboto.hpp"

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

    void postInit() {
        auto io = ImGui::GetIO();
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;
        auto roboto = io.Fonts->AddFontFromMemoryTTF((void *)RobotoRegular, RobotoRegularLen, 48.0f, &fontConfig);
        roboto->Scale /= 3;
        io.FontDefault = roboto;
    }

    void shutdown() {
        if (isUsingDx12) {
            DX12::shutdown();
        } else {
            DX11::shutdown();
        }
    }

    void render() {
        if (initialized) {
            if (GetAsyncKeyState(VK_INSERT) & 1) {
                showMenu = !showMenu;
                ImGui::GetIO().MouseDrawCursor = showMenu;
            }
        }

        if (showMenu) {
            ImGui::Begin("TOF-SIH", &showMenu);

            Menu::Layout::render();

            ImGui::End();
        }
    }

} // namespace Menu