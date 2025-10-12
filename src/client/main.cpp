#include "std_include.hpp"

#include "loader/loader.hpp"
#include "loader/component_loader.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/finally.hpp>

extern __declspec(thread) char tls_data[TLS_PAYLOAD_SIZE];

namespace
{
    DECLSPEC_NORETURN void WINAPI exit_hook(const int code)
    {
        component_loader::pre_destroy();
        exit(code);
    }

    void verify_tls()
    {
        const auto self = loader::get_main_module();
        const auto self_tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(
            self.get_ptr() + self.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

        const auto ref = reinterpret_cast<DWORD64>(&tls_data);
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

        const auto* callback = get_tls_callbacks();
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

    bool handle_process_runner()
    {
        const auto* const command = "-proc ";
        const char* parent_proc = strstr(GetCommandLineA(), command);

        if (!parent_proc)
        {
            return false;
        }

        const auto pid = static_cast<DWORD>(atoi(parent_proc + strlen(command)));
        const utils::nt::handle<> process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
        if (process_handle)
        {
            WaitForSingleObject(process_handle, INFINITE);
        }

        return true;
    }

    void enable_dpi_awareness()
    {
        const utils::nt::library user32{"user32.dll"};

        {
            const auto set_dpi = user32 ? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>("SetProcessDpiAwarenessContext") : nullptr;
            if (set_dpi)
            {
                set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
                return;
            }
        }

        {
            const utils::nt::library shcore{"shcore.dll"};
            const auto set_dpi = shcore ? shcore.get_proc<HRESULT(WINAPI*)(PROCESS_DPI_AWARENESS)>("SetProcessDpiAwareness") : nullptr;
            if (set_dpi)
            {
                set_dpi(PROCESS_PER_MONITOR_DPI_AWARE);
                return;
            }
        }

        {
            const auto set_dpi = user32 ? user32.get_proc<BOOL(WINAPI*)()>("SetProcessDPIAware") : nullptr;
            if (set_dpi)
            {
                set_dpi();
            }
        }
    }

    void* import_resolver(const std::string& module, const std::string& function)
    {
        if (function == "ExitProcess")
        {
            return &exit_hook;
        }

        return component_loader::load_import(module, function);
    }
}

int main(int argc, char** argv)
{
    if (handle_process_runner())
    {
        return 0;
    }

    utils::flags flags{argc, argv};

    FARPROC entry_point{};
    srand(static_cast<uint32_t>(time(nullptr)) ^ ~(GetTickCount() * GetCurrentProcessId()));

    enable_dpi_awareness();

    {
        auto premature_shutdown = true;
        const auto _ = utils::finally([&premature_shutdown]() {
            if (premature_shutdown)
            {
                component_loader::pre_destroy();
            }
        });

        try
        {
            verify_tls();
            if (!component_loader::post_start())
                return 0;

            assert(utils::nt::library{} == loader::get_main_module());

            const auto lib = loader::load("witcher3.exe", import_resolver);
            loader::apply_main_module(lib);

            assert(utils::nt::library{} == loader::get_game_module());

            g_call_tls_callbacks = true;
            run_tls_callbacks(DLL_PROCESS_ATTACH);

            entry_point = static_cast<FARPROC>(lib.get_entry_point());

            if (!component_loader::post_load())
                return 0;
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

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    return main(__argc, __argv);
}
