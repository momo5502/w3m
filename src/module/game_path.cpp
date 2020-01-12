#include <std_include.hpp>
#include "loader/loader.hpp"
#include "loader/module_loader.hpp"
#include "utils/io.hpp"
#include "utils/nt.hpp"
#include "utils/com.hpp"
#include "utils/hook.hpp"
#include "properties.hpp"

class game_path final : public module
{
public:
	void post_start() override
	{
		if(!utils::io::file_exists("witcher3.exe"))
		{
			set_witcher_path();
		}
	}

	void post_load() override
	{
		// Patch the lookup of the save folder
		utils::hook::jump(0x1400EFD40_g, &get_save_folder);
	}

	void* load_import(const std::string& module, const std::string& function) override
	{
		if(function == "SHGetFolderPathW")
		{
			return &sh_get_folder_path_w;
		}

		return nullptr;
	}

private:
	static void* get_save_folder()
	{
		static struct
		{
			const wchar_t* folder = L"user";
			const size_t maybe_size = wcslen(folder);
		} save_folder;
		
		return &save_folder;
	}
	
	static HRESULT __stdcall sh_get_folder_path_w(HWND hwnd, int csidl, HANDLE token, DWORD flags, LPWSTR path)
	{
		if(csidl == CSIDL_MYDOCUMENTS)
		{
			const utils::nt::module main = loader::get_main_module();
			const auto main_path = main.get_folder();
			const std::wstring wide_path(main_path.begin(), main_path.end());
			wcscpy_s(path, MAX_PATH, wide_path.data());
			return S_OK;
		}

		return SHGetFolderPathW(hwnd, csidl, token, flags, path);
	}
	
	static void set_witcher_path()
	{
		std::string path;
		if(properties::get("game_path", path) && try_set_witcher_path(path))
		{
			return;
		}

		if(!utils::com::select_folder(path, "Select Witcher 3 installation path"))
		{
			module_loader::trigger_premature_shutdown();
		}
		else if(!try_set_witcher_path(path))
		{
			throw std::runtime_error("Witcher 3 was not found at the specified path!");
		}

		properties::set("game_path", path);
	}

	static bool try_set_witcher_path(const std::filesystem::path& path)
	{
		if (path.empty()) return false;

		if(std::filesystem::exists(path / "witcher3.exe"))
		{
			set_directory(path.generic_string());
			return true;
		}

		const auto alt_path = std::filesystem::path(path) / "bin\\x64";
		if (std::filesystem::exists(alt_path / "witcher3.exe"))
		{
			set_directory(alt_path.generic_string());
			return true;
		}

		return false;
	}

	static void set_directory(const std::string& path)
	{
		SetDllDirectoryA(path.data());
		SetCurrentDirectoryA(path.data());
	}
};

REGISTER_MODULE(game_path)
