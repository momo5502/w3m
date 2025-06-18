#pragma once

#include <utils/properties.hpp>

namespace properties
{
    namespace detail
    {
        utils::properties& get_properties();
    }

    template <typename T>
    bool get(const std::string& name, T& value)
    {
        return detail::get_properties().get(name, value);
    }

    template <typename T>
    void set(const std::string& name, const T& value)
    {
        detail::get_properties().set(name, value);
    }

    inline void remove(const std::string& name)
    {
        detail::get_properties().remove(name);
    }
}
