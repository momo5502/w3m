#include "std_include.hpp"

#pragma comment(linker, "/stack:0x2000000")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language=''\"")

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
}

// The naming of the file enforces early linking and thus
// a better placement in the tls segment
__declspec(thread) char tls_data[TLS_PAYLOAD_SIZE];