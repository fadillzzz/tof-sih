#include "kiero.h"

#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

#include "dx11_impl.hpp"
#include "menu.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Menu {
    namespace DX11 {
        LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
                return true;
            }
            return CallWindowProc(wndProc, hwnd, uMsg, wParam, lParam);
        }

        long __stdcall hookPresent(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) {
            if (!initialized) {
                DXGI_SWAP_CHAIN_DESC desc;
                pSwapChain->GetDesc(&desc);

                ID3D11Device *device;
                pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&device);

                device->GetImmediateContext(&context);

                ID3D11Texture2D *backBuffer;
                pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&backBuffer);
                device->CreateRenderTargetView(backBuffer, NULL, &mainRenderTargetView);
                backBuffer->Release();

                ImGui::CreateContext();
                ImGuiIO &io = ImGui::GetIO();
                (void)io;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
                ImGui_ImplWin32_Init(desc.OutputWindow);
                ImGui_ImplDX11_Init(device, context);

                initialized = true;

                std::cout << "Menu initialized with D3D11 backend" << std::endl;

                wndProc = (WNDPROC)SetWindowLongPtr((HWND)desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
            }

            ImGui::GetIO().MouseDrawCursor = showMenu;

            if (initialized) {
                if (GetAsyncKeyState(VK_INSERT) & 1) {
                    showMenu = !showMenu;
                }
            }

            if (showMenu) {
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                ImGui::Begin("Menu", &showMenu);

                for (auto &func : getRegisteredMenu()) {
                    ((void (*)())func)();
                }

                ImGui::End();

                ImGui::Render();
                context->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }

            return present(pSwapChain, SyncInterval, Flags);
        }

        bool init() {
            kiero::init(kiero::RenderType::D3D11);
            kiero::bind(8, (void **)&present, (void *)hookPresent);

            return true;
        }

        void shutdown() {
            kiero::unbind(8);
            kiero::shutdown();

            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    } // namespace DX11
} // namespace Menu