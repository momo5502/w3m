#pragma once

#include <string>

enum class component_priority
{
	min = 0,
	// must run after the steam_proxy
	name,
	// must run after the updater
	steam_proxy,
	launcher,
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
