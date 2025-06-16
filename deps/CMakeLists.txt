momo_silence_deprecation_warnings()

##########################################

option(BUILD_TESTING "" OFF)

##########################################

if (MSVC)

set(ASMJIT_STATIC ON)
set(ASMJIT_NO_AARCH64 ON)
set(ASMJIT_NO_FOREIGN ON)
set(ASMJIT_NO_CUSTOM_FLAGS ON)

add_subdirectory(asmjit)

##########################################

option(HTTP_ONLY "" ON)
option(BUILD_CURL_EXE "" OFF)
option(BUILD_SHARED_LIBS "" OFF)
option(BUILD_STATIC_LIBS "" ON)
option(BUILD_EXAMPLES "" OFF)
option(BUILD_TESTING "" OFF)

option(CURL_ENABLE_EXPORT_TARGET "" OFF)

option(CURL_DISABLE_DICT "" ON)
option(CURL_DISABLE_FILE "" ON)
option(CURL_DISABLE_LDAP "" ON)
option(CURL_DISABLE_LDAPS "" ON)
option(CURL_DISABLE_FTP "" ON)
option(CURL_DISABLE_GOPHER "" ON)
option(CURL_DISABLE_IMAP "" ON)
option(CURL_DISABLE_MQTT "" ON)
option(CURL_DISABLE_POP3 "" ON)
option(CURL_DISABLE_RTSP "" ON)
option(CURL_DISABLE_SMTP "" ON)
option(CURL_DISABLE_SMB "" ON)
option(CURL_DISABLE_TELNET "" ON)
option(CURL_DISABLE_TFTP "" ON)

option(CURL_USE_LIBPSL "" OFF)
option(CURL_USE_LIBSSH2 "" OFF)
option(USE_NGHTTP2 "" OFF)
option(USE_LIBIDN2 "" OFF)

set(CURL_ZLIB "OFF" CACHE STRING "")
set(CURL_ZSTD "OFF" CACHE STRING "")
set(CURL_BROTLI "OFF" CACHE STRING "")

if(MSVC)
  set(CURL_USE_SCHANNEL ON)
endif()

add_subdirectory(curl)

##########################################

add_subdirectory(minhook)
add_subdirectory(html-ui)

##########################################

include(udis86.cmake)

endif()

##########################################

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE "${CMAKE_CURRENT_LIST_DIR}/rapidjson/include")

##########################################

set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZLIB_BUILD_MINIZIP OFF CACHE BOOL "" FORCE)
set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(zlib)
target_compile_definitions(zlibstatic PUBLIC ZLIB_CONST=1)
#target_include_directories(zlibstatic PUBLIC ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})

if (TARGET zlib)
    set_target_properties(zlib PROPERTIES EXCLUDE_FROM_ALL TRUE)
    set_target_properties(zlib PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif()

##########################################

add_subdirectory(libtommath)

if (MSVC)
    target_compile_options(libtommath PRIVATE "/wd4146")
endif()

##########################################

option(WITH_LTM "" OFF)

add_subdirectory(libtomcrypt)

target_compile_definitions(libtomcrypt PUBLIC LTM_DESC)
target_compile_definitions(libtomcrypt PUBLIC USE_LTM)
target_link_libraries(libtomcrypt PUBLIC libtommath)

if (MSVC)
    target_compile_options(libtomcrypt PRIVATE "/wd4703")
endif()

##########################################

momo_restore_deprecation_warnings()
