cmake_minimum_required(VERSION 3.26.4)

##########################################

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

##########################################

project(w3m LANGUAGES C CXX)

##########################################

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##########################################

include(cmake/version.cmake)
include(cmake/utils.cmake)
include(cmake/compiler-env.cmake)

##########################################

momo_set_new_artifact_directory()

##########################################

momo_add_subdirectory_and_get_targets("deps" EXTERNAL_TARGETS)
momo_add_subdirectory_and_get_targets("src" OWN_TARGETS)

##########################################

momo_targets_set_folder("External Dependencies" ${EXTERNAL_TARGETS})

momo_targets_exclude_from_all(${EXTERNAL_TARGETS})
momo_targets_disable_warnings(${EXTERNAL_TARGETS})

momo_targets_expose_includes(${OWN_TARGETS})
momo_targets_set_warnings_as_errors(${OWN_TARGETS})
