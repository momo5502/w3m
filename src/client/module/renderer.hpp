#pragma once

namespace renderer
{
    struct position
    {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct color
    {
        uint8_t r = 0xFF;
        uint8_t g = 0xFF;
        uint8_t b = 0xFF;
        uint8_t a = 0xFF;
    };

    void draw_text(std::string text, position position, color color);
}
