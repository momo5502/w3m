momo_silence_deprecation_warnings()

##########################################

if (MSVC)

set(ASMJIT_STATIC ON)
set(ASMJIT_NO_AARCH64 ON)
set(ASMJIT_NO_FOREIGN ON)
set(ASMJIT_NO_CUSTOM_FLAGS ON)

add_subdirectory(asmjit)

##########################################

add_subdirectory(minhook)
add_subdirectory(html-ui)

endif()

##########################################

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE "${CMAKE_CURRENT_LIST_DIR}/rapidjson/include")

##########################################

momo_restore_deprecation_warnings()