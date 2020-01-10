#include <std_include.hpp>
#include "window.hpp"

void window::post_load()
{
	this->kill_ = false;
	this->thread_ = std::thread([this]()
	{
		while (!this->kill_ && !get_game_window())
		{
			std::this_thread::sleep_for(1ms);
		}

		if (const auto game_window = get_game_window())
		{
			SetWindowTextA(game_window, "Witcher 3: Online");
		}
	});
}

void window::pre_destroy()
{
	this->kill_ = true;
	if (this->thread_.joinable())
	{
		this->thread_.join();
	}
}

HWND window::get_game_window()
{
	HWND window = nullptr;
	EnumWindows(enum_windows_proc, LPARAM(&window));
	return window;
}

BOOL __stdcall window::enum_windows_proc(const HWND window, const LPARAM param)
{
	DWORD process = 0;
	GetWindowThreadProcessId(window, &process);

	if (process == GetCurrentProcessId())
	{
		char class_name[500] = { 0 };
		GetClassNameA(window, class_name, sizeof(class_name));

		if (class_name == "W2ViewportClass"s)
		{
			*reinterpret_cast<HWND*>(param) = window;
		}
	}

	return TRUE;
}

REGISTER_MODULE(window);
