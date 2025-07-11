#include "std_include.hpp"
#include "server.hpp"

#include <utils/string.hpp>
#include <utils/byte_buffer.hpp>

#include "console.hpp"

namespace
{
    void send_authentication_request(const network::manager& manager, const network::address& source, client& client)
    {
        if (client.authentication_nonce.empty())
        {
            console::log("Authenticating player: %s (%llX)", source.to_string().data(), client.guid);
            client.authentication_nonce = utils::cryptography::random::get_challenge();
        }

        utils::buffer_serializer buffer{};
        buffer.write(game::PROTOCOL);
        buffer.write_string(client.authentication_nonce);

        (void)manager.send(source, "authRequest", buffer.get_buffer());
    }

    void send_killed_command(const network::manager& manager, const network::address& victim, const client& killer)
    {
        utils::buffer_serializer buffer{};
        buffer.write(game::PROTOCOL);
        buffer.write(killer.guid);

        (void)manager.send(victim, "killed", buffer.get_buffer());
    }

    void handle_authentication_response(server::client_map& clients, const network::address& source,
                                        const std::string_view& data)
    {
        utils::buffer_deserializer buffer(data);
        const auto protocol = buffer.read<uint32_t>();
        if (protocol != game::PROTOCOL)
        {
            return;
        }

        const auto key = buffer.read_string();
        const auto signature = buffer.read_string();

        utils::cryptography::ecc::key crypto_key{};
        crypto_key.deserialize(key);
        if (!crypto_key.is_valid())
        {
            return;
        }

        auto& client = clients[source];

        const auto print_failure = [&](const char* reason) {
            if (!client.has_printed_failure)
            {
                client.has_printed_failure = true;
                console::log("Authentication failed (%s): %s", source.to_string().data(), reason); //
            }
        };

        if (client.authentication_nonce.empty())
        {
            print_failure("Nonce not set");
            return;
        }

        if (crypto_key.get_hash() != client.guid)
        {
            print_failure("Key doesn't match GUID");
            return;
        }

        if (!verify_message(crypto_key, client.authentication_nonce, signature))
        {
            print_failure("Invalid signature");
            return;
        }

        client.public_key = std::move(crypto_key);
        client.last_packet = std::chrono::high_resolution_clock::now();

        console::log("Player authenticated: %s (%llX)", source.to_string().data(), client.guid);
    }

    void handle_player_kill(const network::manager& manager, server::client_map& clients,
                            const network::address& source, const std::string_view& data)
    {
        utils::buffer_deserializer buffer(data);
        const auto protocol = buffer.read<uint32_t>();
        if (protocol != game::PROTOCOL)
        {
            return;
        }

        const auto& killer = clients[source];
        if (!killer.is_authenticated())
        {
            return;
        }

        const auto player_guid = buffer.read<uint64_t>();

        for (auto& client : clients)
        {
            if (client.second.is_authenticated() && client.second.guid == player_guid)
            {
                send_killed_command(manager, client.first, killer);
                break;
            }
        }
    }

    void handle_player_state(const network::manager& manager, server::client_map& clients,
                             const network::address& source, const std::string_view& data)
    {
        utils::buffer_deserializer buffer(data);
        const auto protocol = buffer.read<uint32_t>();
        if (protocol != game::PROTOCOL)
        {
            return;
        }

        const auto player_state = buffer.read<game::player>();

        auto& client = clients[source];
        client.last_packet = std::chrono::high_resolution_clock::now();
        client.guid = player_state.guid;
        client.name.assign(player_state.name.data(), strnlen(player_state.name.data(), player_state.name.size()));
        client.current_state = std::move(player_state.state);
        client.state_id += 1;

        if (!client.is_authenticated())
        {
            send_authentication_request(manager, source, client);
        }
    }

    void send_state(const network::manager& manager, const server::client_map& clients)
    {
        std::vector<game::player> states{};
        states.reserve(clients.size());

        for (const auto& val : clients | std::views::values)
        {
            if (!val.is_authenticated())
            {
                continue;
            }

            game::player player{};
            player.guid = val.guid;
            player.state = val.current_state;
            player.state.state_id = val.state_id;
            utils::string::copy(player.name, val.name.data());

            states.emplace_back(std::move(player));
        }

        utils::buffer_serializer buffer{};
        buffer.write(game::PROTOCOL);
        buffer.write_vector(states);

        for (const auto& client : clients)
        {
            if (client.second.is_authenticated())
            {
                (void)manager.send(client.first, "states", buffer.get_buffer());
            }
        }
    }
}

server::server(const uint16_t port)
    : manager_(port)
{
    this->on("state", &handle_player_state);
    this->on("kill", &handle_player_kill);
    this->on("authResponse", &handle_authentication_response);
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
        std::this_thread::sleep_for(30ms);
    }
}

void server::stop()
{
    this->stop_ = true;
}

void server::run_frame()
{
    this->clients_.access([this](client_map& clients) {
        const auto now = std::chrono::high_resolution_clock::now();

        for (auto i = clients.begin(); i != clients.end();)
        {
            const auto last_packet_diff = now - i->second.last_packet;

            if (last_packet_diff > 20s)
            {
                console::log("Removing player: %s", i->first.to_string().data());
                i = clients.erase(i);
            }
            else
            {
                ++i;
            }
        }

        send_state(this->manager_, clients);
    });
}

void server::on(const std::string& command, callback callback)
{
    this->on(command,
             [c = std::move(callback)](const network::manager&, client_map& clients, const network::address& source,
                                       const std::string_view& data) { c(clients, source, data); });
}

void server::on(const std::string& command, reply_callback callback)
{
    this->manager_.on(command,
                      [this, c = std::move(callback)](const network::address& source, const std::string_view& data) {
                          this->clients_.access([&](client_map& clients) { c(this->manager_, clients, source, data); });
                      });
}
