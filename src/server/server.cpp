#include "std_include.hpp"
#include "server.hpp"

server::server(const uint16_t port)
	: manager_(port)
{
}

uint16_t server::get_ipv4_port() const
{
	return this->manager_.get_ipv4_socket().get_port();
}

uint16_t server::get_ipv6_port() const
{
	return this->manager_.get_ipv6_socket().get_port();
}

void server::run()
{
	this->stop_ = false;

	while (!this->stop_)
	{
		this->run_frame();
		std::this_thread::sleep_for(10ms);
	}
}

void server::stop()
{
	this->stop_ = true;
}

void server::run_frame()
{
}
