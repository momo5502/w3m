#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"
#include "../game_version.hpp"

#include "window.hpp"
#include "scheduler.hpp"

namespace window
{
    namespace
    {
        BOOL __stdcall enum_windows_proc(const HWND window, const LPARAM param)
        {
            DWORD process = 0;
            GetWindowThreadProcessId(window, &process);

            if (process == GetCurrentProcessId())
            {
                char class_name[500] = {0};
                GetClassNameA(window, class_name, sizeof(class_name));

                if (class_name == "W2ViewportClass"s)
                {
                    *reinterpret_cast<HWND*>(param) = window;
                    return FALSE;
                }
            }

            return TRUE;
        }

        struct component final : component_interface
        {
            void post_load() override
            {
                scheduler::loop([]() {
                    if (const auto game_window = get_game_window())
                    {
                        SetWindowTextA(game_window, W3M_MODNAME);
                        return scheduler::cond_end;
                    }

                    return scheduler::cond_continue;
                });
            }
        };
    }

    HWND get_game_window()
    {
        static HWND window = nullptr;
        if (!window || !IsWindow(window))
        {
            EnumWindows(enum_windows_proc, reinterpret_cast<LPARAM>(&window));
        }

        return window;
    }
}

REGISTER_COMPONENT(window::component)
