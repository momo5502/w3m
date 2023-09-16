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
		void* allocate_object(const size_t size)
		{
			return reinterpret_cast<void* (*)(uint64_t, size_t, uint64_t, uint64_t)>(0x1402597F0_g)(0, size, 2, 14);
		}

		void destroy_object(void* game)
		{
			if (game)
			{
				reinterpret_cast<void(*)(void*)>(0x140259710_g)(game);
			}
		}

		managed_script_string::managed_script_string(const std::string& str)
			: managed_script_string(utils::string::convert(str))
		{
		}

		managed_script_string& managed_script_string::operator=(const std::string& str)
		{
			*this = managed_script_string(str);
			return *this;
		}

		managed_script_string::managed_script_string(const std::wstring_view& str)
		{
			this->str_.length = static_cast<uint32_t>(str.size() + 1);
			this->str_.string = allocate<wchar_t>(this->str_.length);

			memcpy(this->str_.string, str.data(), static_cast<size_t>(this->str_.length) * 2);
		}

		managed_script_string& managed_script_string::operator=(const std::wstring_view& str)
		{
			*this = managed_script_string(str);
			return *this;
		}

		managed_script_string::~managed_script_string()
		{
			destroy_object(this->str_.string);
		}

		managed_script_string::managed_script_string(const managed_script_string& obj)
		{
			this->operator=(obj);
		}

		managed_script_string& managed_script_string::operator=(const managed_script_string& obj)
		{
			if (this != &obj)
			{
				*this = obj.to_view();
			}

			return *this;
		};

		managed_script_string::managed_script_string(managed_script_string&& obj) noexcept
		{
			this->operator=(std::move(obj));
		}

		managed_script_string& managed_script_string::operator=(managed_script_string&& obj) noexcept
		{
			if (this != &obj)
			{
				this->~managed_script_string();
				this->str_ = obj.str_;
				obj.str_ = {};
			}

			return *this;
		}

		std::wstring_view managed_script_string::to_view() const
		{
			if (this->str_.length == 0)
			{
				return {};
			}

			return {this->str_.string, this->str_.length - 1};
		}

		std::wstring managed_script_string::to_wstring() const
		{
			return std::wstring{this->to_view()};
		}

		std::string managed_script_string::to_string() const
		{
			return utils::string::convert(this->to_wstring());
		}

		bool managed_script_string::operator==(const managed_script_string& obj) const
		{
			return this->to_view() == obj.to_view();
		}

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

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::hook::call(0x1410058B7_g, register_script_functions_stub);
		}
	};
}

REGISTER_COMPONENT(scripting::component)
