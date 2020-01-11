#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "utils/hook.hpp"

class console final : public module
{
public:
	void post_start() override
	{
		FreeConsole();
	}
	
	void post_load() override
	{
		// Enable ingame console
		utils::hook::set<BYTE>(0x141B4D183_g, 1);
	}
};

REGISTER_MODULE(console)
