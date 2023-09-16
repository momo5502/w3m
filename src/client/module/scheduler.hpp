#pragma once

namespace scheduler
{
	const bool cond_continue = false;
	const bool cond_end = true;

	void frame(const std::function<bool()>& callback);
}
