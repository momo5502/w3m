#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "utils/hook.hpp"

class console final : public module
{
public:
	void post_load() override
	{
		// Enable ingame console
		const auto config_vars = "4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? 45 33 C9 C7 44 24"_sig;

		for(size_t i = 0; i < config_vars.count(); ++i)
		{
			const auto var = config_vars.get(i);
			const auto string = utils::hook::extract<char*>(var + 3);
			if(string == "DBGConsoleOn"s)
			{
				utils::hook::set<BYTE>(var + 0x6F, 1);
				break;
			}
		}
	}
};

REGISTER_MODULE(console)
