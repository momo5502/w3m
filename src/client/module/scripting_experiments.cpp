#include "../std_include.hpp"

#include "../loader/component_loader.hpp"

#include <game/structs.hpp>
#include <utils/byte_buffer.hpp>
#include <utils/concurrency.hpp>

#include "network.hpp"
#include "scripting.hpp"

namespace scripting_experiments
{
	namespace
	{
		struct W3mPlayerState
		{
			scripting::game::EulerAngles angles{};
			scripting::game::Vector position{};
			scripting::game::Vector velocity{};
			float speed;
			int move_type{};
		};

		using players = std::vector<W3mPlayerState>;
		utils::concurrency::container<players> g_players;

		game::vec3_t convert(const scripting::game::EulerAngles& euler_angles)
		{
			game::vec3_t angles{};
			angles[0] = euler_angles.Pitch;
			angles[1] = euler_angles.Yaw;
			angles[2] = euler_angles.Roll;

			return angles;
		}

		scripting::game::EulerAngles convert(const game::vec3_t& angles)
		{
			scripting::game::EulerAngles euler_angles{};
			euler_angles.Pitch = angles[0];
			euler_angles.Yaw = angles[1];
			euler_angles.Roll = angles[2];

			return euler_angles;
		}

		game::vec4_t convert(const scripting::game::Vector& game_vector)
		{
			game::vec4_t vector{};
			vector[0] = game_vector.X;
			vector[1] = game_vector.Y;
			vector[2] = game_vector.Z;
			vector[3] = game_vector.W;

			return vector;
		}

		scripting::game::Vector convert(const game::vec4_t& vector)
		{
			scripting::game::Vector game_vector{};
			game_vector.X = vector[0];
			game_vector.Y = vector[1];
			game_vector.Z = vector[2];
			game_vector.W = vector[3];

			return game_vector;
		}

		// ----------------------------------------------

		scripting::array<W3mPlayerState> get_player_states()
		{
			return g_players.access<scripting::array<W3mPlayerState>>([](const players& players)
			{
				return players;
			});
		}

		int get_player_count()
		{
			return g_players.access<int>([](const players& players)
			{
				return static_cast<int>(players.size());
			});
		}

		void store_player_state(const W3mPlayerState& state)
		{
			game::player_state player_state{};

			player_state.angles = convert(state.angles);
			player_state.position = convert(state.position);
			player_state.velocity = convert(state.velocity);
			player_state.speed = state.speed;
			player_state.move_type = state.move_type;

			utils::buffer_serializer buffer{};
			buffer.write(game::PROTOCOL);
			buffer.write(player_state);

			network::send(network::get_master_server(), "state", buffer.get_buffer());
		}

		void receive_player_states(const network::address& address, const std::string_view& data)
		{
			if (address != network::get_master_server())
			{
				return;
			}

			utils::buffer_deserializer buffer(data);
			const auto protocol = buffer.read<uint32_t>();
			if (protocol != game::PROTOCOL)
			{
				return;
			}

			const auto player_states = buffer.read_vector<game::player_state>();

			g_players.access([&player_states](players& players)
			{
				players.resize(0);
				players.reserve(player_states.size());

				for (const auto& state : player_states)
				{
					W3mPlayerState player_state{};
					player_state.angles = convert(state.angles);
					player_state.position = convert(state.position);
					player_state.velocity = convert(state.velocity);
					player_state.move_type = state.move_type;
					player_state.speed = state.speed;

					players.emplace_back(player_state);
				}
			});
		}
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			scripting::register_function<store_player_state>(L"StorePlayerState");
			scripting::register_function<get_player_count>(L"GetPlayerCount");
			scripting::register_function<get_player_states>(L"GetPlayerStates");

			network::on("states", &receive_player_states);
		}
	};
}

REGISTER_COMPONENT(scripting_experiments::component)
