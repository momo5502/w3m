#include "../std_include.hpp"
#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include "renderer.hpp"

#include <utils/hook.hpp>
#include <utils/concurrency.hpp>

#include "scripting.hpp"

namespace renderer
{
	namespace
	{
		struct text_command
		{
			std::string text{};
			position position{};
			color color{};
		};

		using command_queue = std::queue<text_command>;
		utils::concurrency::container<command_queue> render_commands{};

		using frame_callbacks = std::list<frame_callback>;
		utils::concurrency::container<frame_callbacks> callbacks{};


		void render_text(void* a2, float x, float y, const scripting::string& text, const color& color)
		{
			auto* a1 = *reinterpret_cast<void**>(0x144E063E0_g);

			reinterpret_cast<void(*)(void*, void*, float, float, const scripting::string&, uint32_t)>(
				0x140471800_g)(a1, a2, x, y, text, *reinterpret_cast<const uint32_t*>(&color.r));
		}

		void renderer_stub(void* a2)
		{
			if (!a2)
			{
				return;
			}

			callbacks.access([&](const frame_callbacks& callback_list)
			{
				for (const auto& cb : callback_list)
				{
					cb();
				}
			});

			command_queue queue{};

			render_commands.access([&queue](command_queue& commands)
			{
				if (!commands.empty())
				{
					commands.swap(queue);
				}
			});

			while (!queue.empty())
			{
				auto& command = queue.front();

				render_text(a2, command.position.x, command.position.y, {command.text}, command.color);

				queue.pop();
			}
		}

		struct component final : component_interface
		{
			void post_load() override
			{
				utils::hook::jump(0x1404697CE_g, utils::hook::assemble([](utils::hook::assembler& a)
				{
					a.call(0x14046FCF0_g);
					a.pushaq();
					a.mov(rcx, rbx);
					a.call_aligned(renderer_stub);
					a.popaq();
					a.jmp(0x1404697D3_g);
				}));
			}
		};
	}

	void on_frame(frame_callback callback)
	{
		callbacks.access([&](frame_callbacks& callback_list)
		{
			callback_list.emplace_back(std::move(callback));
		});
	}

	void draw_text(std::string text, const position position, const color color)
	{
		text_command command{};
		command.text = std::move(text);
		command.position = position;
		command.color = color;

		render_commands.access([&command](command_queue& commands)
		{
			commands.emplace(std::move(command));
		});
	}
}

REGISTER_COMPONENT(renderer::component)
