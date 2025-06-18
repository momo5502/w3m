#pragma once

#include <string>
#include <unordered_set>

namespace utils
{
    class flags
    {
      public:
        flags(int argc, char** argv);

        bool has_flag(std::string flag) const;

      private:
        std::unordered_set<std::string> flags_{};
    };
}
