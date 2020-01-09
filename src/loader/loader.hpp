#pragma once
#include "utils/nt.hpp"

namespace loader
{
	utils::nt::module get_module();
	utils::nt::module load(const std::string& name);
}
