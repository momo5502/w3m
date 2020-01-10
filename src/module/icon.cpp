#include <std_include.hpp>
#include "loader/module_loader.hpp"
#include "loader/loader.hpp"
#include "utils/nt.hpp"

class icon final : public module
{
public:
	void* load_import(const std::string& module, const std::string& function) override
	{
		if (function == "LoadIconW")
		{
			return &load_icon_w;
		}

		return nullptr;
	}

private:
	static HICON __stdcall load_icon_w(HINSTANCE instance, const wchar_t* name)
	{
		if (!name || size_t(name) >= 100 && size_t(name) <= 110 && instance == loader::get_module())
		{
			return LoadIconA(GetModuleHandleA(nullptr), MAKEINTRESOURCE(102));
		}

		return LoadIconW(instance, name);
	}
};

REGISTER_MODULE(icon)
