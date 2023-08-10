#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "../network/socket.hpp"

namespace
{
	namespace scripting
	{
		struct unknown_script_context
		{
			uint64_t some_value;
			char pad[0x28];
			uint8_t* some_stack;
		};


		struct script_string
		{
			wchar_t* string{};
			uint32_t length{};
		};

		class managed_script_string
		{
		public:
			managed_script_string()
			{
				this->str_.length = *reinterpret_cast<uint32_t*>(0x1451CF140);
				this->str_.string = reinterpret_cast<wchar_t* (*)(uint64_t, uint64_t, uint64_t, uint64_t)>(0x1402597F0)(
					0, 2ULL * this->str_.length, 2, 14);
			}

			managed_script_string(const std::string& str)
				: managed_script_string(utils::string::convert(str))
			{
			}

			managed_script_string(const std::wstring& str)
			{
				this->str_.length = static_cast<uint32_t>(str.size() + 1);
				this->str_.string = reinterpret_cast<wchar_t* (*)(uint64_t, uint64_t, uint64_t, uint64_t)>(0x1402597F0)(
					0, 2ULL * this->str_.length, 2, 14);
				memcpy(this->str_.string, str.data(), static_cast<size_t>(this->str_.length) * 2);
			}

			~managed_script_string()
			{
				if (this->str_.string)
				{
					reinterpret_cast<void(*)(wchar_t*)>(0x140259710)(this->str_.string);
				}
			}

			managed_script_string(const managed_script_string&) = delete;
			managed_script_string& operator=(const managed_script_string&) = delete;

			managed_script_string(managed_script_string&& obj) noexcept
			{
				this->operator=(std::move(obj));
			}

			managed_script_string& operator=(managed_script_string&& obj) noexcept
			{
				if (this != &obj)
				{
					this->~managed_script_string();
					this->str_ = obj.str_;
					obj.str_ = {};
				}

				return *this;
			}

			std::wstring to_wstring() const
			{
				if (this->str_.length == 0)
				{
					return {};
				}

				return {this->str_.string, this->str_.length - 1};
			}

			std::string to_string() const
			{
				return utils::string::convert(this->to_wstring());
			}

			script_string str_{};
		};

		struct vec3_t
		{
			float x{0.0};
			float y{0.0};
			float z{0.0};
		};

		template <typename T>
		T read_argument(unknown_script_context* ctx)
		{
			T value{};
			(*reinterpret_cast<void(**)(uint64_t, unknown_script_context*, T*)>(0x144DFE040 + static_cast<uint32_t>(*(
				ctx->some_stack++)) * 8))(
				ctx->some_value, ctx, &value);
			return value;
		}

		template <>
		std::wstring read_argument<std::wstring>(unknown_script_context* ctx)
		{
			const auto managed_string = read_argument<managed_script_string>(ctx);
			return managed_string.to_wstring();
		}

		template <>
		std::string read_argument<std::string>(unknown_script_context* ctx)
		{
			const auto managed_string = read_argument<managed_script_string>(ctx);
			return managed_string.to_string();
		}

		template <typename T>
		auto convertReturnValue(T val)
		{
			static_assert(std::is_arithmetic_v<T>);
			return val;
		}

		template <>
		auto convertReturnValue(vec3_t val)
		{
			return val;
		}

		template <>
		auto convertReturnValue(std::string val)
		{
			return managed_script_string(val);
		}

		std::mutex m{};
		network::address otherAddr{};
		std::optional<vec3_t> otherPos{};
		std::optional<vec3_t> myPos{};
		std::optional<network::socket> sock{};

		//using script_function = void(*)(void* a1, unknown_script_context* ctx, uint64_t* return_value);

		void test_func(void* /*a1*/, unknown_script_context* ctx, vec3_t* return_value)
		{
			const auto argument = read_argument<vec3_t>(ctx);
			/*const auto argument2 = read_argument<uint32_t>(ctx);
			const auto argument3 = read_argument<managed_script_string>(ctx);*/

			// No idea what that is, yet
			++ctx->some_stack;

			if (sock)
			{
				sock->send(otherAddr, std::string(reinterpret_cast<const char*>(&argument), sizeof(argument)));
			}

			std::lock_guard _{m};
			myPos = argument;

			if (return_value && otherPos)
			{
				*return_value = *otherPos;
			}
		}

