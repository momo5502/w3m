#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "utils/io.hpp"
#include "utils/com.hpp"
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

private:
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
			SetCurrentDirectoryA(path.generic_string().data());
			return true;
		}

		const auto alt_path = std::filesystem::path(path) / "bin\\x64";
		if (std::filesystem::exists(alt_path / "witcher3.exe"))
		{
			SetCurrentDirectoryA(alt_path.generic_string().data());
			return true;
		}

		return false;
	}
};

REGISTER_MODULE(game_path)
