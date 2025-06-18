#include "std_include.hpp"

#include "server.hpp"
#include "console.hpp"

extern "C"
{
    int s_read_arc4random(void*, size_t)
    {
        return -1;
    }

    int s_read_getrandom(void*, size_t)
    {
        return -1;
    }

    int s_read_urandom(void*, size_t)
    {
        return -1;
    }

    int s_read_ltm_rng(void*, size_t)
    {
        return -1;
    }
}

namespace
{
    void run()
    {
        console::set_title("W3M Server");
        console::log("Starting W3M Server");

        server s{28960};

        console::log("Running on %hu (v4) and %hu (v6)", s.get_ipv4_port(), s.get_ipv6_port());

        console::signal_handler handler([&s] { s.stop(); });

        s.run();

        console::log("Terminating server...");
    }
}

int main(int /*argc*/, const char* /*argv*/[])
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        console::error("%s", e.what());
        return 1;
    }

    return 0;
}
