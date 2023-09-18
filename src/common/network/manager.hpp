#pragma once

#include <thread>
#include <functional>

#include "address.hpp"
#include "socket.hpp"

#include "../utils/concurrency.hpp"

namespace network
{
	class manager
	{
	public:
		manager(uint16_t port);

		using callback = std::function<void(const address&, const std::string_view&)>;
		using callback_map = std::unordered_map<std::string, callback>;

		void on(const std::string& command, callback callback);
		bool send(const address& address, const std::string& command, const std::string& data = {},
		          char separator = ' ') const;

		bool send_data(const address& address, const void* data, size_t length) const;
		bool send_data(const address& address, const std::string& data) const;

		void stop();

	private:
		socket socket_v4_{};
		socket socket_v6_{};

		utils::concurrency::container<callback_map> callbacks_{};

		std::jthread thread_{};

		void packet_receiver(const std::stop_token& stop_token);
	};
}
