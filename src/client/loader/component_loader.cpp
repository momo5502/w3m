#include "../std_include.hpp"
#include "component_loader.hpp"

std::vector<std::unique_ptr<component_interface>>& component_loader::get_components()
{
	static std::vector<std::unique_ptr<component_interface>> components{};
	return components;
}

void component_loader::register_component(std::unique_ptr<component_interface>&& component)
{
	get_components().emplace_back(std::move(component));
}

bool component_loader::post_start()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	try
	{
		for (const auto& component : get_components())
		{
			component->post_start();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

bool component_loader::post_load()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	try
	{
		for (const auto& component : get_components())
		{
			component->post_load();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

void component_loader::pre_destroy()
{
	static auto handled = false;
	if (handled) return;
	handled = true;

	for (const auto& component : get_components())
	{
		component->pre_destroy();
	}
}

void* component_loader::load_import(const std::string& module, const std::string& function)
{
	void* function_ptr = nullptr;

	for (const auto& component : get_components())
	{
		const auto module_function_ptr = component->load_import(module, function);
		if (module_function_ptr)
		{
			function_ptr = module_function_ptr;
		}
	}

	return function_ptr;
}

void component_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}
