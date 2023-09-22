#pragma once

#include <unordered_map>

#include <network/manager.hpp>

#include "client.hpp"

class server
{
public:
	server(uint16_t port);

	uint16_t get_ipv4_port() const;
	uint16_t get_ipv6_port() const;

	void run();
	void stop();

private:
	using client_map = std::unordered_map<network::address, client>;

	std::atomic_bool stop_{false};
	network::manager manager_;

	void run_frame();
};
