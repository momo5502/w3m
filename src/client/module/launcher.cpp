#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <momo/html_ui.hpp>

namespace
{
	namespace launcher
	{
		bool run_launcher()
		{
			uint32_t directx_version{};

			momo::html_ui window("Witcher 3: Online", 500, 300);

			window.register_handler("launch", [&](const uint32_t version)
			{
				directx_version = version;
				window.close();
			});

			window.load_html(R"code(
<!DOCTYPE html>
<html>

<head>
    <style>
        html {
            height: 100%;
            background: linear-gradient(135deg, #FFFFFF, #01CDFF);
        }

        * {
            font-family: -apple-system, BlinkMacSystemFont, segoe ui, Roboto, Oxygen, Ubuntu, Cantarell, open sans, helvetica neue, sans-serif;
        }

        h1 {
            color: rgb(57, 57, 57);
        }

        button {
            margin: 5px;
            display: inline-block;
            outline: none;
            cursor: pointer;
            font-size: 14px;
            line-height: 1;
            border-radius: 10px;
            transition-property: background-color, border-color, color, box-shadow, filter;
            transition-duration: .3s;
            border: 1px solid transparent;
            letter-spacing: 2px;
            min-width: 160px;
            text-transform: uppercase;
            white-space: normal;
            font-weight: 700;
            text-align: center;
            padding: 16px 14px 18px;
            color: rgb(57, 57, 57);
            border: 2px solid rgb(57, 57, 57);
            background-color: transparent;
            height: 48px;
        }

        button:hover,
        button.active {
            color: #fff;
            background-color: rgb(57, 57, 57);
        }

		button[disabled],
		button[disabled]:hover {
			cursor: default;
			opacity: 0.5;
			color: rgb(57, 57, 57);
			background-color: transparent;
		}
    </style>
</head>

<body>
    <center>
        <h1>Witcher 3: Online</h1>
        <br>
        <button onclick="window.external.launch(11)">DirectX 11</button>
		<button onclick="window.external.launch(12)" disabled>DirectX 12</button>
    </center>
</body>

</html>
)code");

			momo::html_ui::show_windows();

			return directx_version == 11;
		}

		struct component final : component_interface
		{
			void post_start() override
			{
				if (!run_launcher())
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
