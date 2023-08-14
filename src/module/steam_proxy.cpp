#include <std_include.hpp>
#include "steam_proxy.hpp"
#include "scheduler.hpp"
#include "utils/nt.hpp"
#include "utils/string.hpp"
#include "loader/loader.hpp"
#include "loader/component_loader.hpp"
#include "steam/interface.hpp"

namespace steam_proxy
{
	namespace
	{
		class component final : public component_interface
		{
		public:
			component()
			{
				this->run_mod();
			}

			void post_load() override
			{
				this->load_client();
				this->clean_up_on_error();

				try
				{
					this->start_mod("Witcher 3: Online", 292030);
				}
				catch (std::exception& e)
				{
					printf("Steam: %s\n", e.what());
				}
			}

			void pre_destroy() override
			{
				if (this->steam_client_module_)
				{
					if (this->steam_pipe_)
					{
						if (this->global_user_)
						{
							this->steam_client_module_.invoke<void>("Steam_ReleaseUser", this->steam_pipe_,
							                                        this->global_user_);
						}

						this->steam_client_module_.invoke<bool>("Steam_BReleaseSteamPipe", this->steam_pipe_);
					}
				}
			}

		private:
			utils::nt::library steam_client_module_{};
			utils::nt::library steam_overlay_module_{};

			steam::interface client_engine_{};
			steam::interface client_user_{};
			steam::interface client_utils_{};

			void* steam_pipe_ = nullptr;
			void* global_user_ = nullptr;

			void run_mod() const
			{
				const auto command = "-proc ";
				const char* parent_proc = strstr(GetCommandLineA(), command);

				if (parent_proc)
				{
					const auto pid = atoi(parent_proc + strlen(command));
					const auto process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid);
					if (process_handle && process_handle != INVALID_HANDLE_VALUE)
					{
						WaitForSingleObject(process_handle, INFINITE);
						CloseHandle(process_handle);
					}

					TerminateProcess(GetCurrentProcess(), 0);
				}
			}

			void* load_client_engine() const
			{
				if (!this->steam_client_module_) return nullptr;

				for (auto i = 1; i > 0; ++i)
				{
					std::string name = utils::string::va("CLIENTENGINE_INTERFACE_VERSION%03i", i);
					const auto client_engine = this->steam_client_module_
					                               .invoke<void*>("CreateInterface", name.data(), nullptr);
					if (client_engine) return client_engine;
				}

				return nullptr;
			}

			void load_client()
			{
				const auto steam_path = get_steam_install_directory();
				if (steam_path.empty()) return;

				utils::nt::library::load(steam_path / "tier0_s64.dll");
				utils::nt::library::load(steam_path / "vstdlib_s64.dll");
				this->steam_overlay_module_ = utils::nt::library::load(steam_path / "gameoverlayrenderer64.dll");
				this->steam_client_module_ = utils::nt::library::load(steam_path / "steamclient64.dll");
				if (!this->steam_client_module_) return;

				this->client_engine_ = load_client_engine();
				if (!this->client_engine_) return;

				this->steam_pipe_ = this->steam_client_module_.invoke<void*>("Steam_CreateSteamPipe");
				this->global_user_ = this->steam_client_module_.invoke<void*>(
					"Steam_ConnectToGlobalUser", this->steam_pipe_);
				this->client_user_ = this->client_engine_.invoke<void*>(8, this->steam_pipe_, this->global_user_);
				// GetIClientUser
				this->client_utils_ = this->client_engine_.invoke<void*>(13, this->steam_pipe_); // GetIClientUtils
			}

			void start_mod(const std::string& title, size_t app_id)
			{
				if (!this->client_utils_ || !this->client_user_) return;

				if (!this->client_user_.invoke<bool>("BIsSubscribedApp", app_id))
				{
					app_id = 480; // Spacewar
				}

				this->client_utils_.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

				const auto self = loader::get_main_module();
				const auto path = self.get_path();

				char our_directory[MAX_PATH] = {0};
				GetCurrentDirectoryA(sizeof(our_directory), our_directory);


				const std::string cmdline = utils::string::va("\"%s\" -proc %d", path.generic_string().data(), GetCurrentProcessId());

				steam::game_id game_id;
				game_id.raw.type = 1; // k_EGameIDTypeGameMod
				game_id.raw.app_id = app_id & 0xFFFFFF;

				const auto* mod_id = "W3X.";
				game_id.raw.mod_id = *reinterpret_cast<const unsigned int*>(mod_id) | 0x80000000;

				this->client_user_.invoke<bool>("SpawnProcess", self.get_path().generic_string().data(), cmdline.data(), our_directory,
				                                &game_id.bits, title.data(), 0, 0, 0);
			}

			void clean_up_on_error()
			{
				scheduler::frame([this]()
				{
					if (this->steam_client_module_
						&& this->steam_pipe_
						&& this->global_user_
						&& this->steam_client_module_.invoke<bool>("Steam_BConnected", this->global_user_,
						                                           this->steam_pipe_)
						&& this->steam_client_module_.invoke<bool>("Steam_BLoggedOn", this->global_user_,
						                                           this->steam_pipe_))
					{
						return scheduler::cond_continue;
					}

					this->client_engine_ = nullptr;
					this->client_user_ = nullptr;
					this->client_utils_ = nullptr;

					this->steam_pipe_ = nullptr;
					this->global_user_ = nullptr;

					this->steam_client_module_ = utils::nt::library{nullptr};

					return scheduler::cond_end;
				});
			}
		};
	}

	std::filesystem::path get_steam_install_directory()
	{
		HKEY reg_key;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\Valve\\Steam", 0, KEY_QUERY_VALUE,
		                  &reg_key) !=
			ERROR_SUCCESS)
		{
			return {};
		}

		char path[MAX_PATH] = {0};
		DWORD length = sizeof(path);
		RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path),
		                 &length);
		RegCloseKey(reg_key);

		return path;
	}
}

//REGISTER_COMPONENT(steam_proxy::component)
