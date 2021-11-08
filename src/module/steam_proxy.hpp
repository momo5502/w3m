#pragma once
#include "loader/module_loader.hpp"
#include "steam/interface.hpp"
#include "utils/nt.hpp"

class steam_proxy final : public module
{
public:
	steam_proxy();

	void post_load() override;
	void pre_destroy() override;

	static std::filesystem::path get_steam_install_directory();

private:
	utils::nt::library steam_client_module_{};
	utils::nt::library steam_overlay_module_{};

	steam::interface client_engine_ {};
	steam::interface client_user_ {};
	steam::interface client_utils_ {};

	void* steam_pipe_ = nullptr;
	void* global_user_ = nullptr;

	void run_mod() const;
	void* load_client_engine() const;
	void load_client();
	void start_mod(const std::string& title, size_t app_id);
	void clean_up_on_error();
};
