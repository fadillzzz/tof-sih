namespace Menu {
    namespace DX11 {
        static bool showMenu = false;
        static bool initialized = false;

        typedef long(__stdcall *D3D11Present)(IDXGISwapChain *, uint32_t, uint32_t);
        static D3D11Present present = nullptr;
        static WNDPROC wndProc = nullptr;

        void init();
        void shutdown();
    } // namespace DX11
} // namespace Menu
