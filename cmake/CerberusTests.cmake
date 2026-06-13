include(FetchContent)

function(cerberus_add_tests library_target)
  if(NOT GOOGLETEST)
    return()
  endif()

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
  )

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
  if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()

  FetchContent_MakeAvailable(googletest)

  include(CTest)
  enable_testing()

  file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h"
  )

  add_executable(cerberus-test ${test_sources})
  target_link_libraries(cerberus-test PRIVATE ${library_target} gtest_main)
  target_include_directories(cerberus-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

  if(MSVC)
    set_target_properties(cerberus-test PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
  endif()

  include(GoogleTest)
  gtest_discover_tests(cerberus-test DISCOVERY_MODE PRE_TEST)
endfunction()
