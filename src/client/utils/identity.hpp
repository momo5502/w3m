#pragma once

#include <utils/cryptography.hpp>

namespace utils::identity
{
    uint64_t get_guid();
    cryptography::ecc::key& get_key();
}
