#pragma once
#include "loader/module_loader.hpp"

class window final : public module
{
public:
	void post_load() override;
	void pre_destroy() override;

	static HWND get_game_window();

private:
	std::thread thread_;
	bool kill_ = false;

	static BOOL __stdcall enum_windows_proc(const HWND window, const LPARAM param);
};
