#pragma once

#include <network/manager.hpp>

class server
{
public:
	server(uint16_t port);

	uint16_t get_ipv4_port() const;
	uint16_t get_ipv6_port() const;

	void run();
	void stop();

private:
	std::atomic_bool stop_{false};
	network::manager manager_;

	void run_frame();
};
