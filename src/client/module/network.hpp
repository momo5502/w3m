#pragma once

#include <network/address.hpp>

namespace network
{
    using callback = std::function<void(const address&, const std::string_view&)>;

    void on(const std::string& command, callback callback);
    bool send(const address& address, const std::string& command, const std::string& data = {}, char separator = ' ');

    bool send_data(const address& address, const void* data, size_t length);
    bool send_data(const address& address, const std::string& data);

    const address& get_master_server();
}
