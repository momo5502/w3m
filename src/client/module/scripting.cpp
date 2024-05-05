#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include "scripting.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace scripting
{
	namespace
	{
		using registration_info = std::pair<const wchar_t*, game::script_function*>;
		using registration_vector = std::vector<registration_info>;

		registration_vector& get_registration_vector()
		{
			static registration_vector v{};
			return v;
		}

		game::CFunction* allocate_cfunction(int name_id, game::script_function* function)
		{
			auto* perform_memory_allocation = reinterpret_cast<void* (*)(size_t size, size_t maybe_alignment)>(
				0x140259780_g);
			auto* CFunction_ctor = reinterpret_cast<game::CFunction * (*)(game::CFunction* str_struct, int* str_id,
			                                                              game::script_function* func)>(0x1402BAFB0_g);

			auto* memory = static_cast<game::CFunction*>(perform_memory_allocation(sizeof(game::CFunction), 16));
			if (!memory)
			{
				return memory;
			}

			memset(memory, 0, sizeof(game::CFunction));
			CFunction_ctor(memory, &name_id, function);

			return memory;
		}

		void register_cfunction(game::CFunction& function)
		{
			auto* scripting_context_singleton = reinterpret_cast<game::global_script_context * (*)()>(0x14025AC70_g);
			auto* register_script_function = reinterpret_cast<void(*)(game::global_script_context* ctx,
			                                                          game::CFunction* func)>(
				0x14029AB70_g);

			auto* ctx = scripting_context_singleton();
			register_script_function(ctx, &function);
		}

		int register_name_string(const wchar_t* name)
		{
			auto* register_name = reinterpret_cast<void(*)(int* str_struct, const wchar_t* str)>(0x14029F360_g);

			int name_id{};
			register_name(&name_id, name);
			return name_id;
		}

		void perform_script_function_registration(const wchar_t* name, game::script_function* function)
		{
			const auto name_id = register_name_string(name);
			auto* cfunction = allocate_cfunction(name_id, function);
			register_cfunction(*cfunction);
		}

		void register_script_functions_stub()
		{
			reinterpret_cast<void(*)()>(0x14102D6F0_g)();

			auto& registration_vector = get_registration_vector();
			for (const auto& entry : registration_vector)
			{
				perform_script_function_registration(entry.first, entry.second);
			}

			registration_vector.clear();
		}
	}

	namespace detail
	{
		void read_script_argument(game::script_execution_context& ctx, void* value)
		{
			using function_type = void(uint64_t, game::script_execution_context*, void*);

			const auto index = static_cast<uint32_t>(*(ctx.some_stack++));
			const auto function = reinterpret_cast<function_type**>(0x144DFE040_g)[index];
			function(ctx.some_value, &ctx, value);
		}

		void register_script_function(const wchar_t* name, game::script_function* function)
		{
			get_registration_vector().emplace_back(name, function);
		}
	}

	void* allocate_memory(const size_t size)
	{
		return reinterpret_cast<void* (*)(uint64_t, size_t, uint64_t, uint64_t)>(0x1402597F0_g)(0, size, 2, 14);
	}

	void free_memory(void* memory)
	{
		if (memory)
		{
			reinterpret_cast<void(*)(void*)>(0x140259710_g)(memory);
		}
	}

	string::string(const std::string_view& str)
		: string()
	{
		this->resize(str.size() + 1, 0);

		for (size_t i = 0; i < str.size(); ++i)
		{
			this->at(i) = str[i];
		}
	}

	string& string::operator=(const std::string_view& str)
	{
		*this = string(str);
		return *this;
	}

	string::string(const std::wstring_view& str)
		: string()
	{
		this->resize(str.size() + 1, 0);

		for (size_t i = 0; i < str.size(); ++i)
		{
			this->at(i) = str[i];
		}
	}

	string& string::operator=(const std::wstring_view& str)
	{
		*this = string(str);
		return *this;
	}

	string::string(const wchar_t* str)
		: string(std::wstring_view(str))
	{
	}

	string& string::operator=(const wchar_t* str)
	{
		*this = string(str);
		return *this;
	}

	string::string(const char* str, const size_t length)
		: string(std::string_view(str, length))
	{
	}

	string::string(const char* str)
		: string(std::string_view(str))
	{
	}

	string& string::operator=(const char* str)
	{
		*this = string(str);
		return *this;
	}

	std::wstring_view string::to_view() const
	{
		if (this->empty())
		{
			return {};
		}

		return {this->data(), this->size() - 1};
	}

	std::wstring string::to_wstring() const
	{
		return std::wstring{this->to_view()};
	}

	std::string string::to_string() const
	{
		return utils::string::convert(this->to_wstring());
	}

	bool string::operator==(const string& obj) const
	{
		if (this->size() != obj.size())
		{
			return false;
		}

		return memcmp(this->data(), obj.data(), this->size()) == 0;
	}

	bool string::operator!=(const string& obj) const
	{
		return !this->operator==(obj);
	}

	bool string::operator==(const std::string_view& obj) const
	{
		if (this->empty() && obj.empty())
		{
			return true;
		}

		if ((obj.size() + 1) != this->size())
		{
			return false;
		}

		for (size_t i = 0; i < obj.size(); ++i)
		{
			if (this->at(i) != static_cast<wchar_t>(obj[i]))
			{
				return false;
			}
		}

		return this->at(obj.size()) == 0;
	}

	bool string::operator!=(const std::string_view& obj) const
	{
		return !this->operator==(obj);
	}

	bool string::operator==(const std::wstring_view& obj) const
	{
		if (this->empty() && obj.empty())
		{
			return true;
		}

		if ((obj.size() + 1) != this->size())
		{
			return false;
		}

		return this->to_view() == obj && this->at(obj.size()) == 0;
	}

	bool string::operator!=(const std::wstring_view& obj) const
	{
		return !this->operator==(obj);
	}

	struct component final : component_interface
	{
		void post_load() override
		{
			utils::hook::call(0x1410058B7_g, register_script_functions_stub);
		}
	};
}

REGISTER_COMPONENT(scripting::component)
