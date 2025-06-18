#include "../std_include.hpp"

#include "../loader/component_loader.hpp"

#include <utils/hook.hpp>

namespace
{
    namespace game_module
    {
        utils::hook::detour file_name_a_hook;
        utils::hook::detour file_name_w_hook;

        DWORD __stdcall get_module_file_name_a(HMODULE module, const LPSTR filename, const DWORD size)
        {
            if (!module)
            {
                module = GetModuleHandleA(nullptr);
            }

            return file_name_a_hook.invoke<DWORD>(module, filename, size);
        }

        DWORD __stdcall get_module_file_name_w(HMODULE module, const LPWSTR filename, const DWORD size)
        {
            if (!module)
            {
                module = GetModuleHandleA(nullptr);
            }

            return file_name_w_hook.invoke<DWORD>(module, filename, size);
        }

        class component final : public component_interface
        {
          public:
            void post_start() override
            {
                file_name_a_hook.create(&GetModuleFileNameA, &get_module_file_name_a);
                file_name_w_hook.create(&GetModuleFileNameW, &get_module_file_name_w);
            }
        };
    }
}

REGISTER_COMPONENT(game_module::component)
