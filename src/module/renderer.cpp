#include <std_include.hpp>
#include "utils/hook.hpp"
#include "loader/loader.hpp"
#include "renderer.hpp"

renderer::color::color(const std::string& hex)
{
	static const std::regex color_pattern("^#[0-9A-Fa-f]{6}([0-9A-Fa-f]{2})?$");

	if(!std::regex_match(hex, color_pattern))
	{
		throw std::runtime_error("Invalid color");
	}

	const auto parse_byte = [&](const size_t index)
	{
		char byte_text[3] = { hex[index], hex[index + 1], 0 };
		return uint8_t(strtoul(byte_text, nullptr, 16));
	};

	this->r = parse_byte(1);
	this->g = parse_byte(3);
	this->b = parse_byte(5);

	if(hex.size() == 9)
	{
		this->a = parse_byte(7);
	}
}

renderer::color::color(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha)
	: r(red), g(green), b(blue), a(alpha)
{

}

void renderer::draw_text(const std::string& text, const position position, const color color)
{
	auto* instance = module_loader::get<renderer>();
	if (instance)
	{
		text_command command;
		command.text.append(text.begin(), text.end());
		command.position = position;
		command.color = color;

		std::lock_guard _(instance->mutex_);
		instance->command_queue_.push_back(command);
	}
}

void renderer::frame(const std::function<void()>& callback)
{
	auto* instance = module_loader::get<renderer>();
	if (instance)
	{
		instance->callbacks_.add(callback);
	}
}

void renderer::post_load()
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

		a.call(&execute_frame_static);

		a.bind(skip_console);
		a.jmp(console_draw_code + 0xD);
	});

	utils::hook::jump(console_draw_code, render_stub);
}

void renderer::execute_frame(void* a1, void* a2)
{
	for (auto callback : this->callbacks_)
	{
		(*callback)();
	}

	std::lock_guard _(this->mutex_);
	for (const auto& command : this->command_queue_)
	{
		draw_text_internal(a1, a2, command);
	}

	this->command_queue_.clear();
}

void renderer::execute_frame_static(void* a1, void* a2)
{
	auto* instance = module_loader::get<renderer>();
	if (instance)
	{
		instance->execute_frame(a1, a2);
	}
}

void renderer::draw_text_internal(void* a1, void* a2, const text_command& command)
{
	text_object text{};
	text.text = command.text.data();
	text.length = uint32_t(command.text.size());

	static const auto draw_text_func = utils::hook::follow_branch("E8 ? ? ? ? 44 39 77 38"_sig.get(0));
	utils::hook::invoke<void>(draw_text_func, a1, a2, command.position.x, command.position.y, &text, command.color);
}

REGISTER_MODULE(renderer);