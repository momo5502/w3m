#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "utils/hook.hpp"

namespace
{
	namespace camera
	{
		utils::hook::detour handle_input_hook;

		bool handle_debug_input(void* this_ptr, void* viewport, const uint64_t input_key,
		                        const uint64_t input_action, const float tick)
		{
			static const auto handle_camera_input = "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 80 3D ? ? ? ? ?"_sig.
				get(0);
			if (utils::hook::invoke<bool>(handle_camera_input, this_ptr, input_key, input_action, tick))
			{
				return true;
			}

			return handle_input_hook.invoke<bool>(this_ptr, viewport, input_key, input_action, tick);
		}

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				const auto handle_input = "48 83 EC 38 80 3D ? ? ? ? ? 74 3D"_sig.get(0);
				handle_input_hook = utils::hook::detour(handle_input, &handle_debug_input);
			}
		};
	}
}

REGISTER_COMPONENT(camera::component)