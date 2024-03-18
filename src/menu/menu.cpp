#include "kiero.h"

#include "imgui.h"

#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include "menu.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Menu {
    LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (showMenu) {
            ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
            return true;
        }
        return CallWindowProc(wndProc, hwnd, uMsg, wParam, lParam);
    }

    HRESULT APIENTRY hookPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags) {
        if (!initialized) {
            if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void **)&device))) {
                ImGui::CreateContext();

                ImGuiIO &io = ImGui::GetIO();
                (void)io;
                ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                DXGI_SWAP_CHAIN_DESC Desc;
                pSwapChain->GetDesc(&Desc);
                pSwapChain->GetHwnd(&windowHandle);
                Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                Desc.OutputWindow = windowHandle;
                Desc.Windowed = ((GetWindowLongPtr(windowHandle, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

                bufferCount = Desc.BufferCount;
                frameContext = new FrameContext[bufferCount];

                D3D12_DESCRIPTOR_HEAP_DESC DescriptorImGuiRender = {};
                DescriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                DescriptorImGuiRender.NumDescriptors = bufferCount;
                DescriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                if (device->CreateDescriptorHeap(&DescriptorImGuiRender, IID_PPV_ARGS(&descriptorHeapImGuiRender)) !=
                    S_OK)
                    return oPresent(pSwapChain, SyncInterval, Flags);

                ID3D12CommandAllocator *Allocator;
                if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator)) != S_OK)
                    return oPresent(pSwapChain, SyncInterval, Flags);

                for (size_t i = 0; i < bufferCount; i++) {
                    frameContext[i].commandAllocator = Allocator;
                }

                if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator, NULL,
                                              IID_PPV_ARGS(&commandList)) != S_OK ||
                    commandList->Close() != S_OK) {
                    return oPresent(pSwapChain, SyncInterval, Flags);
                }

                D3D12_DESCRIPTOR_HEAP_DESC DescriptorBackBuffers;
                DescriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                DescriptorBackBuffers.NumDescriptors = bufferCount;
                DescriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                DescriptorBackBuffers.NodeMask = 1;

                if (device->CreateDescriptorHeap(&DescriptorBackBuffers, IID_PPV_ARGS(&descriptorHeapBackBuffers)) !=
                    S_OK)
                    return oPresent(pSwapChain, SyncInterval, Flags);

                const auto RTVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = descriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

                for (size_t i = 0; i < bufferCount; i++) {
                    ID3D12Resource *pBackBuffer = nullptr;
                    frameContext[i].descriptorHandle = RTVHandle;
                    pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
                    device->CreateRenderTargetView(pBackBuffer, nullptr, RTVHandle);
                    frameContext[i].resource = pBackBuffer;
                    RTVHandle.ptr += RTVDescriptorSize;
                }

                ImGui_ImplWin32_Init(windowHandle);
                ImGui_ImplDX12_Init(device, bufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, descriptorHeapImGuiRender,
                                    descriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
                                    descriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
                ImGui_ImplDX12_CreateDeviceObjects();
                wndProc = (WNDPROC)SetWindowLongPtr(windowHandle, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);

                initialized = true;

                std::cout << "Menu initialized" << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_INSERT) & 1) {
            showMenu = !showMenu;
        }

        std::cout << "Show menu: " << showMenu << " "
                  << "Command queue: " << commandQueue << " "
                  << "Initialized: " << initialized << std::endl;

        if (showMenu == false || initialized == false || commandQueue == nullptr) {
            std::cout << "Skipping menu render" << std::endl;
            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        std::cout << "Rendering menu" << std::endl;

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (showMenu) {
            ImGui::Text("Hello, world!");
        }

        ImGui::EndFrame();

        FrameContext &currentFrameContext = frameContext[pSwapChain->GetCurrentBackBufferIndex()];
        currentFrameContext.commandAllocator->Reset();

        D3D12_RESOURCE_BARRIER Barrier;
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        Barrier.Transition.pResource = currentFrameContext.resource;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->Reset(currentFrameContext.commandAllocator, nullptr);
        commandList->ResourceBarrier(1, &Barrier);
        commandList->OMSetRenderTargets(1, &currentFrameContext.descriptorHandle, FALSE, nullptr);
        commandList->SetDescriptorHeaps(1, &descriptorHeapImGuiRender);

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &Barrier);
        commandList->Close();
        commandQueue->ExecuteCommandLists(1, (ID3D12CommandList *const *)(&commandList));

        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    void hookExecuteCommandLists(ID3D12CommandQueue *queue, UINT NumCommandLists, ID3D12CommandList *ppCommandLists) {
        if (!commandQueue) {
            commandQueue = queue;
        }

        oExecuteCommandLists(queue, NumCommandLists, ppCommandLists);
    }

    void init() {
        kiero::init(kiero::RenderType::D3D12);
        kiero::bind(140, (void **)&oPresent, hookPresent);
        kiero::bind(54, (void **)&oExecuteCommandLists, hookExecuteCommandLists);
    }

    void shutdown() {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        kiero::unbind(54);
        kiero::unbind(140);
        kiero::shutdown();
    }

} // namespace Menu