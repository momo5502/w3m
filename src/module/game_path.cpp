#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "utils/io.hpp"
#include "utils/com.hpp"
#include "properties.hpp"
#include "steam_proxy.hpp"

namespace
{
	namespace game_path
	{
		void set_directory(const std::filesystem::path& path)
		{
			const auto wide_path = path.generic_wstring();
			SetDllDirectoryW(wide_path.data());
			SetCurrentDirectoryW(wide_path.data());
		}

		std::string get_default_witcher_path()
		{
			const auto steam_path = steam_proxy::get_steam_install_directory();
			if (steam_path.empty()) return {};

			const auto witcher_path = steam_path / "steamapps/common/The Witcher 3";
			if (!std::filesystem::exists(witcher_path)) return {};

			return witcher_path.generic_string();
		}

		bool try_set_witcher_path(const std::filesystem::path& path)
		{
			if (path.empty())
			{
				return false;
			}

			if (std::filesystem::exists(path / "witcher3.exe"))
			{
				set_directory(path);
				return true;
			}

			const auto alt_path = std::filesystem::path(path) / "bin" / "x64";
			if (std::filesystem::exists(alt_path / "witcher3.exe"))
			{
				set_directory(alt_path);
				return true;
			}

			return false;
		}

		void set_witcher_path()
		{
			std::string path;
			if (properties::get("game_path", path) && try_set_witcher_path(path))
			{
				return;
			}

			if (!utils::com::select_folder(path, "Select Witcher 3 installation path", get_default_witcher_path()))
			{
				component_loader::trigger_premature_shutdown();
			}
			else if (!try_set_witcher_path(path))
			{
				throw std::runtime_error("Witcher 3 was not found at the specified path!");
			}

			properties::set("game_path", path);
		}

		class component final : public component_interface
		{
		public:
			void post_start() override
			{
				if (!utils::io::file_exists("witcher3.exe"))
				{
					set_witcher_path();
				}
			}
		};
	}
}

REGISTER_COMPONENT(game_path::component)
