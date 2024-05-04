file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/udis86/libudis86/*.c
    ${CMAKE_CURRENT_LIST_DIR}/extra/udis86/libudis86/*.c
)

add_library(udis86 ${SRC_FILES})

target_include_directories(udis86 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/udis86")
target_include_directories(udis86 INTERFACE "${CMAKE_CURRENT_LIST_DIR}/extra/udis86")

target_include_directories(udis86 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/udis86/libudis86")
target_include_directories(udis86 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/extra/udis86/libudis86")