		using script_function = void(void* a1, unknown_script_context* ctx, void* return_value);

		void registerFunction(const wchar_t* name, script_function* function)
		{
			struct CFunction;
			struct script_context;

			auto* performMemoryAllocation = reinterpret_cast<void* (*)(size_t size, size_t maybeAlignment)>(
				0x140259780);
			auto* registerScriptString = reinterpret_cast<void (*)(int* str_struct, const wchar_t* str)>(0x14029F360);
			auto* CFunction_ctor = reinterpret_cast<CFunction*(*)(CFunction* str_struct, int* str_id,
			                                                      script_function* func)>(0x1402BAFB0);
			auto* scriptingContextSingleton = reinterpret_cast<script_context*(*)()>(0x14025AC70);
			auto* registerScriptFunction = reinterpret_cast<void(*)(script_context* ctx, CFunction* func)>(0x14029AB70);

			constexpr size_t size = 192;
			auto* memory = static_cast<CFunction*>(performMemoryAllocation(size, 16));
			if (memory)
			{
				int str_id;

				memset(memory, 0, size);
				registerScriptString(&str_id, name);
				CFunction_ctor(memory, &str_id, function);
				//auto fn_id = *(DWORD*)((uint8_t*)memory + 0xA8);
				//(void**)memory)[1] = (void*)0x12345;
				//*(DWORD*)((uint8_t*)memory + 0x14) = 1;
				//OutputDebugStringA("");
			}

			auto* ctx = scriptingContextSingleton();
			registerScriptFunction(ctx, memory);
		}


		template <auto Function, typename Return, typename... Args>
		void dispatcher_function(void* /*a1*/, unknown_script_context* ctx, void* return_value)
		{
			try
			{
				if constexpr (std::is_same_v<Return, void>)
				{
					Function(read_argument<Args>(ctx)...);
				}
				else
				{
					auto value = convertReturnValue(Function(read_argument<Args>(ctx)...));

					if (return_value)
					{
						*static_cast<decltype(value)*>(return_value) = std::move(value);
					}
				}
			}
			catch (...)
			{
			}

			++ctx->some_stack;
		}

		template <typename Return, typename... Args>
		struct dispatcher_helper
		{
			dispatcher_helper(Return (*)(Args...))
			{
			}

			template <auto Function>
			script_function* create() const
			{
				return &dispatcher_function<Function, Return, std::remove_cvref_t<Args>...>;
			}
		};

		template <auto Function>
		script_function* create_dispatcher_function()
		{
			dispatcher_helper helper(Function);
			return helper.template create<Function>();
		}

		template <auto Function>
		void register_function(const wchar_t* name)
		{
			auto* dispatcher = create_dispatcher_function<Function>();
			registerFunction(name, dispatcher);
		}

		std::string my_script_func(const std::string& str)
		{
			return "My string: " + str;
		}

		void register_script_functions_stub()
		{
			reinterpret_cast<void(*)()>(0x14102D6F0)();

			register_function<my_script_func>(L"Testicles");
			registerFunction(L"MomoTestFunc", reinterpret_cast<script_function*>(test_func));
		}

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				// Prevent pausing the game when focus is lost
				utils::hook::set<uint8_t>(0x14035F039, 0xEB);

				utils::hook::copy_string(0x1421133A0,
				                         ("my-mutex-name-" + std::to_string(static_cast<uint64_t>(time(nullptr)))).
				                         data());

				utils::hook::call(0x1410058B7, register_script_functions_stub);

				//utils::hook::jump(0x140E68390, test_func);

				/*utils::hook::call(0x140E6CDF8, +[](void* a1, const wchar_t* /*a2*)
				{
					reinterpret_cast<void(*)(void*, const wchar_t*)>(0x14029F360)(a1, L"MomoTestFunc");
				});*/
			}
		};
	}
}

REGISTER_COMPONENT(scripting::component)
