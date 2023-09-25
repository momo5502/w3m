#pragma once

#include <array>
#include <cstdint>

namespace game
{
	constexpr uint32_t PROTOCOL = 1;

	using vec3_t = std::array<float, 3>;
	using vec4_t = std::array<float, 4>;

	struct player_state
	{
		vec3_t angles{};
		vec4_t position{};
		vec4_t velocity{};
		float speed;
		int32_t move_type{};
	};
}
