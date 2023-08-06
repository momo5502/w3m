#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "loader/loader.hpp"
#include "utils/hook.hpp"

namespace
{
	namespace filesystem
	{
		HANDLE __stdcall find_first_file_ex_w(const wchar_t* file_name, const FINDEX_INFO_LEVELS info_level_id,
		                                      const LPVOID find_file_data, const FINDEX_SEARCH_OPS search_op,
		                                      const LPVOID search_filter, const DWORD additional_flags)
		{
			OutputDebugStringW(file_name);
			if (file_name == L"D:\\Games\\SteamLibrary\\steamapps\\common\\The Witcher 3\\mods\\*."s)
			{
				OutputDebugStringA("");
			}

			return FindFirstFileExW(file_name, info_level_id, find_file_data, search_op, search_filter,
			                        additional_flags);
		}

		HANDLE __stdcall create_file_a(const char* filename, const DWORD desired_access,
		                               const DWORD share_mode,
		                               const LPSECURITY_ATTRIBUTES security_attributes,
		                               const DWORD creation_disposition, const DWORD flags_and_attributes,
		                               const HANDLE template_file)
		{
			//OutputDebugStringA(filename);

			return CreateFileA(filename, desired_access, share_mode, security_attributes, creation_disposition,
			                   flags_and_attributes, template_file);
		}

		HANDLE __stdcall create_file_w(const wchar_t* filename, const DWORD desired_access,
		                               const DWORD share_mode,
		                               const LPSECURITY_ATTRIBUTES security_attributes,
		                               const DWORD creation_disposition, const DWORD flags_and_attributes,
		                               const HANDLE template_file)
		{
			return CreateFileW(filename, desired_access, share_mode, security_attributes,
			                   creation_disposition,
			                   flags_and_attributes, template_file);
		}

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				const auto module = loader::get_main_module();
				const auto wide_path = module.get_folder().generic_wstring() + L"/";

				static struct game_string
				{
					const wchar_t* string;
					size_t length;
					wchar_t path[MAX_PATH];
				} game_path;

				game_path.string = game_path.path;
				game_path.length = wide_path.size() + 1;
				wcscpy_s(game_path.path, wide_path.data());

				[[maybe_unused]] const auto mods_path_stub = utils::hook::assemble([](utils::hook::assembler& a)
				{
					a.mov(rcx, size_t(&game_path));

					a.lea(rdx, ptr(rbp, -0x48, 8));
					a.mov(ptr(rsp, 0x2d8, 8), rbx);
					a.jmp(0x140049B74_g);
				});

				//utils::hook::jump(0x140049B68_g, mods_path_stub);

				[[maybe_unused]] const auto mods_path_stub_2 = utils::hook::assemble([](utils::hook::assembler& a)
				{
					a.mov(rcx, size_t(&game_path));
					a.lea(r8, ptr(rsp, 0x40, 8));
					a.jmp(0x140049BDD_g);
				});

				//utils::hook::jump(0x140049BD1_g, mods_path_stub_2);
			}

			void* load_import(const std::string& /*module*/, const std::string& function) override
			{
				if (function == "CreateFileA")
				{
					return &create_file_a;
				}

				if (function == "CreateFileW")
				{
					return &create_file_w;
				}

				if (function == "FindFirstFileExW")
				{
					return &find_first_file_ex_w;
				}

				return nullptr;
			}
		};
	}
}

//REGISTER_COMPONENT(filesystem::component)
