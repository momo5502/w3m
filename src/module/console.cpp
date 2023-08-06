#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

namespace
{
	namespace console
	{
		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				// Enable ingame console
				const auto config_vars = "48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 05 ? ? ? ? C6 05 ? ? ? ? ?"_sig;

				for (size_t i = 0; i < config_vars.count(); ++i)
				{
					const auto var = config_vars.get(i);
					const auto array = utils::hook::extract<char**>(var + 3);
					if (array[2] == "DBGConsoleOn"s)
					{
						utils::hook::set<BYTE>(var + 0x19, 1);
						break;
					}
				}
			}
		};
	}
}

REGISTER_COMPONENT(console::component)
