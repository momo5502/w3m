#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "utils/hook.hpp"

class camera final : public module
{
public:
	void post_load() override
	{
		utils::hook::set<void*>(0x1422CA508_g, &handle_debug_input);
	}

private:
	static bool handle_debug_input(void* this_ptr, void* viewport, const uint64_t input_key, const uint64_t input_action, const float tick)
	{
		const auto handle_camera_input = reinterpret_cast<bool(*)(void*, uint64_t, uint64_t, float)>(0x140115600_g);
		if(handle_camera_input(this_ptr, input_key, input_action, tick))
		{
			return true;
		}

		const auto original = reinterpret_cast<decltype(&handle_debug_input)>(0x1406ED1C0_g);
		return original(this_ptr, viewport, input_key, input_action, tick);
	}
};

REGISTER_MODULE(camera)
