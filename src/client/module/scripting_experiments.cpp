#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <network/socket.hpp>

#include <utils/string.hpp>

#include "scripting.hpp"

namespace scripting_experiments
{
	namespace
	{
		struct W3mPlayerInfo
		{
			scripting::detail::managed_script_string name{};
			uint64_t guid{};
		};

		struct W3mPlayerState
		{
			scripting::game::EulerAngles angles{};
			scripting::game::Vector position{};
			scripting::game::Vector velocity{};
			float speed;
			int move_type{};
		};

		// ----------------------------------------------

		std::mutex m{};
		network::address other_addr{};
		int state_id{0};
		W3mPlayerState player_state{};
		std::optional<network::socket> sock{};

		W3mPlayerInfo get_player_info(int /*player_id*/)
		{
			W3mPlayerInfo info{};
			return info;
		}

		W3mPlayerState get_player_state(int /*player_id*/)
		{
			std::lock_guard _{m};
			return player_state;
		}

		int get_player_state_id()
		{
			std::lock_guard _{m};
			return state_id;
		}

		int get_player_count()
		{
			return 1;
		}

		void store_player_state(const W3mPlayerState& state)
		{
			if (sock)
			{
				sock->send(other_addr, std::string(reinterpret_cast<const char*>(&state), sizeof(state)));
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			auto& s = sock.emplace();
			s.set_blocking(false);

			network::address myaddr{"0.0.0.0"};
			myaddr.set_port(28960);
			s.bind_port(myaddr);

			/*if(!s.bind_port(myaddr))
			{
				other_addr = myaddr;
				myaddr.set_port(28961);
				s.bind_port(myaddr);
			}
			else
			{
				myaddr.set_port(28961);
				other_addr = myaddr;
			}*/
			wchar_t name[MAX_PATH]{ 0 };
			GetHostNameW(name, sizeof(name));

			network::address oa{name == std::wstring(L"Maurice-Laptop") ? "192.168.178.50" : "192.168.178.50", AF_INET};
			oa.set_port(28960);
			other_addr = oa;

			std::thread([&s]
			{
				while (true)
				{
					std::string data{};
					network::address target{};

					while (s.receive(target, data))
					{
						if (data.size() == sizeof(W3mPlayerState))
						{
							std::lock_guard _{m};
							memcpy(&player_state, data.data(), sizeof(player_state));
							++state_id;
						}
					}

					(void)s.sleep(1s);
				}
			}).detach();
		}

		void post_load() override
		{
			scripting::register_function<store_player_state>(L"StorePlayerState");
			scripting::register_function<get_player_count>(L"GetPlayerCount");
			scripting::register_function<get_player_info>(L"GetPlayerInfo");
			scripting::register_function<get_player_state>(L"GetPlayerState");
			scripting::register_function<get_player_state_id>(L"GetPlayerStateId");
		}
	};
}

REGISTER_COMPONENT(scripting_experiments::component)
