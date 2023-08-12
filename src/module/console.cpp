#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "loader/loader.hpp"
#include "utils/hook.hpp"

namespace console
{
	namespace
	{
		void log_message_stub(void* /*this_*/, const wchar_t* format, ...)
		{
			va_list ap{};
			va_start(ap, format);

			_vfwprintf_p(stdout, (std::wstring(format) + L"\n").data(), ap);

			va_end(ap);
		}

		void create_console()
		{
			AllocConsole();
			AttachConsole(GetCurrentProcessId());
			ShowWindow(GetConsoleWindow(), SW_SHOW);

			FILE* fp{};
			freopen_s(&fp, "CONIN$", "r", stdin);
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);

			SetConsoleTitleA("Witcher 3: Console");

			SetConsoleCtrlHandler(+[](DWORD /*ctrl_type*/) -> BOOL
			{
				_Exit(0);
			}, TRUE);
		}
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			create_console();

			utils::hook::jump(0x14025D5B0_g, log_message_stub);

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

REGISTER_COMPONENT(console::component)
