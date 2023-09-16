#pragma once

#include "component_interface.hpp"

#include <memory>
#include <vector>

namespace component_loader
{
	std::vector<std::unique_ptr<component_interface>>& get_components();
	void register_component(std::unique_ptr<component_interface>&& component);

	class premature_shutdown_trigger final : public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override
		{
			return "Premature shutdown requested";
		}
	};

	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of_v<component_interface, T>, "Component has invalid base class");

	public:
		installer()
		{
			register_component(std::make_unique<T>());
		}
	};

	template <typename T>
	T* get()
	{
		for (const auto& component : get_components())
		{
			if (typeid(*component) == typeid(T))
			{
				return reinterpret_cast<T*>(component.get());
			}
		}

		return nullptr;
	}

	bool post_start();
	bool post_load();
	void pre_destroy();

	void* load_import(const std::string& module, const std::string& function);

	void trigger_premature_shutdown();
}

#define REGISTER_COMPONENT(name)           \
namespace                                  \
{                                          \
	component_loader::installer<name> _ci; \
}
