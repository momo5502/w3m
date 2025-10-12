#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"
#include "../game_version.hpp"

#include <utils/nt.hpp>

#include "window.hpp"
#include "scheduler.hpp"

namespace
{
    namespace splash
    {
        class splash_image
        {
          public:
            splash_image()
                : main_(loader::get_main_module())
            {
                WNDCLASSA wnd_class;

                wnd_class.style = 0;
                wnd_class.cbClsExtra = 0;
                wnd_class.cbWndExtra = 0;
                wnd_class.lpszMenuName = nullptr;
                wnd_class.lpfnWndProc = DefWindowProcA;
                wnd_class.hInstance = this->main_;
                wnd_class.hIcon = LoadIconA(this->main_, reinterpret_cast<LPCSTR>(102));
                wnd_class.hCursor = LoadCursorA(nullptr, IDC_APPSTARTING);
                wnd_class.hbrBackground = reinterpret_cast<HBRUSH>(6);
                wnd_class.lpszClassName = "Witcher Splash Screen";

                RegisterClassA(&wnd_class);
            }

            ~splash_image()
            {
                this->destroy();
                UnregisterClassA("Witcher Splash Screen", this->main_);
            }

            void make_foreground_window() const
            {
                if (this->window_ && IsWindow(this->window_))
                {
                    SetForegroundWindow(this->window_);
                }
            }

            void close()
            {
                const auto window = this->destroy();

                MSG msg{};
                while (window && IsWindow(window))
                {
                    if (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessageW(&msg);
                    }
                    else
                    {
                        std::this_thread::sleep_for(1ms);
                    }
                }
            }

            void show()
            {
                constexpr int initial_width = 320;
                constexpr int initial_height = 100;

                const auto x_pixels = GetSystemMetrics(SM_CXFULLSCREEN);
                const auto y_pixels = GetSystemMetrics(SM_CYFULLSCREEN);

                auto image = static_cast<HWND>(LoadImageA(this->main_, MAKEINTRESOURCE(IMAGE_SPLASH), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
                if (!image)
                {
                    return;
                }

                this->window_ = CreateWindowExA(WS_EX_APPWINDOW, "Witcher Splash Screen", W3M_MODNAME, WS_POPUP | WS_SYSMENU,
                                                (x_pixels - initial_width) / 2, (y_pixels - initial_height) / 2, initial_width,
                                                initial_height, nullptr, nullptr, this->main_, nullptr);

                if (!this->window_)
                {
                    return;
                }

                const auto image_window = CreateWindowExA(0, "Static", nullptr, WS_CHILD | WS_VISIBLE | 0xEu, 0, 0, initial_width,
                                                          initial_height, this->window_, nullptr, this->main_, nullptr);
                if (!image_window)
                {
                    return;
                }

                RECT rect;
                SendMessageA(image_window, 0x172u, 0, reinterpret_cast<LPARAM>(image));
                GetWindowRect(image_window, &rect);

                const int width = rect.right - rect.left;
                const int height = rect.bottom - rect.top;

                rect.left = (x_pixels - width) / 2;
                rect.top = (y_pixels - height) / 2;

                rect.right = rect.left + width;
                rect.bottom = rect.top + height;
                AdjustWindowRect(&rect, WS_CHILD | WS_VISIBLE | 0xEu, 0);
                SetWindowPos(this->window_, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);

                ShowWindow(this->window_, SW_SHOW);
                UpdateWindow(this->window_);

                SetForegroundWindow(this->window_);
            }

          private:
            utils::nt::library main_{};
            HWND window_{};

            HWND destroy()
            {
                const auto window = this->window_;
                this->window_ = nullptr;

                if (!window || !IsWindow(window))
                {
                    return nullptr;
                }

                ShowWindow(window, SW_HIDE);
                DestroyWindow(window);

                if (GetWindowThreadProcessId(window, nullptr) != GetCurrentThreadId())
                {
                    return nullptr;
                }

                return window;
            }
        };

        struct component final : component_interface
        {
            splash_image image_{};

            void post_start() override
            {
                image_.show();

                scheduler::loop([this]() {
                    if (window::get_game_window())
                    {
                        image_.close();
                        return scheduler::cond_end;
                    }

                    return scheduler::cond_continue;
                });
            }

            void post_load() override
            {
                image_.make_foreground_window();
            }

            void pre_destroy() override
            {
                image_.close();
            }
        };
    }
}

REGISTER_COMPONENT(splash::component)
