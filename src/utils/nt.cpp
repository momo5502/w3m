#include <std_include.hpp>
#include "nt.hpp"

namespace utils::nt
{
	module module::load(const std::string& name)
	{
		return module(LoadLibraryA(name.data()));
	}

	module module::load(const std::filesystem::path& path)
	{
		return module::load(path.generic_string());
	}

	module module::get_by_address(void* address)
	{
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(address), &handle);
		return module(handle);
	}

	module::module()
	{
		this->module_ = GetModuleHandleA(nullptr);
	}

	module::module(const std::string& name)
	{
		this->module_ = GetModuleHandleA(name.data());
	}

	module::module(const HMODULE handle)
	{
		this->module_ = handle;
	}

	bool module::operator==(const module& obj) const
	{
		return this->module_ == obj.module_;
	}

	module::operator bool() const
	{
		return this->is_valid();
	}

	module::operator HMODULE() const
	{
		return this->get_handle();
	}

	PIMAGE_NT_HEADERS module::get_nt_headers() const
	{
		if (!this->is_valid()) return nullptr;
		return reinterpret_cast<PIMAGE_NT_HEADERS>(this->get_ptr() + this->get_dos_header()->e_lfanew);
	}

	PIMAGE_DOS_HEADER module::get_dos_header() const
	{
		return reinterpret_cast<PIMAGE_DOS_HEADER>(this->get_ptr());
	}

	PIMAGE_OPTIONAL_HEADER module::get_optional_header() const
	{
		if (!this->is_valid()) return nullptr;
		return &this->get_nt_headers()->OptionalHeader;
	}

	std::vector<PIMAGE_SECTION_HEADER> module::get_section_headers() const
	{
		std::vector<PIMAGE_SECTION_HEADER> headers;

		auto nt_headers = this->get_nt_headers();
		auto section = IMAGE_FIRST_SECTION(nt_headers);

		for (uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i, ++section)
		{
			if (section) headers.push_back(section);
			else OutputDebugStringA("There was an invalid section :O");
		}

		return headers;
	}

	std::uint8_t* module::get_ptr() const
	{
		return reinterpret_cast<std::uint8_t*>(this->module_);
	}

	void module::unprotect() const
	{
		if (!this->is_valid()) return;

		DWORD protection;
		VirtualProtect(this->get_ptr(), this->get_optional_header()->SizeOfImage, PAGE_EXECUTE_READWRITE,
		               &protection);
	}

	size_t module::get_relative_entry_point() const
	{
		if (!this->is_valid()) return 0;
		return this->get_nt_headers()->OptionalHeader.AddressOfEntryPoint;
	}

	void* module::get_entry_point() const
	{
		if (!this->is_valid()) return nullptr;
		return this->get_ptr() + this->get_relative_entry_point();
	}

	bool module::is_valid() const
	{
		return this->module_ != nullptr && this->get_dos_header()->e_magic == IMAGE_DOS_SIGNATURE;
	}

	std::string module::get_name() const
	{
		if (!this->is_valid()) return "";

		auto path = this->get_path();
		const auto pos = path.find_last_of("/\\");
		if (pos == std::string::npos) return path;

		return path.substr(pos + 1);
	}

	std::string module::get_path() const
	{
		if (!this->is_valid()) return "";

		char name[MAX_PATH] = {0};
		GetModuleFileNameA(this->module_, name, sizeof name);

		return name;
	}

	std::string module::get_folder() const
	{
		if (!this->is_valid()) return "";

		const auto path = std::filesystem::path(this->get_path());
		return path.parent_path().generic_string();
	}

	void module::free()
	{
		if (this->is_valid())
		{
			FreeLibrary(this->module_);
			this->module_ = nullptr;
		}
	}

	HMODULE module::get_handle() const
	{
		return this->module_;
	}

	void** module::get_iat_entry(const std::string& module_name, const std::string& proc_name) const
	{
		if (!this->is_valid()) return nullptr;

		const module other_module(module_name);
		if (!other_module.is_valid()) return nullptr;

		const auto target_function = other_module.get_proc<void*>(proc_name);
		if (!target_function) return nullptr;

		auto* header = this->get_optional_header();
		if (!header) return nullptr;

		auto* import_descriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(this->get_ptr() + header->DataDirectory
			[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		while (import_descriptor->Name)
		{
			if (!_stricmp(reinterpret_cast<char*>(this->get_ptr() + import_descriptor->Name), module_name.data()))
			{
				auto* original_thunk_data = reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->
					OriginalFirstThunk + this->get_ptr());
				auto* thunk_data = reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->FirstThunk + this->
					get_ptr());

				while (original_thunk_data->u1.AddressOfData)
				{
					const size_t ordinal_number = original_thunk_data->u1.AddressOfData & 0xFFFFFFF;

					if (ordinal_number > 0xFFFF) continue;

					if (GetProcAddress(other_module.module_, reinterpret_cast<char*>(ordinal_number)) ==
						target_function)
					{
						return reinterpret_cast<void**>(&thunk_data->u1.Function);
					}

					++original_thunk_data;
					++thunk_data;
				}

				//break;
			}

			++import_descriptor;
		}

		return nullptr;
	}

	void raise_hard_exception()
	{
		int data = false;
		const module ntdll("ntdll.dll");
		ntdll.invoke_pascal<void>("RtlAdjustPrivilege", 19, true, false, &data);
		ntdll.invoke_pascal<void>("NtRaiseHardError", 0xC000007B, 0, nullptr, nullptr, 6, &data);
	}
}
