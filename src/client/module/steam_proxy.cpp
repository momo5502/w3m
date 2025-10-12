#include "../std_include.hpp"
#include "../loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <utils/finally.hpp>
#include <utils/concurrency.hpp>

#include "../steam/interface.hpp"
#include "../steam/steam.hpp"

#include "../game_version.hpp"

#include "steam_proxy.hpp"
#include "scheduler.hpp"

namespace steam_proxy
{
    namespace
    {
        utils::nt::library steam_client_module{};
        utils::nt::library steam_overlay_module{};

        steam::HSteamPipe steam_pipe = 0;
        steam::HSteamUser global_user = 0;

        steam::interface client_engine{};
        steam::interface client_user{};
        steam::interface client_utils{};
        steam::interface client_friends{};

        void* load_client_engine()
        {
            if (!steam_client_module)
                return nullptr;

            for (auto i = 1; i <= 999; ++i)
            {
                std::string name = utils::string::va("CLIENTENGINE_INTERFACE_VERSION%03i", i);
                auto* const temp_client_engine = steam_client_module.invoke<void*>("CreateInterface", name.data(), nullptr);
                if (temp_client_engine)
                    return temp_client_engine;
            }

            return nullptr;
        }

        const std::string& get_steam_install_path()
        {
            static std::string install_path{};
            if (!install_path.empty())
            {
                return install_path;
            }

            HKEY reg_key;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, R"(Software\WOW6432Node\Valve\Steam)", 0, KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS)
            {
                char path[MAX_PATH] = {0};
                DWORD length = sizeof(path);
                RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path), &length);
                RegCloseKey(reg_key);

                install_path = path;
            }

            return install_path;
        }

        constexpr uint32_t get_app_id()
        {
            return 292030;
        }

        void load_client()
        {
            SetEnvironmentVariableA("SteamAppId", std::to_string(get_app_id()).data());

            const std::filesystem::path steam_path = get_steam_install_path();
            if (steam_path.empty())
                return;

            utils::nt::library::load(steam_path / "tier0_s64.dll");
            utils::nt::library::load(steam_path / "vstdlib_s64.dll");
            steam_overlay_module = utils::nt::library::load(steam_path / "gameoverlayrenderer64.dll");
            steam_client_module = utils::nt::library::load(steam_path / "steamclient64.dll");
            if (!steam_client_module)
                return;

            client_engine = load_client_engine();
            if (!client_engine)
                return;

            steam_pipe = steam_client_module.invoke<steam::HSteamPipe>("Steam_CreateSteamPipe");
            global_user = steam_client_module.invoke<steam::HSteamUser>("Steam_ConnectToGlobalUser", steam_pipe);

            client_user = client_engine.invoke<void*>(8, global_user, steam_pipe);
            client_utils = client_engine.invoke<void*>(14, steam_pipe);
            client_friends = client_engine.invoke<void*>(13, global_user, steam_pipe);
        }

        void do_cleanup()
        {
            client_engine = nullptr;
            client_user = nullptr;
            client_utils = nullptr;
            client_friends = nullptr;

            steam_pipe = 0;
            global_user = 0;
        }

        bool perform_cleanup_if_needed()
        {
            if (steam_client_module && steam_pipe && global_user &&
                steam_client_module.invoke<bool>("Steam_BConnected", global_user, steam_pipe) &&
                steam_client_module.invoke<bool>("Steam_BLoggedOn", global_user, steam_pipe))
            {
                return false;
            }

            do_cleanup();
            return true;
        }

        void clean_up_on_error()
        {
            scheduler::schedule(
                [] {
                    if (perform_cleanup_if_needed())
                    {
                        return scheduler::cond_end;
                    }

                    return scheduler::cond_continue;
                },
                scheduler::async);
        }

        void start_mod_unsafe(const std::string& title, size_t app_id)
        {
            if (!client_utils || !client_user)
            {
                return;
            }

            if (!client_user.invoke<bool>("BIsSubscribedApp", app_id))
            {
                app_id = 480; // Spacewar
            }

            client_utils.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

            char our_directory[MAX_PATH] = {0};
            GetCurrentDirectoryA(sizeof(our_directory), our_directory);

            const auto self = utils::nt::library::get_by_address(start_mod_unsafe);
            const auto path = self.get_path();
            const auto* cmdline = utils::string::va("\"%s\" -proc %d", path.generic_string().data(), GetCurrentProcessId());

            steam::game_id game_id;
            game_id.raw.type = 1; // k_EGameIDTypeGameMod
            game_id.raw.app_id = app_id & 0xFFFFFF;

            const auto* mod_id = "w3m";
            game_id.raw.mod_id = *reinterpret_cast<const unsigned int*>(mod_id) | 0x80000000;

            client_user.invoke<bool>("SpawnProcess", path.generic_string().data(), cmdline, our_directory, &game_id.bits, title.data(), 0,
                                     0, 0);
        }

        void start_mod(const std::string& title, const size_t app_id)
        {
            __try
            {
                return start_mod_unsafe(title, app_id);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                do_cleanup();
            }
        }
    }

    struct component final : component_interface
    {
        void post_load() override
        {
            load_client();
            perform_cleanup_if_needed();

            start_mod("\xF0\x9F\x90\xBA"
                      " " W3M_NAME,
                      get_app_id());
            clean_up_on_error();
        }

        void pre_destroy() override
        {
            if (steam_client_module && steam_pipe)
            {
                if (global_user)
                {
                    steam_client_module.invoke<void>("Steam_ReleaseUser", steam_pipe, global_user);
                }

                (void)steam_client_module.invoke<bool>("Steam_BReleaseSteamPipe", steam_pipe);
            }
        }

        component_priority priority() const override
        {
            return component_priority::steam_proxy;
        }
    };

    const utils::nt::library& get_overlay_module()
    {
        return steam_overlay_module;
    }

    const char* get_player_name()
    {
        if (client_friends)
        {
            return client_friends.invoke<const char*>("GetPersonaName");
        }

        return nullptr;
    }
}

REGISTER_COMPONENT(steam_proxy::component)
