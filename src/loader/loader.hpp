#pragma once
#include "utils/nt.hpp"

namespace loader
{
	using resolver = std::function<void*(const std::string& module, const std::string & function)>;
	
	utils::nt::module get_game_module();
	utils::nt::module get_main_module();
	utils::nt::module load(const std::string& name, const resolver& import_resolver = {});

	size_t reverse_g(size_t val);
	size_t reverse_g(const void* val);
}

// Outside the namespace for easier use
size_t operator"" _g(size_t val);