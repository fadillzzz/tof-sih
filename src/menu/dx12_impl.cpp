#include "kiero.h"

#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"

#include "dx12_impl.hpp"
#include "menu.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Menu {
    namespace DX12 {
        LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) {
                return true;
            }
            return CallWindowProc(wndProc, hwnd, uMsg, wParam, lParam);
        }

        FrameContext *WaitForNextFrameResources() {
            uint32_t nextFrameIndex = (frameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
            frameIndex = nextFrameIndex;

            HANDLE waitableObjects[] = {swapChainWaitableObject, nullptr};
            uint32_t numWaitableObjects = 1;

            FrameContext *frameCtx = &frameContext[nextFrameIndex];
            uint64_t fenceValue = frameCtx->fenceValue;

            if (fenceValue != 0) {
                frameCtx->fenceValue = 0;
                fence->SetEventOnCompletion(fenceValue, fenceEvent);
                waitableObjects[1] = fenceEvent;
                numWaitableObjects = 2;
            }

            WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

            return frameCtx;
        }

        void CreateRenderTarget(IDXGISwapChain3 *pSwapChain, ID3D12Device *device) {
            for (uint32_t i = 0; i < NUM_BACK_BUFFERS; i++) {
                ID3D12Resource *backBuffer = nullptr;
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
                device->CreateRenderTargetView(backBuffer, nullptr, mainRenderTargetDescriptor[i]);
                mainRenderTargetResource[i] = backBuffer;
            }
        }

        HRESULT APIENTRY hookPresent(IDXGISwapChain3 *pSwapChain, UINT SyncInterval, UINT Flags) {
            if (!initialized) {
                if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void **)&device))) {
                    ImGui::CreateContext();
                    ImGuiIO &io = ImGui::GetIO();
                    (void)io;
                    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                    swapChainWaitableObject = pSwapChain->GetFrameLatencyWaitableObject();
                    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

                    {
                        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                        desc.NumDescriptors = NUM_BACK_BUFFERS;
                        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                        desc.NodeMask = 1;
                        device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeapBackBuffers));

                        uintptr_t rtvDescriptorSize =
                            device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
                        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
                            descriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();
                        for (uint32_t i = 0; i < NUM_BACK_BUFFERS; i++) {
                            mainRenderTargetDescriptor[i] = rtvHandle;
                            rtvHandle.ptr += rtvDescriptorSize;
                        }
                    }

                    CreateRenderTarget(pSwapChain, device);
                    pSwapChain->GetHwnd(&windowHandle);

                    {
                        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                        desc.NumDescriptors = 1;
                        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                        device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeapImGuiRender));

                        for (uint32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
                            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                           IID_PPV_ARGS(&frameContext[i].commandAllocator));
                        }
                    }

                    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameContext[0].commandAllocator,
                                              nullptr, IID_PPV_ARGS(&commandList));

                    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

                    ImGui_ImplWin32_Init(windowHandle);
                    ImGui_ImplDX12_Init(device, NUM_FRAMES_IN_FLIGHT, DXGI_FORMAT_R8G8B8A8_UNORM,
                                        descriptorHeapImGuiRender,
                                        descriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(),
                                        descriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());

                    initialized = true;

                    std::cout << "Menu initialized with D3D12 backend" << std::endl;

                    wndProc = (WNDPROC)SetWindowLongPtr(windowHandle, GWLP_WNDPROC, (LONG_PTR)WndProc);
                }
            }

            ImGui::GetIO().MouseDrawCursor = showMenu;

            if (initialized) {
                if (GetAsyncKeyState(VK_INSERT) & 1) {
                    showMenu = !showMenu;
                }
            }

            if (showMenu && commandQueue != nullptr) {
                ImGui_ImplDX12_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                ImGui::Begin("Menu", &showMenu);

                for (auto &func : getRegisteredMenu()) {
                    ((void (*)())func)();
                }

                ImGui::End();

                ImGui::Render();

                FrameContext *frameCtx = WaitForNextFrameResources();
                uint32_t backBufferIdx = pSwapChain->GetCurrentBackBufferIndex();
                frameCtx->commandAllocator->Reset();

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource = mainRenderTargetResource[backBufferIdx];
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
                commandList->Reset(frameCtx->commandAllocator, nullptr);
                commandList->ResourceBarrier(1, &barrier);

                commandList->OMSetRenderTargets(1, &mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
                commandList->SetDescriptorHeaps(1, &descriptorHeapImGuiRender);
                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                commandList->ResourceBarrier(1, &barrier);
                commandList->Close();

                commandQueue->ExecuteCommandLists(1, (ID3D12CommandList *const *)&commandList);

                const auto retVal = oPresent(pSwapChain, SyncInterval, Flags);

                UINT64 fenceValue = fenceLastSignaledValue + 1;
                commandQueue->Signal(fence, fenceValue);
                fenceLastSignaledValue = fenceValue;
                frameCtx->fenceValue = fenceValue;

                return retVal;
            }

            return oPresent(pSwapChain, SyncInterval, Flags);
        }

        void hookExecuteCommandLists(ID3D12CommandQueue *queue, UINT NumCommandLists,
                                     ID3D12CommandList *ppCommandLists) {
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
            kiero::unbind(140);
            kiero::unbind(54);
            kiero::shutdown();

            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    } // namespace DX12
} // namespace Menu