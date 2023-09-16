#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include "scheduler.hpp"

#include "utils/concurrent_list.hpp"

namespace scheduler
{
	namespace
	{
		std::atomic_bool kill{false};
		std::thread thread;
		utils::concurrent_list<std::function<bool()>> callbacks;

		void execute()
		{
			for (auto callback : callbacks)
			{
				const auto res = (*callback)();
				if (res == cond_end)
				{
					callbacks.remove(callback);
				}
			}
		}

		class component final : public component_interface
		{
		public:
			~component() override
			{
				if (thread.joinable())
				{
					thread.detach();
				}
			}

			void post_start() override
			{
				thread = std::thread([this]()
				{
					while (!kill)
					{
						execute();
						std::this_thread::sleep_for(10ms);
					}
				});
			}

			void pre_destroy() override
			{
				kill = true;
				if (thread.joinable())
				{
					thread.join();
				}
			}
		};
	}

	void frame(const std::function<bool()>& callback)
	{
		callbacks.add(callback);
	}
}

REGISTER_COMPONENT(scheduler::component)
