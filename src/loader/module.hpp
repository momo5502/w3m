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

	virtual void* load_import(const std::string& module, const std::string& function)
	{
		return nullptr;
	}
};
