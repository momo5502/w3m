#include "../std_include.hpp"
#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include "renderer.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/concurrency.hpp>

#include "scripting.hpp"

namespace renderer
{
    namespace
    {
        struct text_command
        {
            std::string text{};
            position position{};
            color color{};
        };

        struct CDebugConsole;
        struct CRenderFrame;

        using command_queue = std::queue<text_command>;
        utils::concurrency::container<command_queue> render_commands{};

        void render_text(CRenderFrame* frame, float x, float y, const scripting::string& text, const color& color)
        {
            auto* console = *reinterpret_cast<CDebugConsole**>(0x14532DFE0_g);
            reinterpret_cast<void (*)(CDebugConsole*, CRenderFrame*, float, float, const scripting::string&, uint32_t)>(0x14156FB20_g)(
                console, frame, x, y, text, *reinterpret_cast<const uint32_t*>(&color.r));
        }

        void renderer_stub(CRenderFrame* frame)
        {
            if (!frame)
            {
                return;
            }

            scheduler::execute(scheduler::renderer);

            command_queue queue{};

            render_commands.access([&queue](command_queue& commands) {
                if (!commands.empty())
                {
                    commands.swap(queue);
                }
            });

            while (!queue.empty())
            {
                auto& command = queue.front();

                render_text(frame, command.position.x, command.position.y, {command.text}, command.color);

                queue.pop();
            }
        }

        struct component final : component_interface
        {
            void post_load() override
            {
                utils::hook::jump(0x141565977_g, utils::hook::assemble([](utils::hook::assembler& a) {
                                      a.call(0x141571280_g);
                                      a.pushaq();
                                      a.mov(rcx, rbx);
                                      a.call_aligned(renderer_stub);
                                      a.popaq();
                                      a.jmp(0x14156597C_g);
                                  }));
            }
        };
    }

    void draw_text(std::string text, const position position, const color color)
    {
        text_command command{};
        command.text = std::move(text);
        command.position = position;
        command.color = color;

        render_commands.access([&command](command_queue& commands) { commands.emplace(std::move(command)); });
    }
}

REGISTER_COMPONENT(renderer::component)
