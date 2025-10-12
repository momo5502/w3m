#pragma once

#include <unordered_map>

#include <network/manager.hpp>
#include <utils/concurrency.hpp>

#include "client.hpp"

class server
{
  public:
    using client_map = std::unordered_map<network::address, client>;

    server(uint16_t port);

    uint16_t get_ipv4_port() const;
    uint16_t get_ipv6_port() const;

    void run();
    void stop();

  private:
    utils::concurrency::container<client_map> clients_{};

    std::atomic_bool stop_{false};
    network::manager manager_;

    void run_frame();

    using callback = std::function<void(client_map&, const network::address&, const std::string_view&)>;
    void on(const std::string& command, callback callback);

    using reply_callback = std::function<void(const network::manager&, client_map&, const network::address&, const std::string_view&)>;
    void on(const std::string& command, reply_callback callback);
};
