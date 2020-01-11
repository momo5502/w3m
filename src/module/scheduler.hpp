#pragma once
#include "loader/module_loader.hpp"
#include "utils/concurrent_list.hpp"

class scheduler final : public module
{	
public:
	static const bool cond_continue = false;
	static const bool cond_end = true;
	
	static void frame(const std::function<bool()>& callback);

	void post_start() override;
	void pre_destroy() override;

private:
	bool kill_ = false;
	std::thread thread_;
	
	utils::concurrent_list<std::function<bool()>> callbacks_;

	void execute();
};
