#pragma once
#include "loader/module_loader.hpp"
#include "utils/concurrent_list.hpp"

class renderer final : public module
{	
public:
	// TODO: Probably wrong
	struct text_object
	{
		const wchar_t* text = nullptr;
		uint32_t length = 0;
		uint32_t idk = 0;
	};
	
	class position
	{
	public:
		float x = 0.0f;
		float y = 0.0f;
	};

	class color
	{
	public:
		color() = default;
		color(const std::string& hex);
		color(const char* hex) : color(std::string(hex)) {}
		color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

		uint8_t r = 0xFF;
		uint8_t g = 0xFF;
		uint8_t b = 0xFF;
		uint8_t a = 0xFF;
	};

	static void draw_text(const std::string& text, position position, color color);
	static void frame(const std::function<void()>& callback);

	void post_load() override;

private:
	class text_command
	{
	public:
		std::wstring text;
		position position;
		color color;
	};

	std::mutex mutex_;
	std::vector<text_command> command_queue_;
	utils::concurrent_list<std::function<void()>> callbacks_;

	void execute_frame(void* a1, void* a2);
	static void execute_frame_static(void* a1, void* a2);
	static void draw_text_internal(void* a1, void* a2, const text_command& command);
};
