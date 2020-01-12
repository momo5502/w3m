#include <std_include.hpp>
#include "loader/loader.hpp"
#include "loader/module_loader.hpp"
#include "utils/hook.hpp"

class game_module final : public module
{
public:
	void post_start() override
	{
		get_module_handle_a_hook = utils::hook::detour(&GetModuleHandleA, &get_module_handle_a);
		get_module_handle_w_hook = utils::hook::detour(&GetModuleHandleW, &get_module_handle_w);
		get_module_handle_ex_w_hook = utils::hook::detour(&GetModuleHandleExA, &get_module_handle_ex_a);
		get_module_handle_ex_w_hook = utils::hook::detour(&GetModuleHandleExW, &get_module_handle_ex_w);
		get_module_file_name_a_hook = utils::hook::detour(&GetModuleFileNameA, &get_module_file_name_a);
		get_module_file_name_w_hook = utils::hook::detour(&GetModuleFileNameW, &get_module_file_name_w);
	}

private:
	static utils::hook::detour get_module_handle_a_hook;
	static utils::hook::detour get_module_handle_w_hook;
	static utils::hook::detour get_module_handle_ex_a_hook;
	static utils::hook::detour get_module_handle_ex_w_hook;
	static utils::hook::detour get_module_file_name_a_hook;
	static utils::hook::detour get_module_file_name_w_hook;
	
	static HMODULE __stdcall get_module_handle_a(const LPCSTR module_name)
	{
		if (!module_name)
		{
			return loader::get_game_module();
		}

		return get_module_handle_a_hook.get<decltype(GetModuleHandleA)>()(module_name);
	}

	static HMODULE __stdcall get_module_handle_w(const LPWSTR module_name)
	{
		if (!module_name)
		{
			return loader::get_game_module();
		}

		return get_module_handle_w_hook.get<decltype(GetModuleHandleW)>()(module_name);
	}

	static BOOL __stdcall get_module_handle_ex_a(const DWORD flags, const LPCSTR module_name, HMODULE* module)
	{
		if (!module_name)
		{
			*module = loader::get_game_module();
			return TRUE;
		}

		return get_module_handle_ex_a_hook.get<decltype(GetModuleHandleExA)>()(flags, module_name, module);
	}

	static BOOL __stdcall get_module_handle_ex_w(const DWORD flags, const LPCWSTR module_name, HMODULE* module)
	{
		if (!module_name)
		{
			*module = loader::get_game_module();
			return TRUE;
		}

		return get_module_handle_ex_w_hook.get<decltype(GetModuleHandleExW)>()(flags, module_name, module);
	}

	static DWORD __stdcall get_module_file_name_a(HMODULE module, const LPSTR filename, const DWORD size)
	{
		if (!module)
		{
			module = loader::get_game_module();
		}

		return get_module_file_name_a_hook.get<decltype(GetModuleFileNameA)>()(module, filename, size);
	}

	static DWORD __stdcall get_module_file_name_w(HMODULE module, const LPWSTR filename, const DWORD size)
	{
		if (!module)
		{
			module = loader::get_game_module();
		}

		return get_module_file_name_w_hook.get<decltype(GetModuleFileNameW)>()(module, filename, size);
	}
};

utils::hook::detour game_module::get_module_handle_a_hook;
utils::hook::detour game_module::get_module_handle_w_hook;
utils::hook::detour game_module::get_module_handle_ex_a_hook;
utils::hook::detour game_module::get_module_handle_ex_w_hook;
utils::hook::detour game_module::get_module_file_name_a_hook;
utils::hook::detour game_module::get_module_file_name_w_hook;

REGISTER_MODULE(game_module)
