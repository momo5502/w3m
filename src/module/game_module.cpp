#include <std_include.hpp>
#include "loader/loader.hpp"
#include "loader/module_loader.hpp"
#include "utils/hook.hpp"

class game_module final : public module
{
public:
	void post_start() override
	{
		handle_a_hook_ = utils::hook::detour(&GetModuleHandleA, &get_module_handle_a);
		handle_w_hook_ = utils::hook::detour(&GetModuleHandleW, &get_module_handle_w);
		handle_ex_w_hook_ = utils::hook::detour(&GetModuleHandleExA, &get_module_handle_ex_a);
		handle_ex_w_hook_ = utils::hook::detour(&GetModuleHandleExW, &get_module_handle_ex_w);
		file_name_a_hook_ = utils::hook::detour(&GetModuleFileNameA, &get_module_file_name_a);
		file_name_w_hook_ = utils::hook::detour(&GetModuleFileNameW, &get_module_file_name_w);
	}

private:
	static utils::hook::detour handle_a_hook_;
	static utils::hook::detour handle_w_hook_;
	static utils::hook::detour handle_ex_a_hook_;
	static utils::hook::detour handle_ex_w_hook_;
	static utils::hook::detour file_name_a_hook_;
	static utils::hook::detour file_name_w_hook_;
	
	static HMODULE __stdcall get_module_handle_a(const LPCSTR module_name)
	{
		if (!module_name)
		{
			return loader::get_game_module();
		}

		return handle_a_hook_.get<decltype(GetModuleHandleA)>()(module_name);
	}

	static HMODULE __stdcall get_module_handle_w(const LPWSTR module_name)
	{
		if (!module_name)
		{
			return loader::get_game_module();
		}

		return handle_w_hook_.get<decltype(GetModuleHandleW)>()(module_name);
	}

	static BOOL __stdcall get_module_handle_ex_a(const DWORD flags, const LPCSTR module_name, HMODULE* module)
	{
		if (!module_name)
		{
			*module = loader::get_game_module();
			return TRUE;
		}

		return handle_ex_a_hook_.get<decltype(GetModuleHandleExA)>()(flags, module_name, module);
	}

	static BOOL __stdcall get_module_handle_ex_w(const DWORD flags, const LPCWSTR module_name, HMODULE* module)
	{
		if (!module_name)
		{
			*module = loader::get_game_module();
			return TRUE;
		}

		return handle_ex_w_hook_.get<decltype(GetModuleHandleExW)>()(flags, module_name, module);
	}

	static DWORD __stdcall get_module_file_name_a(HMODULE module, const LPSTR filename, const DWORD size)
	{
		if (!module)
		{
			module = loader::get_game_module();
		}

		return file_name_a_hook_.get<decltype(GetModuleFileNameA)>()(module, filename, size);
	}

	static DWORD __stdcall get_module_file_name_w(HMODULE module, const LPWSTR filename, const DWORD size)
	{
		if (!module)
		{
			module = loader::get_game_module();
		}

		return file_name_w_hook_.get<decltype(GetModuleFileNameW)>()(module, filename, size);
	}
};

utils::hook::detour game_module::handle_a_hook_;
utils::hook::detour game_module::handle_w_hook_;
utils::hook::detour game_module::handle_ex_a_hook_;
utils::hook::detour game_module::handle_ex_w_hook_;
utils::hook::detour game_module::file_name_a_hook_;
utils::hook::detour game_module::file_name_w_hook_;

REGISTER_MODULE(game_module)
