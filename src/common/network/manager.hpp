#pragma once

#include <thread>
#include <optional>
#include <functional>

#include "address.hpp"
#include "socket.hpp"

#include "../utils/concurrency.hpp"

namespace network
{
    class manager
    {
      public:
        manager(const std::optional<uint16_t>& port = {});

        manager(manager&&) = delete;
        manager(const manager&) = delete;
        manager& operator=(manager&&) = delete;
        manager& operator=(const manager&) = delete;

        using callback = std::function<void(const address&, const std::string_view&)>;
        using callback_map = std::unordered_map<std::string, callback>;

        void on(const std::string& command, callback callback);
        bool send(const address& address, const std::string& command, const std::string& data = {}, char separator = ' ') const;

        bool send_data(const address& address, const void* data, size_t length) const;
        bool send_data(const address& address, const std::string& data) const;

        void stop();

        const socket& get_ipv4_socket() const;
        const socket& get_ipv6_socket() const;

      private:
        socket socket_v4_{};
        socket socket_v6_{};

        utils::concurrency::container<callback_map> callbacks_{};

        std::jthread thread_{};

        void packet_receiver(const std::stop_token& stop_token);
    };
}
