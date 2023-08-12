#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "scripting.hpp"

namespace scripting_experiments
{
	namespace
	{
		std::string my_test_function(const std::string& game_string)
		{
			return "New string: " + game_string;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			scripting::register_function<my_test_function>(L"Testicles");
		}
	};
}

REGISTER_COMPONENT(scripting_experiments::component)
