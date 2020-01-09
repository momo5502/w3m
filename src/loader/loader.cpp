#include <std_include.hpp>
#include "loader.hpp"
#include "utils/string.hpp"

namespace loader
{
utils::nt::module main_module;

HMODULE __stdcall GetModuleHandleA_(LPCSTR lpModuleName)
{
	if (!lpModuleName) {
		return main_module;
	}

	return GetModuleHandleA(lpModuleName);
}

HMODULE __stdcall GetModuleHandleW_(LPWSTR lpModuleName)
{
	if (!lpModuleName) {
		return main_module;
	}

	return GetModuleHandleW(lpModuleName);
}

BOOL __stdcall GetModuleHandleExW_(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule)
{
	if (!lpModuleName) {
		*phModule = main_module;
		return TRUE;
	}

	return GetModuleHandleExW(dwFlags, lpModuleName, phModule);
}

DWORD __stdcall GetModuleFileNameW_(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	if (!hModule) {
		hModule = main_module;
	}

	return GetModuleFileNameW(hModule, lpFilename, nSize);
}

void* resolve_special_imports(const std::string& module, const std::string& function)
{
	if (function == "GetModuleHandleA") {
		return GetModuleHandleA_;
	}
	else if (function == "GetModuleHandleW") {
		return GetModuleHandleW_;
	}
	else if (function == "GetModuleHandleExW") {
		return GetModuleHandleExW_;
	}
	else if (function == "GetModuleFileNameW") {
		return GetModuleFileNameW_;
	}

	return nullptr;
}

void load_imports(const utils::nt::module& target)
{
	const auto import_directory = &target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	auto descriptor = PIMAGE_IMPORT_DESCRIPTOR(target.get_ptr() + import_directory->VirtualAddress);

	while (descriptor->Name) {
		std::string name = LPSTR(target.get_ptr() + descriptor->Name);

		auto name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->OriginalFirstThunk);
		auto address_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);

		if (!descriptor->OriginalFirstThunk) {
			name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);
		}

		while (*name_table_entry) {
			FARPROC function = nullptr;
			std::string function_name;

			if (IMAGE_SNAP_BY_ORDINAL(*name_table_entry))
			{
				auto module = utils::nt::module::load(name);
				if (module) {
					function = GetProcAddress(module, MAKEINTRESOURCEA(IMAGE_ORDINAL(*name_table_entry)));
				}

				function_name = "#" + std::to_string(IMAGE_ORDINAL(*name_table_entry));
			}
			else {
				auto import = PIMAGE_IMPORT_BY_NAME(target.get_ptr() + *name_table_entry);
				function_name = import->Name;
				function = FARPROC(resolve_special_imports(name, function_name));

				if (!function) {
					auto module = utils::nt::module::load(name);
					if (module) {
						function = GetProcAddress(module, function_name.data());
					}
				}
			}

			if (!function)
			{
				throw std::runtime_error(utils::string::va("Unable to load import '%s' from module '%s'",
					function_name.data(), name.data()));
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
	load_imports(main_module);

	return main_module;
}
}
