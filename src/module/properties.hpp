#pragma once
#include "loader/module_loader.hpp"

class properties final : public module
{	
public:
	properties();

	template <typename T>
	static bool get(const std::string& name, T& value)
	{
		auto* instance = module_loader::get<properties>();
		if(instance)
		{
			return instance->get_internal(name, value);
		}

		return false;
	}

	template <typename T>
	static void set(const std::string& name, const T& value)
	{
		auto* instance = module_loader::get<properties>();
		if (instance)
		{
			instance->set_internal(name, value);
		}
	}

	static void remove(const std::string& name);

private:
	rapidjson::Document document_;

	void load();
	void save() const;
	void create_member(const std::string& name);

	static std::string get_path();

	template <typename T>
	bool get_internal(const std::string& name, T& value);

	template <typename T>
	void set_internal(const std::string& name, const T& value);

	void remove_internal(const std::string& name);
};
