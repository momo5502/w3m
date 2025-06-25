#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <game/structs.hpp>

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/byte_buffer.hpp>
#include <utils/concurrency.hpp>

#include "../utils/identity.hpp"

#include "network.hpp"
#include "renderer.hpp"
#include "scheduler.hpp"
#include "scripting.hpp"
#include "properties.hpp"
#include "steam_proxy.hpp"

namespace scripting_experiments
{
    namespace
    {

        template <typename T>
        struct game_object
        {
            uint64_t some_type{};
            T* object{};
        };

        template <typename T, size_t Alignment = 4>
        class game_struct
        {
          public:
            template <typename... Args>
            explicit game_struct(Args&&... args)
            {
                new (&storage_) T(std::forward<Args>(args)...);
            }

            game_struct(const game_struct& other)
            {
                new (&storage_) T(other.get());
            }

            game_struct(game_struct&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
            {
                new (&storage_) T(std::move(other.get()));
            }

            game_struct& operator=(const game_struct& other)
            {
                if (this != &other)
                {
                    get() = other.get();
                }

                return *this;
            }

            game_struct& operator=(game_struct&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
            {
                if (this != &other)
                {
                    get() = std::move(other.get());
                }
                return *this;
            }

            ~game_struct()
            {
                get().~T();
            }

            T& operator*()
            {
                return get();
            }
            const T& operator*() const
            {
                return get();
            }

            T* operator->()
            {
                return &get();
            }
            const T* operator->() const
            {
                return &get();
            }

          private:
            alignas(Alignment) std::byte storage_[sizeof(T)];

            T& get()
            {
                return *std::launder(reinterpret_cast<T*>(&storage_));
            }

            const T& get() const
            {
                return *std::launder(reinterpret_cast<const T*>(&storage_));
            }
        };

        // -------------------------------------------

        struct W3mPlayerState
        {
            scripting::game::EulerAngles angles{};
            scripting::game::Vector position{};
            scripting::game::Vector velocity{};
            float speed;
            int move_type{};
        };

        struct W3mPlayer
        {
            uint64_t guid{};
            scripting::string name{};
            scripting::array<W3mPlayerState> state{};
        };

        struct CNewNPC
        {
            char pad[704];
            scripting::string display_name;
        };

        struct CMovingAgentComponent
        {
            char pad[0xF78];
            float m_relativeMoveSpeed;
            char pad2[0x11C];
            int32_t m_moveType;
            char pad3[0xD0];
            float m_desiredAbsoluteSpeed;
            float m_gameplayRelativeMoveSpeed;
            float m_gameplayMoveDirection;
            float m_acceleration;
            float m_deceleration;
            float m_currentSpeedVal;
            float m_lastRelMovementSpeed;

            float ConvertSpeedAbsToRel(const float speedAbs) const
            {
                auto* func = reinterpret_cast<float(__thiscall*)(const CMovingAgentComponent*, float)>(0x14151DB00_g);
                return func(this, speedAbs);
            }
        };

        static_assert(offsetof(CMovingAgentComponent, m_relativeMoveSpeed) == 0xF78);
        static_assert(offsetof(CMovingAgentComponent, m_moveType) == 0x1098);
        static_assert(offsetof(CMovingAgentComponent, m_desiredAbsoluteSpeed) == 0x116C);
        static_assert(offsetof(CMovingAgentComponent, m_gameplayRelativeMoveSpeed) == 0x1170);
        static_assert(offsetof(CMovingAgentComponent, m_acceleration) == 0x1178);
        static_assert(offsetof(CMovingAgentComponent, m_deceleration) == 0x117C);
        static_assert(offsetof(CMovingAgentComponent, m_currentSpeedVal) == 0x1180);
        static_assert(offsetof(CMovingAgentComponent, m_lastRelMovementSpeed) == 0x1184);

        struct players
        {
            std::vector<game::player> infos;
            std::map<uint64_t, uint64_t> state_ids{};
        };

        utils::concurrency::container<players> g_players;

        game::vec3_t convert(const scripting::game::EulerAngles& euler_angles)
        {
            game::vec3_t angles{};
            angles[0] = euler_angles.Roll;
            angles[1] = euler_angles.Pitch;
            angles[2] = euler_angles.Yaw;

            return angles;
        }

        scripting::game::EulerAngles convert(const game::vec3_t& angles)
        {
            scripting::game::EulerAngles euler_angles{};
            euler_angles.Roll = static_cast<float>(angles[0]);
            euler_angles.Pitch = static_cast<float>(angles[1]);
            euler_angles.Yaw = static_cast<float>(angles[2]);

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
            game_vector.X = static_cast<float>(vector[0]);
            game_vector.Y = static_cast<float>(vector[1]);
            game_vector.Z = static_cast<float>(vector[2]);
            game_vector.W = static_cast<float>(vector[3]);

            return game_vector;
        }

        template <size_t Size>
        scripting::string convert(const std::array<char, Size>& str)
        {
            const auto length = strnlen(str.data(), Size);
            return {str.data(), length};
        }

        W3mPlayer convert(const game::player& player, const bool attach_state)
        {
            W3mPlayer w3m_player{};
            w3m_player.guid = player.guid;
            w3m_player.name = convert(player.name);

            if (attach_state)
            {
                W3mPlayerState player_state{};
                player_state.angles = convert(player.state.angles);
                player_state.position = convert(player.state.position);
                player_state.velocity = convert(player.state.velocity);
                player_state.move_type = player.state.move_type;
                player_state.speed = player.state.speed;

                w3m_player.state.push_back(std::move(player_state));
            }

            return w3m_player;
        }

        int32_t get_move_type(const game_object<CMovingAgentComponent>* moving_agent)
        {
            if (!moving_agent || !moving_agent->object)
            {
                return 0;
            }

            return moving_agent->object->m_moveType;
        }

        void set_speed(const game_object<CMovingAgentComponent>* moving_agent, const float abs_speed)
        {
            if (!moving_agent || !moving_agent->object)
            {
                return;
            }

            auto& agent = *moving_agent->object;

            const auto rel_speed = agent.ConvertSpeedAbsToRel(abs_speed);

            agent.m_desiredAbsoluteSpeed = abs_speed;
            agent.m_currentSpeedVal = abs_speed;
            agent.m_relativeMoveSpeed = rel_speed;
            agent.m_lastRelMovementSpeed = rel_speed;
            agent.m_gameplayRelativeMoveSpeed = rel_speed;
        }

        void debug_print(const scripting::string& str)
        {
            puts(str.to_string().c_str());
        }

        void set_display_name(const game_object<CNewNPC>* npc, const scripting::string& name)
        {
            if (npc && npc->object)
            {
                npc->object->display_name = name;
            }
        }

        scripting::array<W3mPlayer> get_player_states()
        {
            return g_players.access<scripting::array<W3mPlayer>>([](players& players) {
                scripting::array<W3mPlayer> w3m_players{};

                if (players.infos.empty())
                {
                    return w3m_players;
                }

                w3m_players.resize(players.infos.size());

                for (size_t i = 0; i < players.infos.size(); ++i)
                {
                    const auto& info = players.infos[i];

                    auto& last_state = players.state_ids[info.guid];
                    const auto has_new_state = info.state.state_id > last_state;
                    last_state = info.state.state_id;

                    w3m_players[i] = convert(info, has_new_state);
                }

                return w3m_players;
            });
        }

        std::string get_default_player_name()
        {
            const auto steam_name = steam_proxy::get_player_name();
            if (steam_name)
            {
                return steam_name;
            }

            return utils::nt::get_user_name();
        }

        void set_stored_player_name(const std::string& name)
        {
            properties::set("player_name", name);
        }

        std::optional<std::string> get_stored_player_name()
        {
            std::string name{};
            if (properties::get("player_name", name))
            {
                return name;
            }

            return std::nullopt;
        }

        std::string load_player_name()
        {
            auto stored_name = get_stored_player_name();
            if (!stored_name)
            {
                stored_name = get_default_player_name();
                set_stored_player_name(*stored_name);
            }

            return std::move(*stored_name);
        }

        std::string& get_player_name()
        {
            static std::string name = load_player_name();
            return name;
        }

        void update_player_name(const scripting::string& wide_name)
        {
            auto name = wide_name.to_string();
            set_stored_player_name(name);
            get_player_name() = std::move(name);
        }

        void store_player_state(const W3mPlayerState& state)
        {
            game::player_state player_state{};

            player_state.angles = convert(state.angles);
            player_state.position = convert(state.position);
            player_state.velocity = convert(state.velocity);
            player_state.speed = state.speed;
            player_state.move_type = state.move_type;

            const auto& username = get_player_name();

            game::player player{};
            player.guid = utils::identity::get_guid();
            player.state = std::move(player_state);
            utils::string::copy(player.name, username.data());

            utils::buffer_serializer buffer{};
            buffer.write(game::PROTOCOL);
            buffer.write(player);

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

            const auto own_guid = utils::identity::get_guid();
            const auto player_data = buffer.read_vector<game::player>();

            g_players.access([&player_data, own_guid](players& players) {
                players.infos.resize(0);
                players.infos.reserve(player_data.size());

                std::set<uint64_t> guids{};

                for (const auto& player : player_data)
                {
                    if (player.guid == own_guid)
                    {
                        continue;
                    }

                    guids.insert(player.guid);
                    players.infos.emplace_back(player);
                }

                for (auto i = players.state_ids.begin(); i != players.state_ids.end();)
                {
                    if (guids.contains(i->first))
                    {
                        ++i;
                        continue;
                    }

                    i = players.state_ids.erase(i);
                }
            });
        }

        void receive_auth_request(const network::address& address, const std::string_view& data)
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

            const auto nonce = buffer.read_string();

            const auto& key = utils::identity::get_key();
            const auto public_key = key.serialize(PK_PUBLIC);
            const auto signature = sign_message(key, nonce);

            utils::buffer_serializer response{};
            response.write(game::PROTOCOL);
            response.write_string(public_key);
            response.write_string(signature);

            network::send(address, "authResponse", response.get_buffer());
        }

        size_t get_player_count()
        {
            return g_players.access<size_t>([](const players& players) {
                return players.infos.size(); //
            });
        }
    }

    struct component final : component_interface
    {
        void post_load() override
        {
            scripting::register_function<store_player_state>(L"W3mStorePlayerState");
            scripting::register_function<get_player_states>(L"W3mGetPlayerStates");
            scripting::register_function<set_display_name>(L"W3mSetNpcDisplayName");
            scripting::register_function<update_player_name>(L"W3mUpdatePlayerName");
            scripting::register_function<get_move_type>(L"W3mGetMoveType");
            scripting::register_function<set_speed>(L"W3mSetSpeed");
            scripting::register_function<debug_print>(L"W3mPrint");

            network::on("states", &receive_player_states);
            network::on("authRequest", &receive_auth_request);

            scheduler::loop(
                [] {
                    renderer::draw_text("Players: " + std::to_string(get_player_count()), {60.0f, 30.0f},
                                        {0xFF, 0xFF, 0xFF, 0xFF});
                },
                scheduler::renderer);
        }
    };
}

REGISTER_COMPONENT(scripting_experiments::component)
