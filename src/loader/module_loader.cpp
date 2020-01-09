#include <std_include.hpp>
#include "module_loader.hpp"

std::vector<std::unique_ptr<module>>* module_loader::modules_ = nullptr;

void module_loader::register_module(std::unique_ptr<module>&& module_)
{
	if (!modules_)
	{
		modules_ = new std::vector<std::unique_ptr<module>>();
		atexit(destroy_modules);
	}

	modules_->push_back(std::move(module_));
}

bool module_loader::post_start()
{
	static auto handled = false;
	if (handled || !modules_) return true;
	handled = true;

	try
	{
		for (const auto& module_ : *modules_)
		{
			module_->post_start();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

bool module_loader::post_load()
{
	static auto handled = false;
	if (handled || !modules_) return true;
	handled = true;

	try
	{
		for (const auto& module_ : *modules_)
		{
			module_->post_load();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

void module_loader::pre_destroy()
{
	static auto handled = false;
	if (handled || !modules_) return;
	handled = true;

	for (const auto& module_ : *modules_)
	{
		module_->pre_destroy();
	}
}

void* module_loader::load_import(const std::string& module, const std::string& function)
{
	void* function_ptr = nullptr;

	for (const auto& module_ : *modules_)
	{
		const auto module_function_ptr = module_->load_import(module, function);
		if (module_function_ptr)
		{
			function_ptr = module_function_ptr;
		}
	}

	return function_ptr;
}

void module_loader::destroy_modules()
{
	pre_destroy();

	if (!modules_) return;

	delete modules_;
	modules_ = nullptr;
}

void module_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}
