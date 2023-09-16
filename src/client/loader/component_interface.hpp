#pragma once

#include <string>

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
	// Must not necessarily be called
	virtual void pre_destroy()
	{
	}

	virtual void* load_import([[maybe_unused]] const std::string& module, [[maybe_unused]] const std::string& function)
	{
		return nullptr;
	}
};
