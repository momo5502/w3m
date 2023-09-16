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
)

set_target_properties(client PROPERTIES OUTPUT_NAME "w3m")
