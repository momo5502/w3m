file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
  *.rc
)

list(SORT SRC_FILES)

add_executable(server ${SRC_FILES})

momo_assign_source_group(${SRC_FILES})

target_precompile_headers(server PRIVATE std_include.hpp)

target_link_libraries(server PRIVATE
  common
)
