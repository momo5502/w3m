#include <std_include.hpp>
#include "loader/module_loader.hpp"

class console final : public module
{
public:
	void post_start() override
	{
		FreeConsole();
	}
};

REGISTER_MODULE(console)
