#pragma once

#include <chrono>
#include <network/address.hpp>

struct client
{
	uint64_t id{};
	std::string name{};
	network::address address{};
	std::chrono::high_resolution_clock::time_point last_packet{};
};
