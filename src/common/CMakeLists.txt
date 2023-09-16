file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
)

add_library(common ${SRC_FILES})

target_link_libraries(common PUBLIC
  RapidJSON
  minhook
  asmjit
)

target_include_directories(common INTERFACE "${CMAKE_CURRENT_LIST_DIR}")