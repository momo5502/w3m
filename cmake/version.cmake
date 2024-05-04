include_guard()

##########################################

find_package(Git)
if(NOT Git_FOUND)
  if(DEFINED ENV{GIT_EXECUTABLE})
    set(GIT_EXECUTABLE "$ENV{GIT_EXECUTABLE}")
  else()
    message(FATAL_ERROR "Unable to find Git executable")
  endif()
endif()

##########################################

execute_process(COMMAND "${GIT_EXECUTABLE}" "describe" "--tags" "--dirty=-d" "--match=v*" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE PROJECT_VERSION
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
execute_process(COMMAND "${GIT_EXECUTABLE}" "rev-parse" "HEAD" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE PROJECT_COMMIT_HASH
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
execute_process(COMMAND "${GIT_EXECUTABLE}" "rev-parse" "--abbrev-ref" "HEAD" OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE PROJECT_COMMIT_REF
WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

##########################################

message("-- Build Version: ${PROJECT_VERSION} @ ${PROJECT_COMMIT_REF} ${PROJECT_COMMIT_HASH}")
