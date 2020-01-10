#pragma once
#include "utils/nt.hpp"

namespace loader
{
	using resolver = std::function<void*(const std::string& module, const std::string & function)>;
	
	utils::nt::module get_module();
	utils::nt::module load(const std::string& name, const resolver& import_resolver = {});
}
