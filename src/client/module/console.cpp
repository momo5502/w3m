#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <utils/hook.hpp>

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
			if (GetStdHandle(STD_OUTPUT_HANDLE))
			{
				return;
			}

			AllocConsole();
			AttachConsole(GetCurrentProcessId());

			FILE* fp{};
			(void)freopen_s(&fp, "CONIN$", "r", stdin);
			(void)freopen_s(&fp, "CONOUT$", "w", stdout);
			(void)freopen_s(&fp, "CONOUT$", "w", stderr);

			SetConsoleTitleA("Witcher 3: Console");

			SetConsoleCtrlHandler(+[](DWORD /*ctrl_type*/) -> BOOL
			{
				TerminateProcess(GetCurrentProcess(), 0);
				return TRUE;
			}, TRUE);
		}
	}

	struct component final : component_interface
	{
		void post_start() override
		{
			if (!utils::nt::is_wine())
			{
				create_console();
			}
		}

		void post_load() override
		{
			if (!utils::nt::is_wine())
			{
				utils::hook::jump(0x14025D5B0_g, log_message_stub);
			}

			// Change console font
			utils::hook::inject(0x14011668D_g, 0x142148C18_g);

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

		component_priority priority() const override
		{
			return component_priority::console;
		}
	};
}

REGISTER_COMPONENT(console::component)
