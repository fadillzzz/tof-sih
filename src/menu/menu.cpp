#include "kiero.h"

#include "dx11_impl.hpp"
#include "dx12_impl.hpp"

#include "menu.hpp"

namespace Menu {
    std::vector<void *> menuToRender;
    bool isUsingDx12 = false;

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

    void registerMenu(void *func) { menuToRender.push_back(func); }

    std::vector<void *> getRegisteredMenu() { return menuToRender; }

} // namespace Menu