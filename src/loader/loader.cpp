#include <std_include.hpp>
#include "loader.hpp"
#include "utils/string.hpp"
#include "module_loader.hpp"

namespace loader
{
	utils::nt::module main_module;

	DECLSPEC_NORETURN void WINAPI exit_hook(const int code)
	{
		module_loader::pre_destroy();
		exit(code);
	}

	HMODULE __stdcall get_module_handle_a(LPCSTR module_name)
	{
		if (!module_name)
		{
			return main_module;
		}

		return GetModuleHandleA(module_name);
	}

	HMODULE __stdcall get_module_handle_w(LPWSTR module_name)
	{
		if (!module_name)
		{
			return main_module;
		}

		return GetModuleHandleW(module_name);
	}

	BOOL __stdcall get_module_handle_ex_w(DWORD flags, LPCWSTR module_name, HMODULE* module)
	{
		if (!module_name)
		{
			*module = main_module;
			return TRUE;
		}

		return GetModuleHandleExW(flags, module_name, module);
	}

	DWORD __stdcall get_module_file_name_w(HMODULE module, LPWSTR filename, DWORD size)
	{
		if (!module)
		{
			module = main_module;
		}

		return GetModuleFileNameW(module, filename, size);
	}

	void* resolve_special_imports(const std::string& module, const std::string& function)
	{
		if (function == "GetModuleHandleA")
		{
			return get_module_handle_a;
		}

		if (function == "GetModuleHandleW")
		{
			return get_module_handle_w;
		}

		if (function == "GetModuleHandleExW")
		{
			return get_module_handle_ex_w;
		}

		if (function == "GetModuleFileNameW")
		{
			return get_module_file_name_w;
		}

		if (function == "ExitProcess")
		{
			return exit_hook;
		}

		return nullptr;
	}

	void load_imports(const utils::nt::module& target)
	{
		const auto import_directory = &target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		auto descriptor = PIMAGE_IMPORT_DESCRIPTOR(target.get_ptr() + import_directory->VirtualAddress);

		while (descriptor->Name)
		{
			std::string name = LPSTR(target.get_ptr() + descriptor->Name);

			auto name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->OriginalFirstThunk);
			auto address_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);

			if (!descriptor->OriginalFirstThunk)
			{
				name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);
			}

			while (*name_table_entry)
			{
				FARPROC function = nullptr;
				std::string function_name;

				if (IMAGE_SNAP_BY_ORDINAL(*name_table_entry))
				{
					auto module = utils::nt::module::load(name);
					if (module)
					{
						function = GetProcAddress(module, MAKEINTRESOURCEA(IMAGE_ORDINAL(*name_table_entry)));
					}

					function_name = "#" + std::to_string(IMAGE_ORDINAL(*name_table_entry));
				}
				else
				{
					auto import = PIMAGE_IMPORT_BY_NAME(target.get_ptr() + *name_table_entry);
					function_name = import->Name;
					function = FARPROC(resolve_special_imports(name, function_name));

					if (!function)
					{
						auto module = utils::nt::module::load(name);
						if (module)
						{
							function = GetProcAddress(module, function_name.data());
						}
					}
				}

				if (!function)
				{
					throw std::runtime_error(utils::string::va("Unable to load import '%s' from module '%s'", function_name.data(), name.data()));
				}

				DWORD old_protect;
				VirtualProtect(address_table_entry, sizeof(*address_table_entry), PAGE_EXECUTE_READWRITE, &old_protect);
				*address_table_entry = reinterpret_cast<uintptr_t>(function);

				name_table_entry++;
				address_table_entry++;
			}

			descriptor++;
		}
	}

	utils::nt::module get_module()
	{
		return main_module;
	}

	utils::nt::module load(const std::string& name)
	{
		main_module = utils::nt::module::load(name);
		if(!main_module)
		{
			throw std::runtime_error(utils::string::va("Unable to load '%s'", name.data()));
		}

		load_imports(main_module);

		return main_module;
	}
}
