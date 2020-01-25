#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "utils/hook.hpp"

class camera final : public module
{
public:
	void post_load() override
	{
		const auto handle_input = utils::hook::signature("48 83 EC 38 80 3D ? ? ? ? ? 74 3D").process().get(0);
		handle_input_hook = utils::hook::detour(handle_input, &handle_debug_input);
	}

private:
	static utils::hook::detour handle_input_hook;

	static bool handle_debug_input(void* this_ptr, void* viewport, const uint64_t input_key, const uint64_t input_action, const float tick)
	{
		static const auto handle_camera_input = utils::hook::signature("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 80 3D ? ? ? ? ?").process().get(0);
		if(utils::hook::invoke<bool>(handle_camera_input, this_ptr, input_key, input_action, tick))
		{
			return true;
		}

		return handle_input_hook.invoke<bool>(this_ptr, viewport, input_key, input_action, tick);
	}
};

utils::hook::detour camera::handle_input_hook;

REGISTER_MODULE(camera)
