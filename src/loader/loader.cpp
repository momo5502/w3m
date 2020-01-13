#include <std_include.hpp>
#include "loader.hpp"
#include "utils/string.hpp"
#include "module_loader.hpp"

namespace loader
{
	utils::nt::module game_module;
	utils::nt::module main_module;

	void load_imports(const utils::nt::module& target, const resolver& import_resolver)
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

					if (import_resolver)
					{
						function = FARPROC(import_resolver(name, function_name));
					}

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

	utils::nt::module get_game_module()
	{
		return game_module;
	}

	utils::nt::module get_main_module()
	{
		return main_module;
	}

	utils::nt::module load(const std::string& name, const resolver& import_resolver)
	{
		game_module = utils::nt::module::load(name);
		if(!game_module)
		{
			throw std::runtime_error(utils::string::va("Unable to load '%s'", name.data()));
		}

		load_imports(game_module, import_resolver);

		return game_module;
	}

	size_t reverse_g(const size_t val)
	{
		static auto base = size_t(loader::get_game_module().get_ptr());
		assert(base != size_t(loader::get_main_module().get_ptr()));
		return 0x140000000 + (val - base);
	}

	size_t reverse_g(const void* val)
	{
		return reverse_g(size_t(val));
	}
}

size_t operator"" _g(const size_t val)
{
	static auto base = size_t(loader::get_game_module().get_ptr());
	assert(base != size_t(loader::get_main_module().get_ptr()));
	return base + (val - 0x140000000);
}