#include "../std_include.hpp"

#include "../loader/loader.hpp"

#include "properties.hpp"
#include "utils/nt.hpp"

namespace properties::detail
{
	namespace
	{
		std::string get_path()
		{
			auto path = loader::get_main_module().get_path();
			path.replace_extension(".json");
			return path.generic_string();
		}
	}

	utils::properties& get_properties()
	{
		static utils::properties properties{get_path()};
		return properties;
	}
}
