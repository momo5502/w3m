#pragma once

#include <string>

#define CHUNK 16384u

namespace utils::compression
{
    namespace zlib
    {
        std::string compress(const std::string& data);
        std::string decompress(const std::string& data);
    }
};
