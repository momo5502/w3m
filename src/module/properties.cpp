#include <std_include.hpp>
#include "properties.hpp"
#include "utils/nt.hpp"
#include "utils/io.hpp"

properties::properties()
{
	this->load();
}

void properties::remove(const std::string& name)
{
	auto* instance = module_loader::get<properties>();
	if (instance)
	{
		instance->remove_internal(name);
	}
}

void properties::load()
{
	std::string data;
	const auto path = get_path();
	if(utils::io::read_file(path, &data))
	{
		this->document_.Parse(data.data(), data.size());
	}
	
	if(!this->document_.IsObject())
	{
		this->document_.SetObject();
	}
}

void properties::save() const
{
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	this->document_.Accept(writer);

	const std::string result(buffer.GetString(), buffer.GetLength());
	utils::io::write_file(get_path(), result);
}

void properties::create_member(const std::string& name)
{
	if(!this->document_.HasMember(name.data()))
	{
		rapidjson::Value key(name.data(), rapidjson::SizeType(name.size()));
		rapidjson::Value value(rapidjson::kNullType);
		this->document_.AddMember(key, value, this->document_.GetAllocator());
	}
}

std::string properties::get_path()
{
	const auto out_folder = std::filesystem::path(utils::nt::module().get_folder());
	const auto out_path = out_folder / "w3x.json";
	return out_path.generic_string();
}

void properties::remove_internal(const std::string& name)
{
	if(this->document_.HasMember(name.data()))
	{
		this->document_.RemoveMember(name.data());
	}
}

template <>
bool properties::get_internal(const std::string& name, std::string& value)
{
	if(!this->document_.HasMember(name.data()))
	{
		return false;
	}

	auto& val = this->document_[name.data()];
	if(!val.IsString())
	{
		return false;
	}

	value = std::string(val.GetString(), val.GetStringLength());
	return true;
}

template <>
bool properties::get_internal(const std::string& name, int64_t& value)
{
	if (!this->document_.HasMember(name.data()))
	{
		return false;
	}

	auto& val = this->document_[name.data()];
	if (!val.IsInt64())
	{
		return false;
	}

	value = val.GetInt64();
	return true;
}

template <>
bool properties::get_internal(const std::string& name, double& value)
{
	if (!this->document_.HasMember(name.data()))
	{
		return false;
	}

	auto& val = this->document_[name.data()];
	if (!val.IsDouble())
	{
		return false;
	}

	value = val.GetDouble();
	return true;
}

template <>
void properties::set_internal(const std::string& name, const std::string& value)
{
	this->create_member(name);

	auto& val = this->document_[name.data()];
	val.SetString(value.data(), rapidjson::SizeType(value.size()));

	this->save();
}

template <>
void properties::set_internal(const std::string& name, const int64_t& value)
{
	this->create_member(name);

	auto& val = this->document_[name.data()];
	val.SetInt64(value);
	
	this->save();
}

template <>
void properties::set_internal(const std::string& name, const double& value)
{
	this->create_member(name);

	auto& val = this->document_[name.data()];
	val.SetDouble(value);

	this->save();
}

REGISTER_MODULE(properties);
