#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "utils/nt.hpp"
#include "window.hpp"

class splash final : public module
{
public:
	void post_start() override
	{
		this->show();

		this->kill_ = false;
		this->thread_ = std::thread([this]()
		{
			while (!this->kill_ && !window::get_game_window())
			{
				std::this_thread::sleep_for(10ms);
			}

			this->destroy();
			this->thread_.detach();
		});
	}

	void pre_destroy() override
	{
		this->kill_ = true;

		MSG msg;
		while (this->thread_.joinable())
		{
			if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				std::this_thread::sleep_for(1ms);
			}
		}
	}

private:
	HWND window_{};
	bool kill_ = false;
	std::thread thread_;

	void destroy()
	{
		const utils::nt::module main;

		if (IsWindow(this->window_))
		{
			ShowWindow(this->window_, SW_HIDE);
			DestroyWindow(this->window_);
			UnregisterClassA("Witcher Splash Screen", main);
		}

		this->window_ = nullptr;
	}

	void show()
	{
		WNDCLASSA wnd_class;

		const utils::nt::module main;

		wnd_class.style = 0;
		wnd_class.cbClsExtra = 0;
		wnd_class.cbWndExtra = 0;
		wnd_class.lpszMenuName = nullptr;
		wnd_class.lpfnWndProc = DefWindowProcA;
		wnd_class.hInstance = main;
		wnd_class.hIcon = LoadIconA(main, LPCSTR(102));
		wnd_class.hCursor = LoadCursorA(nullptr, IDC_APPSTARTING);
		wnd_class.hbrBackground = HBRUSH(6);
		wnd_class.lpszClassName = "Witcher Splash Screen";

		if (RegisterClassA(&wnd_class))
		{
			const auto x_pixels = GetSystemMetrics(SM_CXFULLSCREEN);
			const auto y_pixels = GetSystemMetrics(SM_CYFULLSCREEN);

			auto image = HWND(LoadImageA(main, MAKEINTRESOURCE(IMAGE_SPLASH), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
			if (image)
			{
				this->window_ = CreateWindowExA(WS_EX_APPWINDOW, "Witcher Splash Screen", "Witcher 3: Online", WS_POPUP | WS_SYSMENU, (x_pixels - 320) / 2, (y_pixels - 100) / 2, 320, 100, nullptr, nullptr, main, nullptr);

				if (this->window_)
				{
					const auto image_window = CreateWindowExA(0, "Static", nullptr, WS_CHILD | WS_VISIBLE | 0xEu, 0, 0, 320, 100, this->window_, nullptr, main, nullptr);
					if (image_window)
					{
						RECT rect;
						SendMessageA(image_window, 0x172u, 0, LPARAM(image));
						GetWindowRect(image_window, &rect);

						const int width = rect.right - rect.left;
						rect.left = (x_pixels - width) / 2;

						const int height = rect.bottom - rect.top;
						rect.top = (y_pixels - height) / 2;

						rect.right = rect.left + width;
						rect.bottom = rect.top + height;
						AdjustWindowRect(&rect, WS_CHILD | WS_VISIBLE | 0xEu, 0);
						SetWindowPos(this->window_, nullptr, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);

						ShowWindow(this->window_, SW_SHOW);
						UpdateWindow(this->window_);
					}
				}
			}
		}
	}
};

REGISTER_MODULE(splash)
