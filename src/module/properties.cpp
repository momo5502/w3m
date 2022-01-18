#include <std_include.hpp>
#include "properties.hpp"
#include "utils/nt.hpp"
#include "loader/loader.hpp"

namespace properties::detail
{
	namespace
	{
		std::string get_path()
		{
			const auto out_folder = std::filesystem::path(loader::get_main_module().get_folder());
			const auto out_path = out_folder / "w3x.json";
			return out_path.generic_string();
		}
	}

	utils::properties& get_properties()
	{
		static utils::properties properties{get_path()};
		return properties;
	}
}
