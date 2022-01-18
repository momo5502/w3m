#pragma once

namespace renderer
{
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

		color(const char* hex) : color(std::string(hex))
		{
		}

		color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

		uint8_t r = 0xFF;
		uint8_t g = 0xFF;
		uint8_t b = 0xFF;
		uint8_t a = 0xFF;
	};

	void draw_text(const std::string& text, position position, color color);
	void frame(const std::function<void()>& callback);
}
