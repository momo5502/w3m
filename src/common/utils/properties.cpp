#include "properties.hpp"
#include "nt.hpp"
#include "io.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

namespace utils
{
    properties::properties(std::string path)
        : path_(std::move(path))
    {
        std::string data{};
        if (io::read_file(this->path_, &data))
        {
            this->document_.Parse(data.data(), data.size());
        }

        if (!this->document_.IsObject())
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
        utils::io::write_file(this->path_, result);
    }

    void properties::create_member(const std::string& name)
    {
        if (!this->document_.HasMember(name.data()))
        {
            rapidjson::Value key(name.data(), static_cast<rapidjson::SizeType>(name.size()));
            rapidjson::Value value(rapidjson::kNullType);
            this->document_.AddMember(key, value, this->document_.GetAllocator());
        }
    }

    void properties::remove(const std::string& name)
    {
        if (this->document_.HasMember(name.data()))
        {
            this->document_.RemoveMember(name.data());
        }
    }

    template <>
    bool properties::get(const std::string& name, std::string& value)
    {
        if (!this->document_.HasMember(name.data()))
        {
            return false;
        }

        const auto& val = this->document_[name.data()];
        if (!val.IsString())
        {
            return false;
        }

        value = std::string(val.GetString(), val.GetStringLength());
        return true;
    }

    template <>
    bool properties::get(const std::string& name, int64_t& value)
    {
        if (!this->document_.HasMember(name.data()))
        {
            return false;
        }

        const auto& val = this->document_[name.data()];
        if (!val.IsInt64())
        {
            return false;
        }

        value = val.GetInt64();
        return true;
    }

    template <>
    bool properties::get(const std::string& name, double& value)
    {
        if (!this->document_.HasMember(name.data()))
        {
            return false;
        }

        const auto& val = this->document_[name.data()];
        if (!val.IsDouble())
        {
            return false;
        }

        value = val.GetDouble();
        return true;
    }

    template <>
    void properties::set(const std::string& name, const std::string& value)
    {
        this->create_member(name);

        auto& val = this->document_[name.data()];
        val.SetString(value.data(), static_cast<rapidjson::SizeType>(value.size()));

        this->save();
    }

    template <>
    void properties::set(const std::string& name, const int64_t& value)
    {
        this->create_member(name);

        auto& val = this->document_[name.data()];
        val.SetInt64(value);

        this->save();
    }

    template <>
    void properties::set(const std::string& name, const double& value)
    {
        this->create_member(name);

        auto& val = this->document_[name.data()];
        val.SetDouble(value);

        this->save();
    }
}
