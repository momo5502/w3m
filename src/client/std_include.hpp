#pragma once

#define TLS_PAYLOAD_SIZE 0x2000

#pragma warning(push)
#pragma warning(disable : 4127)
#pragma warning(disable : 4244)
#pragma warning(disable : 4458)
#pragma warning(disable : 4702)
#pragma warning(disable : 4996)
#pragma warning(disable : 5054)
#pragma warning(disable : 6011)
#pragma warning(disable : 6297)
#pragma warning(disable : 6385)
#pragma warning(disable : 6386)
#pragma warning(disable : 6387)
#pragma warning(disable : 26110)
#pragma warning(disable : 26451)
#pragma warning(disable : 26444)
#pragma warning(disable : 26451)
#pragma warning(disable : 26489)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)
#pragma warning(disable : 26812)
#pragma warning(disable : 28020)

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
#include <set>
#include <span>
#include <array>
#include <mutex>
#include <queue>
#include <atomic>
#include <vector>
#include <string>
#include <chrono>
#include <optional>
#include <stdexcept>
#include <functional>
#include <filesystem>
#include <string_view>

#include <cassert>

#pragma warning(pop)

#include "resource.hpp"

using namespace std::literals;
