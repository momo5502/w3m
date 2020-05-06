#pragma once

class module
{
public:
	virtual ~module()
	{
	}

	virtual void post_start()
	{
	}

	virtual void post_load()
	{
	}

	virtual void pre_destroy()
	{
	}

	virtual void* load_import([[maybe_unused]] const std::string& module, [[maybe_unused]] const std::string& function)
	{
		return nullptr;
	}
};
