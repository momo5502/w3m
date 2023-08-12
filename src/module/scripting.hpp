#pragma once

namespace scripting
{
	namespace game
	{
		struct vec3_t
		{
			float x{ 0.0 };
			float y{ 0.0 };
			float z{ 0.0 };
		};

		struct CFunction
		{
			char pad[192];
		};

		struct global_script_context;

		struct script_execution_context
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

		using script_function = void(void* a1, script_execution_context* ctx, void* return_value);
	}

	namespace detail
	{
		class managed_script_string
		{
		public:
			managed_script_string();
			managed_script_string(const std::string& str);
			managed_script_string(const std::wstring& str);

			~managed_script_string();

			managed_script_string(const managed_script_string& obj);
			managed_script_string& operator=(const managed_script_string& obj);

			managed_script_string(managed_script_string&& obj) noexcept;
			managed_script_string& operator=(managed_script_string&& obj) noexcept;

			std::wstring to_wstring() const;
			std::string to_string() const;

			game::script_string str_{};
		};

		void read_script_argument(game::script_execution_context& ctx, void* value);
		void register_script_function(const wchar_t* name, game::script_function* function);


		template <typename T>
		T read_script_argument(game::script_execution_context& ctx)
		{
			T value{};
			read_script_argument(ctx, &value);
			return value;
		}

		template <>
		inline std::wstring read_script_argument<std::wstring>(game::script_execution_context& ctx)
		{
			const auto managed_string = read_script_argument<managed_script_string>(ctx);
			return managed_string.to_wstring();
		}

		template <>
		inline std::string read_script_argument<std::string>(game::script_execution_context& ctx)
		{
			const auto managed_string = read_script_argument<managed_script_string>(ctx);
			return managed_string.to_string();
		}

		template <typename T>
		auto adapt_return_value(T val)
		{
			static_assert(std::is_arithmetic_v<T>);
			return val;
		}

		template <>
		inline auto adapt_return_value(game::vec3_t val)
		{
			return val;
		}

		template <>
		inline auto adapt_return_value(std::string val)
		{
			return managed_script_string(val);
		}


		template <auto Function, typename Return, typename... Args>
		void dispatcher_function(void* /*a1*/, game::script_execution_context* ctx, void* return_value)
		{
			try
			{
				if constexpr (std::is_same_v<Return, void>)
				{
					Function(read_script_argument<Args>(*ctx)...);
				}
				else
				{
					auto value = adapt_return_value(Function(read_script_argument<Args>(*ctx)...));

					if (return_value)
					{
						*static_cast<decltype(value)*>(return_value) = std::move(value);
					}
				}
			}
			catch (const std::exception& e)
			{
				OutputDebugStringA(e.what());
			}

			++ctx->some_stack;
		}

		template <typename Func>
		struct dispatcher_helper;

		template <typename Return, typename... Args>
		struct dispatcher_helper<Return(*)(Args...)>
		{
			template <auto Function>
			game::script_function* create_dispatcher() const
			{
				return &dispatcher_function<Function, Return, std::remove_cvref_t<Args>...>;
			}
		};

		template <auto Function>
		game::script_function* create_dispatcher_function()
		{
			using FunctionType = decltype(Function);
			const dispatcher_helper<FunctionType> helper{};
			return helper.template create_dispatcher<Function>();
		}
	}

	template <auto Function>
	void register_function(const wchar_t* name)
	{
		auto* dispatcher = detail::create_dispatcher_function<Function>();
		detail::register_script_function(name, dispatcher);
	}
}
