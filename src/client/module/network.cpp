#include "../std_include.hpp"

#include "../loader/component_loader.hpp"

#include <network/manager.hpp>

#include "network.hpp"

namespace network
{
    namespace
    {
        manager& get_network_manager()
        {
            static manager m{};
            return m;
        }
    }

    void on(const std::string& command, callback callback)
    {
        get_network_manager().on(command, std::move(callback));
    }

    bool send(const address& address, const std::string& command, const std::string& data, const char separator)
    {
        return get_network_manager().send(address, command, data, separator);
    }

    bool send_data(const address& address, const void* data, const size_t length)
    {
        return get_network_manager().send_data(address, data, length);
    }

    bool send_data(const address& address, const std::string& data)
    {
        return send_data(address, data.data(), data.size());
    }

    const address& get_master_server()
    {
        static const address master{"server.momo5502.com:28960"};
        return master;
    }

    struct component final : component_interface
    {
        void post_load() override
        {
            get_network_manager();
        }

        void pre_destroy() override
        {
            get_network_manager().stop();
        }
    };
}

REGISTER_COMPONENT(network::component)
