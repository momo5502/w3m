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

		W3mPlayerInfo get_player_info(int /*player_id*/)
		{
			W3mPlayerInfo info{};
			return info;
		}

		W3mPlayerState get_player_state(int /*player_id*/)
		{
			return {};
		}

		int get_player_state_id()
		{
			return {};
		}

		int get_player_count()
		{
			return 1;
		}

		void store_player_state(const W3mPlayerState& state)
		{
			(void)state;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
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
