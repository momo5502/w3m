#include <std_include.hpp>
#include "loader/loader.hpp"
#include "loader/component_loader.hpp"
#include "utils/io.hpp"
#include "utils/nt.hpp"
#include "utils/hook.hpp"

namespace
{
	namespace game_save
	{
		void copy_settings()
		{
			const auto new_path = loader::get_main_module().get_folder() / std::filesystem::path("user");

			CHAR documents[MAX_PATH];
			SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, documents);

			const auto save_path = std::filesystem::path(documents) / "The Witcher 3";

			if (std::filesystem::exists(new_path)
				|| !std::filesystem::exists(save_path)
				|| MessageBoxA(nullptr, "Do you want to import your settings and savegames from Witcher 3?",
				               "Import from Witcher 3", MB_ICONINFORMATION | MB_YESNO) != IDYES)
				return;

			utils::io::copy_folder(save_path, new_path);
		}

		void* get_save_folder()
		{
			static struct
			{
				const wchar_t* folder = L"user";
				const size_t maybe_size = wcslen(folder);
			} save_folder;

			return &save_folder;
		}

		HRESULT __stdcall sh_get_folder_path_w(const HWND hwnd, const int csidl, const HANDLE token, const DWORD flags,
		                                       const LPWSTR path)
		{
			if (csidl == CSIDL_MYDOCUMENTS)
			{
				const auto main = loader::get_main_module();
				const auto main_path = main.get_folder().generic_wstring();
				wcscpy_s(path, MAX_PATH, main_path.data());
				return S_OK;
			}

			return SHGetFolderPathW(hwnd, csidl, token, flags, path);
		}

		class component final : public component_interface
		{
		public:
			void post_start() override
			{
				copy_settings();
			}

			void post_load() override
			{
				const auto get_save_folder_call = "E8 ? ? ? ? 83 78 08 01 76 03 48 8B 18 4C 8D 44 24 ?"_sig.get(0);
				utils::hook::jump(utils::hook::follow_branch(get_save_folder_call), &get_save_folder);
			}

			void* load_import([[maybe_unused]] const std::string& module, const std::string& function) override
			{
				if (function == "SHGetFolderPathW")
				{
					return &sh_get_folder_path_w;
				}

				return nullptr;
			}
		};
	}
}

//REGISTER_COMPONENT(game_save::component)
