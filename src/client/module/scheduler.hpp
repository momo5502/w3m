#pragma once

namespace scheduler
{
	constexpr bool cond_continue = false;
	constexpr bool cond_end = true;

	void frame(const std::function<bool()>& callback);
}
