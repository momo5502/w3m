include_guard()

##########################################

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

##########################################

if(MSVC)
  momo_add_c_and_cxx_compile_options(
    /sdl
    /GS
    /guard:cf
  )

  momo_add_compile_options(CXX
    /Zc:__cplusplus
  )
endif()

##########################################

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

##########################################

if(MSVC)
  add_link_options(
    $<$<NOT:$<STREQUAL:${CMAKE_MSVC_RUNTIME_LIBRARY},MultiThreaded>>:/NODEFAULTLIB:libcmt.lib>
    $<$<NOT:$<STREQUAL:${CMAKE_MSVC_RUNTIME_LIBRARY},MultiThreadedDLL>>:/NODEFAULTLIB:msvcrt.lib>
    $<$<NOT:$<STREQUAL:${CMAKE_MSVC_RUNTIME_LIBRARY},MultiThreadedDebug>>:/NODEFAULTLIB:libcmtd.lib>
    $<$<NOT:$<STREQUAL:${CMAKE_MSVC_RUNTIME_LIBRARY},MultiThreadedDebugDLL>>:/NODEFAULTLIB:msvcrtd.lib>
  )
endif()

##########################################

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  momo_add_c_and_cxx_compile_options(/MP)
endif()
