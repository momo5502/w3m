#include "../std_include.hpp"

#include "../loader/component_loader.hpp"

#include <network/socket.hpp>

#include <utils/thread.hpp>
#include <utils/string.hpp>
#include <utils/concurrency.hpp>

#include "network.hpp"

namespace network
{
	namespace
	{
		socket g_socket_v4{};
		socket g_socket_v6{};
		std::jthread g_thread{};

		using callback_map = std::unordered_map<std::string, callback>;
		utils::concurrency::container<callback_map> g_callbacks{};

		bool try_bind_socket(socket& s, const uint16_t port)
		{
			const auto family = s.get_address_family();

			address a{};

			if (family == AF_INET)
			{
				a.set_ipv6(in6addr_any);
			}
			else if (family == AF_INET6)
			{
				a.set_ipv4(in4addr_any);
			}

			a.set_port(port);

			return s.bind_port(a);
		}

		void bind_socket(socket& s, const uint16_t port)
		{
			for (uint16_t i = 0; i < 100; ++i)
			{
				if (try_bind_socket(s, port + i))
				{
					return;
				}
			}

			throw std::runtime_error("Failed to bind socket");
		}

		void dispatch_command(const address& source, const std::string_view& command, const std::string_view& data)
		{
			const auto lower_command = utils::string::to_lower(std::string{command.begin(), command.end()});

			g_callbacks.access([&](const callback_map& callbacks)
			{
				const auto callback = callbacks.find(lower_command);
				if (callback == callbacks.end())
				{
					return;
				}

				try
				{
					callback->second(source, data);
				}
				catch (const std::exception& e)
				{
					printf("Error while handling packet of type %s: %s\n", lower_command.data(), e.what());
					(void)fflush(stdout);
				}
			});
		}

		void handle_data(const address& source, const std::string& packet)
		{
			constexpr int32_t magic = -1;
			constexpr auto magic_size = sizeof(magic);

			if (packet.size() < (magic_size + 1) || memcmp(packet.data(), &magic, magic_size) != 0)
			{
				return;
			}

			const std::string_view buffer(packet.data() + magic_size, packet.size() - magic_size);
			const auto divider = buffer.find_first_of(" \n");
			if (divider == std::string_view::npos)
			{
				return;
			}

			const std::string_view command(buffer.data(), divider);
			if (command.empty())
			{
				return;
			}

			const auto data_start = divider + 1;
			const std::string_view data(buffer.data() + data_start, buffer.size() - data_start);

			dispatch_command(source, command, data);
		}

		bool receive_socket_data(const socket& s)
		{
			address source{};
			std::string data{};

			if (!s.receive(source, data))
			{
				return false;
			}

			handle_data(source, data);
			return true;
		}

		void packet_receiver(const std::stop_token& stop_token)
		{
			while (!stop_token.stop_requested())
			{
				const auto v4_handled = receive_socket_data(g_socket_v4);
				const auto v6_handled = receive_socket_data(g_socket_v6);

				if (!v4_handled && !v6_handled)
				{
					socket::sleep_sockets({&g_socket_v4, &g_socket_v6}, 1s);
				}
			}
		}
	}

	void on(const std::string& command, callback callback)
	{
		g_callbacks.access([&](callback_map& callbacks)
		{
			callbacks[utils::string::to_lower(command)] = std::move(callback);
		});
	}

	bool send(const address& address, const std::string& command, const std::string& data, const char separator)
	{
		std::string packet = "\xFF\xFF\xFF\xFF";
		packet.append(command);
		packet.push_back(separator);
		packet.append(data);

		return send_data(address, packet);
	}

	bool send_data(const address& address, const void* data, const size_t length)
	{
		if (address.is_ipv4())
		{
			return g_socket_v4.send(address, data, length);
		}

		if (address.is_ipv6())
		{
			return g_socket_v6.send(address, data, length);
		}

		return false;
	}

	bool send_data(const address& address, const std::string& data)
	{
		return send_data(address, data.data(), data.size());
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			g_socket_v4 = socket{AF_INET};
			g_socket_v6 = socket{AF_INET6};

			g_socket_v4.set_blocking(false);
			g_socket_v6.set_blocking(false);

			bind_socket(g_socket_v4, 28960);
			bind_socket(g_socket_v6, 28960);

			g_thread = utils::thread::create_named_jthread("Network Dispatcher", packet_receiver);
		}

		void pre_destroy() override
		{
			g_thread = {};
			g_socket_v4 = {};
			g_socket_v6 = {};
		}
	};
}

REGISTER_COMPONENT(network::component)
