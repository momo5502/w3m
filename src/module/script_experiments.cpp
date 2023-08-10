#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "loader/loader.hpp"

#include <utils/hook.hpp>

#include "../network/socket.hpp"

namespace
{
	namespace script_experiments
	{
		struct unknown_script_context
		{
			uint64_t some_value;
			char pad[0x28];
			uint8_t* some_stack;
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
				this->str_.string = reinterpret_cast<wchar_t*(*)(uint64_t, uint64_t, uint64_t, uint64_t)>(0x1402597F0)(
					0, 2ULL * this->str_.length, 2, 14);
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

			script_string str_{};
		};

		struct vec3_t
		{
			float x{0.0};
			float y{0.0};
			float z{0.0};
		};

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

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				auto& s = sock.emplace();
				s.set_blocking(false);

				network::address myaddr{"0.0.0.0"};
				myaddr.set_port(28960);
				s.bind_port(myaddr);

				wchar_t name[MAX_PATH]{ 0 };
				GetHostNameW(name, sizeof(name));

				network::address oa{name == std::wstring(L"Maurice-Laptop") ? "192.168.178.34" : "192.168.178.47"};
				oa.set_port(28960);
				otherAddr = oa;

				std::thread([&s]
				{
					while (true)
					{
						std::string data{};
						network::address target{};

						while (s.receive(target, data))
						{
							if (data.size() == sizeof(vec3_t))
							{
								std::lock_guard _{m};
								otherPos = *reinterpret_cast<vec3_t*>(data.data());
							}
						}

						(void)s.sleep(1s);
					}
				}).detach();

				// Prevent pausing the game when focus is lost
				utils::hook::set<uint8_t>(0x14035F039, 0xEB);

				utils::hook::copy_string(0x1421133A0,
				                         ("my-mutex-name-" + std::to_string(static_cast<uint64_t>(time(nullptr)))).data());

				utils::hook::jump(0x140E68390, test_func);

				utils::hook::call(0x140E6CDF8, +[](void* a1, const wchar_t* /*a2*/)
				{
					reinterpret_cast<void(*)(void*, const wchar_t*)>(0x14029F360)(a1, L"MomoTestFunc");
				});
			}
		};
	}
}

///REGISTER_COMPONENT(script_experiments::component)
