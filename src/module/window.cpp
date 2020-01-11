#include <std_include.hpp>
#include "window.hpp"
#include "scheduler.hpp"

void window::post_load()
{
	scheduler::frame([]()
	{
		if (const auto game_window = get_game_window())
		{
			SetWindowTextA(game_window, "Witcher 3: Online");
			return scheduler::cond_end;
		}

		return scheduler::cond_continue;
	});
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
