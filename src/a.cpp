#include "std_include.hpp"

// The naming of the file enforces early linking and thus
// a better placement in the tls segment
__declspec(thread) char tls_data[TLS_PAYLOAD_SIZE];