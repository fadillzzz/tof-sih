namespace Menu {
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
        ID3D12Resource *resource;
        D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle;
    };

    static uint32_t bufferCount = -1;
    static FrameContext *frameContext;

    typedef HRESULT(APIENTRY *D3D12Present)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
    static D3D12Present oPresent = nullptr;

    typedef void(APIENTRY *ExecuteCommandLists)(ID3D12CommandQueue *queue, UINT NumCommandLists,
                                                ID3D12CommandList *ppCommandLists);
    static ExecuteCommandLists oExecuteCommandLists = nullptr;

    void init();
    void shutdown();
} // namespace Menu
