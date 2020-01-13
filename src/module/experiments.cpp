#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "window.hpp"
#include "utils/string.hpp"

struct CPlayer
{
	char pad1[0x70];
	float rotation[12]; // 3x4
	float position[3];
};

static_assert(offsetof(CPlayer, position) == 0xA0);

struct CGameVtable
{
	char pad[0x1F0];
	CPlayer*(*get_player)(void*);
};

static_assert(offsetof(CGameVtable, get_player) == 0x1F0);

struct CGame
{
	CGameVtable* vftbl;
	char pad2[0x116];
	char pad3;
	bool free_camera_enabled;
};

static_assert(offsetof(CGame, free_camera_enabled) == 0x11F);

static CGame* get_global_game()
{
	return *(CGame**)0x142C5DD38_g;
}

static void get_player_orientation(CPlayer* player, float* orientation)
{
	reinterpret_cast<void(*)(float*, float*)>(0x1400F7FD0_g)(player->rotation, orientation);
}

class experiments final : public module
{
public:
	void post_load() override
	{
		std::thread([]()
		{
			while (true)
			{
				const auto game = get_global_game();
				if (game)
				{
					const auto player = game->vftbl->get_player(game);
					if (player)
					{
						float orientation[3];
						get_player_orientation(player, orientation);

						const auto title = utils::string::va("Position: %.2f %.2f %.2f | Orientation: %.2f %.2f %.2f",
							player->position[0], player->position[1], player->position[2],
							orientation[0], orientation[1], orientation[2]);
						SetWindowTextA(window::get_game_window(), title);
					}
				}

				std::this_thread::sleep_for(100ms);
			}
		}).detach();
	}
};

//REGISTER_MODULE(experiments)
