#pragma once

#include <chrono>

#include <game/structs.hpp>

struct client
{
	std::chrono::high_resolution_clock::time_point last_packet{};
	game::player_state current_state_{};
};
