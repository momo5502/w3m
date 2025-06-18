#include "../std_include.hpp"

#include "../loader/loader.hpp"

#include "properties.hpp"
#include "game_path.hpp"
#include "utils/nt.hpp"

namespace properties::detail
{
    namespace
    {
        std::filesystem::path get_filesystem_path()
        {
            return game_path::get_appdata_path() / "user/properties.json";
        }

        std::string get_path()
        {
            return get_filesystem_path().generic_string();
        }
    }

    utils::properties& get_properties()
    {
        static utils::properties properties{get_path()};
        return properties;
    }
}
