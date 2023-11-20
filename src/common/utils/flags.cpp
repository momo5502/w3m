#include "flags.hpp"
#include "string.hpp"
#include "nt.hpp"

#include <unordered_set>

namespace utils
{
	namespace
	{
		std::unordered_set<std::string> parse_flags(const int argc, char** argv)
		{
			std::unordered_set<std::string> flags{};

			for (auto i = 0; i < argc && argv && argv[i]; ++i)
			{
				std::string flag(argv[i]);
				if (flag[0] == L'-')
				{
					flag.erase(flag.begin());
					flags.emplace(string::to_lower(std::move(flag)));
				}
			}

			return flags;
		}
	}

	flags::flags(const int argc, char** argv)
		: flags_(parse_flags(argc, argv))
	{
	}

	bool flags::has_flag(std::string flag) const
	{
		return this->flags_.contains(string::to_lower(std::move(flag)));
	}
}
