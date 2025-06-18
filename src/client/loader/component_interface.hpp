#pragma once

#include <string>

enum class component_priority
{
    min = 0,
    splash,
    console,
    launcher,
    steam_proxy,
    updater,
};

class component_interface
{
  public:
    virtual ~component_interface() = default;

    virtual void post_start()
    {
    }

    virtual void post_load()
    {
    }

    // Thread cleanup.
    virtual void pre_destroy()
    {
    }

    virtual void* load_import([[maybe_unused]] const std::string& module, [[maybe_unused]] const std::string& function)
    {
        return nullptr;
    }

    virtual component_priority priority() const
    {
        return component_priority::min;
    }
};
