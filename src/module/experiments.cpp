#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "window.hpp"
#include "utils/string.hpp"
#include "utils/hook.hpp"

template<typename T>
struct rva
{
	uint32_t value;

	using type = T*;
	type operator->() const
	{
		const auto addr =  loader::get_game_module().get_ptr() + this->value;
		return type(addr);
	}
};

static_assert(sizeof(rva<void>) == 4);

struct rtti_type_descriptor
{
	void* typeInfo;
	size_t runtime_reference;
	char name[1];
};

struct rtti_object_locator
{
	uint32_t signature;
	uint32_t offset_in_class;
	uint32_t offset_of_constructor;
	rva<rtti_type_descriptor> type_description;
	rva<void> hierarchy_description;
	rva<void> object_base;
};

rtti_object_locator* get_rtti(void* object)
{
	auto* vtable = *reinterpret_cast<void**>(object);
	return reinterpret_cast<rtti_object_locator**>(vtable)[-1];
}

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
	return *reinterpret_cast<CR4Game**>(0x142C5DD38_g);
}

struct IDK
{
	void* viewport;
};

struct CRendererInterface
{
	void* vftbl;
	IDK* idk;
};

struct StrInfo
{
	void* idk;
	size_t strlen;
	void* idk2;
	size_t strlen2;
};

struct SubTextRenderStruct
{
	const wchar_t* text;
	size_t length;
	size_t length2;
	void* idk;
	StrInfo* info;
};

struct SomeTextRenderStruct
{
	char pad[0x30];
	SubTextRenderStruct* sub_struct;
};

utils::hook::detour register_script_hook;
utils::hook::detour render_text_hook;

void* render_text_function(SomeTextRenderStruct* r, const size_t idk)
{
	auto old = r->sub_struct->text;
	/*auto old_len = r->sub_struct->length;
	auto old_len2 = r->sub_struct->length2;
	auto old_info = *r->sub_struct->info;*/

	r->sub_struct->text = L"LUUUL";
	/*r->sub_struct->info->strlen = wcslen(r->sub_struct->text);
	r->sub_struct->info->strlen2 = r->sub_struct->info->strlen;
	r->sub_struct->length = r->sub_struct->info->strlen + 1;
	r->sub_struct->length2 = r->sub_struct->length;*/
	
	auto _ = gsl::finally([&]()
	{
		r->sub_struct->text = old;
		/*r->sub_struct->length = old_len;
		r->sub_struct->length2 = old_len2;
		*r->sub_struct->info = old_info;*/
	});
	
	return render_text_hook.invoke<void*>(r, idk);
}

void register_script_function(const size_t idk, const wchar_t* function)
{
	//OutputDebugStringW(function);
	//OutputDebugStringA("\n");
	register_script_hook.invoke<void>(idk, function);
}

void execute_command(const std::string& command)
{
	auto* renderer = *reinterpret_cast<CRendererInterface**>(0x142BCBB48_g);
	const auto viewport = renderer->idk->viewport;

	auto* console = *reinterpret_cast<void**>(0x142BD98C8_g);
	const auto handler = 0x140243A20_g;

	const auto in = [&](const uint64_t chr)
	{
		utils::hook::invoke<bool>(handler, console, viewport, 1, chr, 0.0f);
		utils::hook::invoke<bool>(handler, console, viewport, 2, chr, 0.0f);
	};

	in(192);

	/*
	for (const auto& chr : command)
	{
		in(chr);
	}

	in(13);
	*/
}

class experiments final : public module
{
public:
	void post_load() override
	{
		register_script_hook = utils::hook::detour(0x140030C50_g, &register_script_function);
		render_text_hook = utils::hook::detour(0x141394F90_g, &render_text_function);

		std::thread([]()
		{
			while (true)
			{
				const auto game = get_global_game();
				if (game && game->vftbl)
				{
					/*
					const auto game_rtti = get_rtti(game);
					const auto type_desc = game_rtti->type_description;
					OutputDebugStringA(type_desc->name);
					*/

					const auto player = game->vftbl->get_player(game);
					if (player)
					{
						float orientation[3];
						player->get_orientation(orientation);

						const auto title = utils::string::va("Position: %.2f %.2f %.2f | Orientation: %.2f %.2f %.2f",
							player->position[0], player->position[1], player->position[2],
							orientation[0], orientation[1], orientation[2]);
						SetWindowTextA(window::get_game_window(), title);

						/*MessageBoxA(0, 0, 0, 0);
						execute_command("spawn('witcher', 1)");*/
					}
				}

				std::this_thread::sleep_for(100ms);
			}
		}).detach();
	}
};

//REGISTER_MODULE(experiments)
