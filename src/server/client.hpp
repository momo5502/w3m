#pragma once

#include <chrono>
#include <optional>

#include <game/structs.hpp>
#include <utils/cryptography.hpp>

struct client
{
    std::chrono::high_resolution_clock::time_point last_packet{};
    uint64_t guid{};
    std::string name{};
    game::player_state current_state{};
    std::string authentication_nonce{};
    std::optional<utils::cryptography::ecc::key> public_key{};
    uint64_t state_id{0};
    bool has_printed_failure{false};

    bool is_authenticated() const
    {
        return this->public_key.has_value();
    }
};
