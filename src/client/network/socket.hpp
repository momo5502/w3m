#pragma once

#include "address.hpp"

#include <chrono>

#ifdef _WIN32
using socklen_t = int;
#define GET_SOCKET_ERROR() (WSAGetLastError())
#else
		using SOCKET = int;
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define GET_SOCKET_ERROR() (errno)
#endif

namespace network
{
	class socket
	{
	public:
		socket(int af = AF_INET);
		~socket();

		socket(const socket& obj) = delete;
		socket& operator=(const socket& obj) = delete;

		socket(socket&& obj) noexcept;
		socket& operator=(socket&& obj) noexcept;

		bool bind_port(const address& target);

		[[maybe_unused]] bool send(const address& target, const std::string& data) const;
		bool receive(address& source, std::string& data) const;

		bool set_blocking(bool blocking);

		static constexpr bool socket_is_ready = true;
		bool sleep(std::chrono::milliseconds timeout) const;
		bool sleep_until(std::chrono::high_resolution_clock::time_point time_point) const;

		SOCKET get_socket() const;
		uint16_t get_port() const;

		static bool sleep_sockets(const std::vector<const socket*>& sockets, std::chrono::milliseconds timeout);
		static bool sleep_sockets_until(const std::vector<const socket*>& sockets,
		                                std::chrono::high_resolution_clock::time_point time_point);

	private:
		uint16_t port_ = 0;
		SOCKET socket_ = INVALID_SOCKET;
	};
}
