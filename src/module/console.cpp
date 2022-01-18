#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "renderer.hpp"

namespace
{
	namespace console
	{
		utils::hook::detour command_handler_hook;
		using command_callback = std::function<void()>;
		std::unordered_map<std::string, command_callback> commands;

		void command_execution_stub(void* a1, renderer::text_object* text, void* a3)
		{
			const std::wstring_view string_view(text->text, std::max(text->length, 1u) - 1);
			const std::string new_string(string_view.begin(), string_view.end());

			const auto command = commands.find(new_string);
			if (command != commands.end())
			{
				command->second();
			}
			else
			{
				command_handler_hook.invoke<void>(a1, text, a3);
			}
		}

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				// Enable ingame console
				const auto config_vars = "4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? 45 33 C9 C7 44 24"_sig;

				for (size_t i = 0; i < config_vars.count(); ++i)
				{
					const auto var = config_vars.get(i);
					const auto string = utils::hook::extract<char*>(var + 3);
					if (string == "DBGConsoleOn"s)
					{
						utils::hook::set<BYTE>(var + 0x6F, 1);
						break;
					}
				}

				commands["quit"] = []()
				{
					exit(0);
				};

				const auto execution_handler = "E8 ? ? ? ? 48 8D 4D 88 E8 ? ? ? ? 49 83 BE ? ? ? ? ?"_sig.get(0);
				const auto command_handler = utils::hook::extract<void*>(execution_handler + 1);

				command_handler_hook = utils::hook::detour(command_handler, command_execution_stub);
			}
		};
	}
}

REGISTER_COMPONENT(console::component)
