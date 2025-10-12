#include "manager.hpp"

#include "socket.hpp"

#include "../utils/thread.hpp"
#include "../utils/string.hpp"

namespace network
{
    namespace
    {
        bool try_bind_socket(socket& s, const uint16_t port)
        {
            const auto family = s.get_address_family();

            address a{};

            if (family == AF_INET)
            {
                a.set_ipv4(INADDR_ANY);
            }
            else if (family == AF_INET6)
            {
                a.set_ipv6(in6addr_any);
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

        void dispatch_command(const utils::concurrency::container<manager::callback_map>& callbacks, const address& source,
                              const std::string_view& command, const std::string_view& data)
        {
            const auto lower_command = utils::string::to_lower(std::string{command.begin(), command.end()});

            callbacks.access([&](const manager::callback_map& map) {
                const auto callback = map.find(lower_command);
                if (callback == map.end())
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

        void handle_data(const utils::concurrency::container<manager::callback_map>& callbacks, const address& source,
                         const std::string& packet)
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

            dispatch_command(callbacks, source, command, data);
        }

        bool receive_socket_data(const utils::concurrency::container<manager::callback_map>& callbacks, const socket& s)
        {
            address source{};
            std::string data{};

            if (!s.receive(source, data))
            {
                return false;
            }

            handle_data(callbacks, source, data);
            return true;
        }

        socket create_and_bind_socket(const int family, const std::optional<uint16_t>& port)
        {
            socket s{family};
            s.set_blocking(false);

            if (port)
            {
                bind_socket(s, *port);
            }

            return s;
        }
    }

    manager::manager(const std::optional<uint16_t>& port)
        : socket_v4_(create_and_bind_socket(AF_INET, port)),
          socket_v6_(create_and_bind_socket(AF_INET6, port))
    {
        thread_ = utils::thread::create_named_jthread("Network Dispatcher",
                                                      [this](const std::stop_token& stop_token) { this->packet_receiver(stop_token); });
    }

    void manager::packet_receiver(const std::stop_token& stop_token)
    {
        while (!stop_token.stop_requested())
        {
            const auto v4_handled = receive_socket_data(this->callbacks_, this->socket_v4_);
            const auto v6_handled = receive_socket_data(this->callbacks_, this->socket_v6_);

            if (!v4_handled && !v6_handled)
            {
                std::vector<const socket*> sockets{&this->socket_v4_, &this->socket_v6_};

                socket::sleep_sockets(sockets, std::chrono::seconds{1});
            }
        }
    }

    void manager::on(const std::string& command, callback callback)
    {
        this->callbacks_.access([&](callback_map& callbacks) { callbacks[utils::string::to_lower(command)] = std::move(callback); });
    }

    bool manager::send(const address& address, const std::string& command, const std::string& data, const char separator) const
    {
        std::string packet = "\xFF\xFF\xFF\xFF";
        packet.append(command);
        packet.push_back(separator);
        packet.append(data);

        return this->send_data(address, packet);
    }

    bool manager::send_data(const address& address, const void* data, const size_t length) const
    {
        if (address.is_ipv4())
        {
            return this->socket_v4_.send(address, data, length);
        }

        if (address.is_ipv6())
        {
            return this->socket_v6_.send(address, data, length);
        }

        return false;
    }

    bool manager::send_data(const address& address, const std::string& data) const
    {
        return this->send_data(address, data.data(), data.size());
    }

    void manager::stop()
    {
        this->thread_ = {};
    }

    const socket& manager::get_ipv4_socket() const
    {
        return this->socket_v4_;
    }

    const socket& manager::get_ipv6_socket() const
    {
        return this->socket_v6_;
    }
}
