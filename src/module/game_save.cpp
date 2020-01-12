#include <std_include.hpp>
#include "loader/loader.hpp"
#include "loader/module_loader.hpp"
#include "utils/io.hpp"
#include "utils/nt.hpp"
#include "utils/hook.hpp"

class game_save final : public module
{
public:
	void post_load() override
	{
		// Patch the lookup of the save folder
		utils::hook::jump(0x1400EFD40_g, &get_save_folder);

		copy_settings();
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
	static void copy_settings()
	{
		const auto new_path = loader::get_main_module().get_folder() / std::filesystem::path("user");
		
		if (std::filesystem::exists(new_path)
			|| MessageBoxA(nullptr, "Do you want to import your settings and savegames from Witcher 3?",
				"Import from Witcher 3", MB_ICONINFORMATION | MB_YESNO) != IDYES) return;

		CHAR documents[MAX_PATH];
		SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, documents);

		const auto save_path = std::filesystem::path(documents) / "The Witcher 3";
		
		utils::io::copy_folder(save_path, new_path);
	}
	
	static void* get_save_folder()
	{
		static struct
		{
			const wchar_t* folder = L"user";
			const size_t maybe_size = wcslen(folder);
		} save_folder;
		
		return &save_folder;
	}
	
	static HRESULT __stdcall sh_get_folder_path_w(const HWND hwnd, const int csidl, const HANDLE token, const DWORD flags, const LPWSTR path)
	{
		if(csidl == CSIDL_MYDOCUMENTS)
		{
			const auto main = loader::get_main_module();
			const auto main_path = main.get_folder();
			const std::wstring wide_path(main_path.begin(), main_path.end());
			wcscpy_s(path, MAX_PATH, wide_path.data());
			return S_OK;
		}

		return SHGetFolderPathW(hwnd, csidl, token, flags, path);
	}
};

REGISTER_MODULE(game_save)
