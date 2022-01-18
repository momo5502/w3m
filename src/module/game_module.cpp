#include <std_include.hpp>
#include "loader/loader.hpp"
#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

namespace
{
	namespace game_module
	{
		utils::hook::detour handle_a_hook;
		utils::hook::detour handle_w_hook;
		utils::hook::detour handle_ex_a_hook;
		utils::hook::detour handle_ex_w_hook;
		utils::hook::detour file_name_a_hook;
		utils::hook::detour file_name_w_hook;

		HMODULE __stdcall get_module_handle_a(const LPCSTR module_name)
		{
			if (!module_name)
			{
				return loader::get_game_module();
			}

			return handle_a_hook.invoke<HMODULE>(module_name);
		}

		HMODULE __stdcall get_module_handle_w(const LPWSTR module_name)
		{
			if (!module_name)
			{
				return loader::get_game_module();
			}

			return handle_w_hook.invoke<HMODULE>(module_name);
		}

		BOOL __stdcall get_module_handle_ex_a(const DWORD flags, const LPCSTR module_name, HMODULE* module)
		{
			if (!module_name)
			{
				*module = loader::get_game_module();
				return TRUE;
			}

			return handle_ex_a_hook.invoke<BOOL>(flags, module_name, module);
		}

		BOOL __stdcall get_module_handle_ex_w(const DWORD flags, const LPCWSTR module_name, HMODULE* module)
		{
			if (!module_name)
			{
				*module = loader::get_game_module();
				return TRUE;
			}

			return handle_ex_w_hook.invoke<BOOL>(flags, module_name, module);
		}

		DWORD __stdcall get_module_file_name_a(HMODULE module, const LPSTR filename, const DWORD size)
		{
			if (!module)
			{
				module = loader::get_game_module();
			}

			return file_name_a_hook.invoke<DWORD>(module, filename, size);
		}

		DWORD __stdcall get_module_file_name_w(HMODULE module, const LPWSTR filename, const DWORD size)
		{
			if (!module)
			{
				module = loader::get_game_module();
			}

			return file_name_w_hook.invoke<DWORD>(module, filename, size);
		}

		class component final : public component_interface
		{
		public:
			void post_start() override
			{
				handle_a_hook = utils::hook::detour(&GetModuleHandleA, &get_module_handle_a);
				handle_w_hook = utils::hook::detour(&GetModuleHandleW, &get_module_handle_w);
				handle_ex_w_hook = utils::hook::detour(&GetModuleHandleExA, &get_module_handle_ex_a);
				handle_ex_w_hook = utils::hook::detour(&GetModuleHandleExW, &get_module_handle_ex_w);
				file_name_a_hook = utils::hook::detour(&GetModuleFileNameA, &get_module_file_name_a);
				file_name_w_hook = utils::hook::detour(&GetModuleFileNameW, &get_module_file_name_w);
			}
		};
	}
}

REGISTER_COMPONENT(game_module::component)
