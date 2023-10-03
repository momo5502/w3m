file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_executable(client WIN32 ${SRC_FILES})

target_precompile_headers(client PRIVATE std_include.hpp)

target_link_libraries(client PRIVATE
  common
  html-ui
)

set_target_properties(client PROPERTIES OUTPUT_NAME "w3m")

add_custom_command(
  TARGET client
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "${PROJECT_SOURCE_DIR}/data/" "$<TARGET_FILE_DIR:client>/data"
)

set_property(GLOBAL PROPERTY VS_STARTUP_PROJECT client)
