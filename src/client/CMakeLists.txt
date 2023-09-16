file(GLOB_RECURSE SRC_FILES 
  *.cpp
  *.hpp
  *.rc
)

add_executable(w3m WIN32 ${SRC_FILES})

target_precompile_headers(w3m PRIVATE std_include.hpp)

target_link_libraries(w3m PRIVATE
  common
)
