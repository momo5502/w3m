file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  *.cpp
  *.hpp
)

add_library(common ${SRC_FILES})

target_link_libraries(common PUBLIC
  RapidJSON
)

if (MSVC)
  target_link_libraries(common PUBLIC
    minhook
    asmjit
  )
elseif (UNIX)
  target_compile_options(common PUBLIC
    "-pthread"
  )
endif()

target_include_directories(common INTERFACE "${CMAKE_CURRENT_LIST_DIR}")