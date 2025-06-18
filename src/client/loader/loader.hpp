#pragma once

#include <utils/nt.hpp>

namespace loader
{
    using resolver = std::function<void*(const std::string& module, const std::string& function)>;

    utils::nt::library get_game_module();
    utils::nt::library get_main_module();
    utils::nt::library load(const std::filesystem::path& file, const resolver& import_resolver = {});
    void apply_main_module(const utils::nt::library& mod);

    size_t reverse_g(size_t val);
    size_t reverse_g(const void* val);
}

// Outside the namespace for easier use
size_t operator"" _g(size_t val);
