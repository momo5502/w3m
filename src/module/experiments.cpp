#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "window.hpp"
#include "utils/string.hpp"

struct CR4Player_Vtable
{
	void* x;
};

struct CR4Player
{
	CR4Player_Vtable* vftbl;
	char pad1[0x68];
	float rotation[12]; // 3x4
	float position[3];

	void get_orientation(float* orientation)
	{
		reinterpret_cast<void(*)(float*, float*)>(0x1400F7FD0_g)(this->rotation, orientation);
	}
};

static_assert(offsetof(CR4Player, rotation) == 0x70);
static_assert(offsetof(CR4Player, position) == 0xA0);

struct CCamera // Name is probably wrong
{
	float position[4];
	float orientation[3];
	float position_2[4];
	float orientation_2[3];
};

struct CR4Game;
struct CR4Game_Vtable
{
	char pad[0x1F0];
	CR4Player*(*get_player)(CR4Game*);
};

static_assert(offsetof(CR4Game_Vtable, get_player) == 0x1F0);

struct CR4Game
{
	CR4Game_Vtable* vftbl;
	char pad2[0x116];
	char pad3;
	bool free_camera_enabled;
	char pad4[0x428];
	CCamera* camera;
};

static_assert(offsetof(CR4Game, free_camera_enabled) == 0x11F);
static_assert(offsetof(CR4Game, camera) == 0x548);

static CR4Game* get_global_game()
{
	return *(CR4Game**)0x142C5DD38_g;
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
						player->get_orientation(orientation);

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
