#pragma once
#include "utils/nt.hpp"

namespace loader
{
	using resolver = std::function<void*(const std::string& module, const std::string & function)>;
	
	utils::nt::module get_game_module();
	utils::nt::module get_main_module();
	utils::nt::module load(const std::string& name, const resolver& import_resolver = {});
}

// Outside the namespace for easier use
size_t operator"" _g(const size_t val);