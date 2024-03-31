add_library(Kiero
    ../../../kiero/kiero.cpp
    ../../../kiero/minhook/src/hook.c
    ../../../kiero/minhook/src/trampoline.c
    ../../../kiero/minhook/src/buffer.c
    ../../../kiero/minhook/src/hde/hde64.c
)
target_compile_definitions(Kiero PRIVATE -DMINHOOK_DISABLE_INTRINSICS -DKIERO_USE_MINHOOK=1 -DKIERO_INCLUDE_D3D11=1 -DKIERO_INCLUDE_D3D12=1)
