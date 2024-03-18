add_library(Kiero
    ../../../kiero/kiero.cpp
    ../../../kiero/minhook/src/hook.c
    ../../../kiero/minhook/src/trampoline.c
    ../../../kiero/minhook/src/buffer.c
    ../../../kiero/minhook/src/hde/hde64.c
)
add_definitions(-DMINHOOK_DISABLE_INTRINSICS)
