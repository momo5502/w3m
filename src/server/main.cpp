#include "std_include.hpp"

#include <network/manager.hpp>

#include "console.hpp"

int main(int argc, const char* argv[])
{
	console::set_title("W3M Server");
	console::log("Starting W3M Server");

	const network::manager manager{28960};

	console::log("Running on %hu (v4) and %hu (v6)", manager.get_ipv4_socket().get_port(),
	             manager.get_ipv6_socket().get_port());

	std::atomic_bool stop{false};
	console::signal_handler handler([&stop]
	{
		stop = true;
	});

	while (!stop)
	{
		std::this_thread::sleep_for(10ms);
	}

	console::log("Terminating server...");

	return 0;
}
