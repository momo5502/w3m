#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <utils/hook.hpp>

namespace patches
{
    struct component final : component_interface
    {
        void post_load() override
        {
            // Prevent pausing the game when focus is lost
            utils::hook::set<uint8_t>(0x141495479_g, 0xEB);

            // Register unique mutex to allow parallel instances
            utils::hook::copy_string(0x1423291F8_g, ("w3m-mutex-" + std::to_string(static_cast<uint64_t>(time(nullptr)))).data());
        }
    };
}

REGISTER_COMPONENT(patches::component)
