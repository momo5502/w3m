include_guard()

##########################################

set(IS_CI OFF)
if(DEFINED ENV{CI} AND ("$ENV{CI}" STREQUAL "true" OR "$ENV{CI}" STREQUAL "1"))
    set(IS_CI ON)
endif()

##########################################

if (IS_CI)
  add_compile_definitions(CI=1)
endif()

##########################################

cmake_policy(SET CMP0069 NEW) 
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

##########################################

if(MSVC)
  momo_add_c_and_cxx_compile_options(
    /sdl
    /GS
    /Gy
    /guard:cf
  )

  momo_add_compile_options(CXX
    /Zc:__cplusplus
  )

  add_link_options(
    /INCREMENTAL:NO
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
