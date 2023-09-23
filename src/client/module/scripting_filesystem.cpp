#include "../std_include.hpp"

#include "../loader/component_loader.hpp"
#include "../loader/loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include "scripting.hpp"

namespace scripting_filesystem
{
	namespace
	{
		using unknown_array = scripting::array<void*>;

		void* load_mod_scripts_stub(unknown_array* a1, unknown_array* a2)
		{
			const auto res = reinterpret_cast<void* (*)(unknown_array*, unknown_array*)>(0x1402A9D50_g)(a1, a2);

			// Always say we have mods
			(*reinterpret_cast<uint8_t**>(0x144DE5FF8_g))[208] |= 1;

			return res;
		}

		std::vector<std::filesystem::path> collect_custom_scripts_files(const std::filesystem::path& base)
		{
			std::vector<std::filesystem::path> files{};

			std::error_code ec{};
			for (const auto& file : std::filesystem::recursive_directory_iterator(
				     base, std::filesystem::directory_options::skip_permission_denied, ec))
			{
				ec = {};
				if (!file.is_regular_file(ec) || ec)
				{
					continue;
				}

				const auto& path = file.path();

				if (path.extension() != ".ws")
				{
					continue;
				}

				files.push_back(path);
			}

			return files;
		}

		std::vector<std::wstring> collect_custom_scripts(const std::filesystem::path& base)
		{
			const auto files = collect_custom_scripts_files(base);

			std::vector<std::wstring> scripts{};
			scripts.reserve(files.size());

			for (const auto& file : files)
			{
				scripts.push_back(utils::string::to_lower(absolute(file).wstring()));
			}

			return scripts;
		}

		void remove_overriden_base_scripts(std::vector<scripting::string>& base_scripts,
		                                   const std::vector<std::wstring>& custom_scripts)
		{
			const std::wstring separator = L"\\scripts\\";

			for (const auto& custom_script : custom_scripts)
			{
				const auto pos = custom_script.find(separator);
				if (pos == std::wstring::npos)
				{
					continue;
				}

				const auto substring = custom_script.substr(pos + separator.size());

				for (auto i = base_scripts.begin(); i != base_scripts.end();)
				{
					if (i->to_view().ends_with(substring))
					{
						i = base_scripts.erase(i);
					}
					else
					{
						++i;
					}
				}
			}
		}

		void collect_script(void* a1, scripting::array<scripting::string>* scripts)
		{
			reinterpret_cast<void(*)(void*, scripting::array<scripting::string>*)>(0x1402A3ED0_g)(a1, scripts);

			const auto custom_scripts = collect_custom_scripts(loader::get_main_module().get_folder() / "data");
			if (custom_scripts.empty())
			{
				return;
			}

			auto base_scripts = scripts->move_to_vector();
			remove_overriden_base_scripts(base_scripts, custom_scripts);

			for (const auto& script : custom_scripts)
			{
				base_scripts.emplace_back(script);
			}

			*scripts = std::move(base_scripts);
		}

		class component final : public component_interface
		{
		public:
			void post_load() override
			{
				utils::hook::call(0x1402AB75D_g, load_mod_scripts_stub);
				utils::hook::call(0x14037213D_g, collect_script);
				utils::hook::call(0x140382F7D_g, collect_script);

				// Force CRC checks
				utils::hook::nop(0x140372FC8_g, 2);
			}
		};
	}
}

REGISTER_COMPONENT(scripting_filesystem::component)
