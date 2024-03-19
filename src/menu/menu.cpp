#include "kiero.h"

#include "imgui.h"

#define USE_DX12 1
#define USE_DX11 0

#ifdef USE_DX12
#include "dx12_impl.hpp"
#endif

#ifdef USE_DX11
#include "dx11_impl.hpp"
#endif

#include "menu.hpp"

namespace Menu {
    std::vector<void *> menuToRender;

    void init() {
#if USE_DX12
        if (kiero::init(kiero::RenderType::D3D12) == kiero::Status::Success) {
            DX12::init();
        }
#endif

#if USE_DX11
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            DX11::init();
        }
#endif
    }

    void shutdown() {
#if USE_DX12
        DX12::shutdown();
#endif
#if USE_DX11
        DX11::shutdown();
#endif
    }

    void registerMenu(void *func) {
        menuToRender.push_back(func);
        std::cout << menuToRender.size() << std::endl;
    }

    std::vector<void *> getRegisteredMenu() { return menuToRender; }

} // namespace Menu