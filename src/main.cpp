#include <std_include.hpp>
#include "loader/loader.hpp"
#include "utils/string.hpp"
#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

DECLSPEC_NORETURN void WINAPI exit_hook(const int code)
{
	component_loader::pre_destroy();
	exit(code);
}

void verify_tls()
{
	const auto self = loader::get_main_module();
	const auto self_tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(self.get_ptr()
		+ self.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

	const auto ref = DWORD64(&tls_data);
	const auto tls_index = *reinterpret_cast<PDWORD>(self_tls->AddressOfIndex);
	const auto tls_vector = *reinterpret_cast<PDWORD64>(__readgsqword(0x58) + 8ull * tls_index);
	const auto offset = ref - tls_vector;

	if (offset != 0 && offset != 16) // Actually 16 is bad, but I think msvc places custom stuff before
	{
		throw std::runtime_error(utils::string::va("TLS payload is at offset 0x%X, but should be at 0!", offset));
	}
}

volatile bool g_call_tls_callbacks = false;

PIMAGE_TLS_CALLBACK* get_tls_callbacks()
{
	const utils::nt::library game{};
	const auto& entry = game.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
	if (!entry.VirtualAddress || !entry.Size)
	{
		return nullptr;
	}

	const auto* tls_dir = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(game.get_ptr() + entry.VirtualAddress);
	return reinterpret_cast<PIMAGE_TLS_CALLBACK*>(tls_dir->AddressOfCallBacks);
}

void run_tls_callbacks(const DWORD reason)
{
	if (!g_call_tls_callbacks)
	{
		return;
	}

	auto* callback = get_tls_callbacks();
	while (callback && *callback)
	{
		(*callback)(GetModuleHandleA(nullptr), reason, nullptr);
		++callback;
	}
}

[[maybe_unused]] thread_local struct tls_runner
{
	tls_runner()
	{
		run_tls_callbacks(DLL_THREAD_ATTACH);
	}

	~tls_runner()
	{
		run_tls_callbacks(DLL_THREAD_DETACH);
	}
} tls_runner;

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	FARPROC entry_point;
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	{
		auto premature_shutdown = true;
		const auto _ = gsl::finally([&premature_shutdown]()
			{
				if (premature_shutdown)
				{
					component_loader::pre_destroy();
				}
			});

		try
		{
			verify_tls();
			if (!component_loader::post_start()) return 0;

			const auto module = loader::load("witcher3.exe", [](const std::string& module, const std::string& function) -> void*
				{
					if (function == "ExitProcess")
					{
						return &exit_hook;
					}

					return component_loader::load_import(module, function);
				});

			const auto version_sig = "48 FF 42 30 48 8D 05 ? ? ? ?"_sig;
			if (version_sig.count() != 1 || utils::hook::extract<char*>(version_sig.get(0) + 0x7) != "v 1.32"s)
			{
				//auto x = utils::hook::extract<char*>(version_sig.get(0) + 0x7);
				//printf("X");
				//throw std::runtime_error("Unsupported game version");
			}

			g_call_tls_callbacks = true;
			run_tls_callbacks(DLL_PROCESS_ATTACH);

			entry_point = FARPROC(module.get_entry_point());

			if (!component_loader::post_load()) return 0;
			premature_shutdown = false;
		}
		catch (std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
			return 1;
		}
	}

	return static_cast<int>(entry_point());
}
