add_subdirectory(common)
add_subdirectory(server)

if (MSVC)
  add_subdirectory(client)
endif()
