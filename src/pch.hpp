#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <TlHelp32.h>

#include <chrono>
#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "../CppSDK/SDK.hpp"
#define IMGUI_DISABLE_OBSOLETE_KEYIO
#include "config.hpp"
#include "imgui.h"
#include "logger/logger.hpp"
#include "misc/cpp/imgui_stdlib.h"
