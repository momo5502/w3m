#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <utils/hook.hpp>

namespace console
{
    namespace
    {
        void log_message_stub(void* /*this_*/, const wchar_t* format, ...)
        {
            va_list ap{};
            va_start(ap, format);

            _vfwprintf_p(stdout, (std::wstring(format) + L"\n").data(), ap);

            va_end(ap);
        }

        void create_console()
        {
            if (GetStdHandle(STD_OUTPUT_HANDLE))
            {
                return;
            }

            AllocConsole();
            AttachConsole(GetCurrentProcessId());

            FILE* fp{};
            (void)freopen_s(&fp, "CONIN$", "r", stdin);
            (void)freopen_s(&fp, "CONOUT$", "w", stdout);
            (void)freopen_s(&fp, "CONOUT$", "w", stderr);

            SetConsoleTitleA("Witcher 3: Console");

            SetConsoleCtrlHandler(
                +[](DWORD /*ctrl_type*/) -> BOOL {
                    TerminateProcess(GetCurrentProcess(), 0);
                    return TRUE;
                },
                TRUE);
        }
    }

    struct component final : component_interface
    {
        void post_start() override
        {
            if (!utils::nt::is_wine())
            {
                create_console();
            }
        }

        void post_load() override
        {
            if (!utils::nt::is_wine())
            {
                utils::hook::jump(0x14027E700_g, log_message_stub);
            }

            // Change console font
            utils::hook::inject(0x140119441_g, 0x142373070_g);
            utils::hook::inject(0x140119465_g, 0x142373070_g);

            // Enable ingame console
            utils::hook::set<BYTE>(0x1400F0CBD_g, 1);
        }

        component_priority priority() const override
        {
            return component_priority::console;
        }
    };
}

REGISTER_COMPONENT(console::component)
