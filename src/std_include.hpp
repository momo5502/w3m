#pragma once

#define TLS_PAYLOAD_SIZE 0x2000

#pragma warning(push)
#pragma warning(disable: 4127)
#pragma warning(disable: 4244)
#pragma warning(disable: 4458)
#pragma warning(disable: 4702)
#pragma warning(disable: 4996)
#pragma warning(disable: 5054)
#pragma warning(disable: 6011)
#pragma warning(disable: 6297)
#pragma warning(disable: 6385)
#pragma warning(disable: 6386)
#pragma warning(disable: 6387)
#pragma warning(disable: 26110)
#pragma warning(disable: 26451)
#pragma warning(disable: 26444)
#pragma warning(disable: 26451)
#pragma warning(disable: 26489)
#pragma warning(disable: 26495)
#pragma warning(disable: 26498)
#pragma warning(disable: 26812)
#pragma warning(disable: 28020)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <ShlObj.h>
#include <d3d11.h>
#include <shellscalingapi.h>
#include <winternl.h>

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <map>
#include <atomic>
#include <vector>
#include <mutex>
#include <queue>
#include <regex>
#include <chrono>
#include <thread>
#include <fstream>
#include <utility>
#include <filesystem>
#include <functional>

#include <udis86.h>
#include <MinHook.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <asmjit/core/jitruntime.h>
#include <asmjit/x86/x86assembler.h>

#pragma warning(pop)

#include "resource.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std::literals;

extern __declspec(thread) char tls_data[TLS_PAYLOAD_SIZE];
