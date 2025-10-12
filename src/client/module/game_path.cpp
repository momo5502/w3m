#include "../std_include.hpp"
#include "game_path.hpp"

#include "../loader/component_loader.hpp"

#include <utils/io.hpp>
#include <utils/com.hpp>
#include <utils/finally.hpp>

#include "properties.hpp"

namespace game_path
{
    namespace
    {
        void set_directory(const std::filesystem::path& path)
        {
            const auto wide_path = path.generic_wstring();
            SetDllDirectoryW(wide_path.data());
            SetCurrentDirectoryW(wide_path.data());
        }

        std::filesystem::path get_steam_install_directory()
        {
            std::filesystem::path install_path{};

            HKEY reg_key;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\WOW6432Node\\Valve\\Steam", 0, KEY_QUERY_VALUE, &reg_key) == ERROR_SUCCESS)
            {
                wchar_t path[MAX_PATH] = {0};
                DWORD length = sizeof(path);
                RegQueryValueExW(reg_key, L"InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path), &length);
                RegCloseKey(reg_key);

                install_path = std::wstring(path, length / 2);
            }

            return install_path;
        }

        std::string get_default_witcher_path()
        {
            if (utils::nt::is_wine())
            {
                return "Z:\\home\\deck\\.local\\share\\Steam\\steamapps\\common";
            }

            const auto steam_path = get_steam_install_directory();
            if (steam_path.empty())
                return {};

            const auto witcher_path = steam_path / "steamapps/common/The Witcher 3";
            if (!std::filesystem::exists(witcher_path))
                return {};

            return witcher_path.generic_string();
        }

        bool try_set_witcher_path(const std::filesystem::path& path)
        {
            if (path.empty())
            {
                return false;
            }

            if (std::filesystem::exists(path / "witcher3.exe"))
            {
                set_directory(path);
                return true;
            }

            const auto alt_path = std::filesystem::path(path) / "bin" / "x64";
            if (std::filesystem::exists(alt_path / "witcher3.exe"))
            {
                set_directory(alt_path);
                return true;
            }

            return false;
        }

        void set_witcher_path()
        {
            std::string path;
            if (properties::get("game_path", path) && try_set_witcher_path(path))
            {
                return;
            }

            if (!utils::com::select_folder(path, "Select Witcher 3 installation path", get_default_witcher_path()))
            {
                component_loader::trigger_premature_shutdown();
            }
            else if (!try_set_witcher_path(path))
            {
                throw std::runtime_error("Witcher 3 was not found at the specified path!");
            }

            properties::set("game_path", path);
        }

        struct component final : component_interface
        {
            void post_start() override
            {
                if (!utils::io::file_exists("witcher3.exe"))
                {
                    set_witcher_path();
                }
            }
        };
    }

    std::filesystem::path get_appdata_path()
    {
        static const auto appdata_path = [] {
            PWSTR path;
            if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
            {
                throw std::runtime_error("Failed to read APPDATA path!");
            }

            auto _ = utils::finally([&path] { CoTaskMemFree(path); });

            static auto appdata = std::filesystem::path(path) / "w3m";
            return appdata;
        }();

        return appdata_path;
    }
}

REGISTER_COMPONENT(game_path::component)
