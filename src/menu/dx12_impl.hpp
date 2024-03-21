namespace Menu {
    namespace DX12 {
        static bool showMenu = false;
        static bool initialized = false;
        static HWND windowHandle = nullptr;
        static WNDPROC wndProc;
        static ID3D12Device *device = nullptr;
        static ID3D12DescriptorHeap *descriptorHeapBackBuffers = nullptr;
        static ID3D12DescriptorHeap *descriptorHeapImGuiRender = nullptr;
        static ID3D12GraphicsCommandList *commandList = nullptr;
        static ID3D12CommandQueue *commandQueue = nullptr;

        struct FrameContext {
            ID3D12CommandAllocator *commandAllocator;
            uint64_t fenceValue;
        };

        static uint32_t frameIndex = 0;

        static uint32_t bufferCount = -1;

        typedef HRESULT(APIENTRY *D3D12Present)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
        static D3D12Present oPresent = nullptr;

        typedef void(APIENTRY *ExecuteCommandLists)(ID3D12CommandQueue *queue, UINT NumCommandLists,
                                                    ID3D12CommandList *ppCommandLists);
        static ExecuteCommandLists oExecuteCommandLists = nullptr;

        const uint32_t NUM_FRAMES_IN_FLIGHT = 3;
        const uint32_t NUM_BACK_BUFFERS = 3;

        static HANDLE swapChainWaitableObject = nullptr;
        static ID3D12Fence *fence = nullptr;
        static HANDLE fenceEvent = nullptr;

        static ID3D12Resource *mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
        static D3D12_CPU_DESCRIPTOR_HANDLE mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
        static FrameContext frameContext[NUM_FRAMES_IN_FLIGHT] = {};
        static uint64_t fenceLastSignaledValue = 0;

        bool init();
        void shutdown();
    } // namespace DX12
} // namespace Menu