# All required variables are to be set by version.cmake
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h ESCAPE_QUOTES)

add_library(version INTERFACE)

target_include_directories(version INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
