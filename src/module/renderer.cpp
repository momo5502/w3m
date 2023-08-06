#include <std_include.hpp>
#include "renderer.hpp"

#include "loader/component_loader.hpp"

#include "utils/hook.hpp"
#include "utils/concurrent_list.hpp"

namespace renderer
{
	namespace
	{
		struct text_command
		{
			std::wstring text;
			position position;
			color color;
		};

		std::mutex mutex;
		std::vector<text_command> command_queue;
		utils::concurrent_list<std::function<void()>> callbacks;

		void draw_text_internal(void* a1, void* a2, const text_command& command)
		{
			text_object text{};
			text.text = command.text.data();
			text.length = uint32_t(command.text.size());

			static const auto draw_text_func = utils::hook::follow_branch("E8 ? ? ? ? 44 39 77 38"_sig.get(0));
			utils::hook::invoke<void>(draw_text_func, a1, a2, command.position.x, command.position.y, &text,
			                          command.color);
		}

		void execute_frame(void* a1, void* a2)
		{
			for (auto callback : callbacks)
			{
				(*callback)();
			}

			std::lock_guard _(mutex);
			for (const auto& command : command_queue)
			{
				draw_text_internal(a1, a2, command);
			}

			command_queue.clear();
		}

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				const auto console_draw_code = "33 D2 8B 88 ? ? ? ?"_sig.get(0) + 0x3A;
				const auto render_stub = utils::hook::assemble([console_draw_code](utils::hook::assembler& a)
				{
					const auto skip_console = a.newLabel();

					a.mov(rax, rdi);
					a.mov(rdx, rax);
					a.test(rcx, rcx);
					a.jz(skip_console);

					a.pushad64();
					a.call(utils::hook::follow_branch(console_draw_code + 0x8));
					a.popad64();

					a.call(&execute_frame);

					a.bind(skip_console);
					a.jmp(console_draw_code + 0xD);
				});

				utils::hook::jump(console_draw_code, render_stub);
			}
		};
	}

	color::color(const std::string& hex)
	{
		static const std::regex color_pattern("^#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?$");

		if (!std::regex_match(hex, color_pattern))
		{
			throw std::runtime_error("Invalid color");
		}

		const auto parse_byte = [&](const size_t index)
		{
			char byte_text[3] = {hex[index], hex[index + 1], 0};
			return uint8_t(strtoul(byte_text, nullptr, 16));
		};

		this->r = parse_byte(1);
		this->g = parse_byte(3);
		this->b = parse_byte(5);

		if (hex.size() == 9)
		{
			this->a = parse_byte(7);
		}
	}

	color::color(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha)
		: r(red), g(green), b(blue), a(alpha)
	{
	}

	void draw_text(const std::string& text, const position position, const color color)
	{
		text_command command;
		command.text.append(text.begin(), text.end());
		command.position = position;
		command.color = color;

		std::lock_guard _(mutex);
		command_queue.push_back(command);
	}

	void frame(const std::function<void()>& callback)
	{
		callbacks.add(callback);
	}
}

//REGISTER_COMPONENT(renderer::component)
