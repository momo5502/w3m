#pragma once

#include <string>
#include <rapidjson/document.h>

namespace utils
{
    class properties
    {
      public:
        properties(std::string path);

        template <typename T>
        bool get(const std::string& name, T& value);

        template <typename T>
        void set(const std::string& name, const T& value);

        void remove(const std::string& name);

      private:
        std::string path_;
        rapidjson::Document document_;

        void save() const;
        void create_member(const std::string& name);
    };
}
