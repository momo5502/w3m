#pragma once
#pragma warning(disable: 4324)

namespace scripting
{
	namespace game
	{
		__declspec(align(16)) struct Vector
		{
			float X{0.0};
			float Y{0.0};
			float Z{0.0};
			float W{0.0};
		};

#pragma pack(push)
#pragma pack(1)
		struct EulerAngles
		{
			float Pitch{0.0};
			float Yaw{0.0};
			float Roll{0.0};
		};
#pragma pack(pop)

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

#pragma pack(push)
#pragma pack(1)
		struct script_string
		{
			wchar_t* string{};
			uint32_t length{};
		};
#pragma pack(pop)

		using script_function = void(void* a1, script_execution_context* ctx, void* return_value);
	}

	namespace detail
	{
		void* allocate_object(size_t size);
		void destroy_object(void* game);

		template <typename T>
		T* allocate(const size_t count = 1)
		{
			auto* array = static_cast<T*>(allocate_object(sizeof(T) * count));

			for (size_t i = 0; i < count; ++i)
			{
				new(array + i) T();
			}

			return array;
		}

		class managed_script_string
		{
		public:
			managed_script_string() = default;

			managed_script_string(const std::string& str);
			managed_script_string& operator=(const std::string& str);

			managed_script_string(const std::wstring_view& str);
			managed_script_string& operator=(const std::wstring_view& str);

			~managed_script_string();

			managed_script_string(const managed_script_string& obj);
			managed_script_string& operator=(const managed_script_string& obj);

			managed_script_string(managed_script_string&& obj) noexcept;
			managed_script_string& operator=(managed_script_string&& obj) noexcept;

			bool operator==(const managed_script_string& obj) const;

			bool operator!=(const managed_script_string& obj) const
			{
				return !this->operator==(obj);
			}

			std::wstring to_wstring() const;
			std::string to_string() const;
			std::wstring_view to_view() const;

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
			//static_assert(std::is_arithmetic_v<T>);
			return val;
		}

		template <>
		inline auto adapt_return_value(game::Vector val)
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

		template <typename Return, typename... Args>
		struct dispatcher_helper
		{
			dispatcher_helper(Return (*)(Args...))
			{
			}

			template <auto Function>
			game::script_function* create_dispatcher() const
			{
				return &dispatcher_function<Function, Return, std::remove_cvref_t<Args>...>;
			}
		};

		template <auto Function>
		game::script_function* create_dispatcher_function()
		{
			const dispatcher_helper helper{Function};
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
