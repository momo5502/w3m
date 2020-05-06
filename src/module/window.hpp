#pragma once
#include "loader/module_loader.hpp"

class window final : public module
{
public:
	void post_load() override;

	static HWND get_game_window();

private:
	std::thread thread_;
	bool kill_ = false;

	static BOOL __stdcall enum_windows_proc(HWND window, LPARAM param);
};
