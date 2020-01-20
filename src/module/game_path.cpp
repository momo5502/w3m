#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "utils/io.hpp"
#include "utils/com.hpp"
#include "properties.hpp"
#include "steam_proxy.hpp"

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

private:
	static void set_witcher_path()
	{
		std::string path;
		if(properties::get("game_path", path) && try_set_witcher_path(path))
		{
			return;
		}

		if(!utils::com::select_folder(path, "Select Witcher 3 installation path", get_default_witcher_path()))
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

	static std::string get_default_witcher_path()
	{
		const auto steam_path = steam_proxy::get_steam_install_directory();
		if (steam_path.empty()) return {};

		const auto witcher_path = steam_path / "steamapps/common/The Witcher 3";
		if (!std::filesystem::exists(witcher_path)) return {};

		return witcher_path.generic_string();
	}
};

REGISTER_MODULE(game_path)
