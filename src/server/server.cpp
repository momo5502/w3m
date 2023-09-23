#include "std_include.hpp"
#include "server.hpp"

#include <utils/byte_buffer.hpp>

namespace
{
	void handle_player_state(server::client_map& clients, const network::address& source, const std::string_view& data)
	{
		utils::buffer_deserializer buffer(data);
		const auto player_state = buffer.read<game::player_state>();

		auto& client = clients[source];
		client.last_packet = std::chrono::high_resolution_clock::now();
		client.current_state_ = player_state;
	}
}

server::server(const uint16_t port)
	: manager_(port)
{
	this->on("state", &handle_player_state);
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
		std::this_thread::sleep_for(1s);
	}
}

void server::stop()
{
	this->stop_ = true;
}

void server::run_frame()
{
	this->clients_.access([](client_map& clients)
	{
		const auto now = std::chrono::high_resolution_clock::now();

		for (auto i = clients.begin(); i != clients.end();)
		{
			const auto last_packet_diff = now - i->second.last_packet;

			if (last_packet_diff > 1min)
			{
				i = clients.erase(i);
			}
			else
			{
				++i;
			}
		}
	});
}

void server::on(const std::string& command, callback callback)
{
	this->on(command, [c = std::move(callback)](const network::manager&, client_map& clients,
	                                            const network::address& source,
	                                            const std::string_view& data)
	{
		c(clients, source, data);
	});
}

void server::on(const std::string& command, reply_callback callback)
{
	this->manager_.on(
		command, [this, c = std::move(callback)](const network::address& source, const std::string_view& data)
		{
			this->clients_.access([&](client_map& clients)
			{
				c(this->manager_, clients, source, data);
			});
		});
}
