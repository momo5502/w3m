#include "../std_include.hpp"
#include "../loader/component_loader.hpp"
#include "../resource.hpp"
#include "../game_version.hpp"

#include <utils/nt.hpp>

#include <momo/html_ui.hpp>

namespace
{
    namespace launcher
    {
        bool run_launcher()
        {
            uint32_t directx_version{};

            momo::html_ui window(W3M_MODNAME, 500, 300);

            window.register_handler("launch", [&](const uint32_t version) {
                directx_version = version;
                window.close();
            });

            window.load_html(utils::nt::load_resource(MAIN_MENU));

            momo::html_ui::show_windows();

            return directx_version == 11;
        }

        struct component final : component_interface
        {
            void post_start() override
            {
                if (!utils::nt::is_wine() && !run_launcher())
                {
                    component_loader::trigger_premature_shutdown();
                }
            }

            component_priority priority() const override
            {
                return component_priority::launcher;
            }
        };
    }
}

REGISTER_COMPONENT(launcher::component)
