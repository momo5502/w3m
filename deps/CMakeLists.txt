momo_silence_deprecation_warnings()

##########################################

set(ASMJIT_STATIC ON)
set(ASMJIT_NO_AARCH64 ON)
set(ASMJIT_NO_FOREIGN ON)

add_subdirectory(asmjit)

##########################################

add_subdirectory(minhook)

##########################################

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE "${CMAKE_CURRENT_LIST_DIR}/rapidjson/include")

##########################################

momo_restore_deprecation_warnings()