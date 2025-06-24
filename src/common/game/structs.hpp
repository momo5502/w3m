#pragma once

#include <array>
#include <cstdint>

namespace game
{
    constexpr uint32_t PROTOCOL = 4;

    using vec3_t = std::array<double, 3>;
    using vec4_t = std::array<double, 4>;

    using name_t = std::array<char, 64>;
    using speed_t = std::array<float, 8>;

    struct player_state
    {
        vec3_t angles{};
        vec4_t position{};
        vec4_t velocity{};
        speed_t speed_values{};
        float speed; // legacy
        int32_t move_type{};
    };

    struct player
    {
        uint64_t guid{};
        name_t name{};
        player_state state{};
    };
}
