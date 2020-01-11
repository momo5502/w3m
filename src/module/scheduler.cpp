#include <std_include.hpp>
#include "scheduler.hpp"

void scheduler::frame(const std::function<bool()>& callback)
{
	auto* instance = module_loader::get<scheduler>();
	if (instance)
	{
		instance->callbacks_.add(callback);
	}
}

void scheduler::execute()
{
	for (auto callback : this->callbacks_)
	{
		const auto res = (*callback)();
		if(res == cond_end)
		{
			this->callbacks_.remove(callback);
		}
	}
}

void scheduler::post_start()
{
	this->thread_ = std::thread([this]()
	{
		while(!this->kill_)
		{
			this->execute();
			std::this_thread::sleep_for(10ms);
		}
	});
}

void scheduler::pre_destroy()
{
	this->kill_ = true;
	if(this->thread_.joinable())
	{
		this->thread_.join();
	}
}

REGISTER_MODULE(scheduler);
