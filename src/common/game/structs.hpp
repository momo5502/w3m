#pragma once

#include <array>
#include <cstdint>

namespace game
{
	constexpr uint32_t PROTOCOL = 2;

	using vec3_t = std::array<float, 3>;
	using vec4_t = std::array<float, 4>;

	using name_t = std::array<char, 64>;

	struct player_state
	{
		vec3_t angles{};
		vec4_t position{};
		vec4_t velocity{};
		float speed;
		int32_t move_type{};
	};

	struct player
	{
		uint64_t guid{};
		name_t name{};
		player_state state{};
	};
}
