#include <std_include.hpp>
#include "loader/loader.hpp"

int main()
{
	FARPROC entry_point;
	
	try
	{
		const auto module = loader::load("witcher3.exe");
		entry_point = FARPROC(module.get_entry_point());
	}
	catch (std::exception & e)
	{
		MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
		return 1;
	}

	if(!entry_point) {
		return 1;
	}
	
	return int(entry_point());
}
