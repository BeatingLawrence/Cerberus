function(cerberus_link_postgresql target)
  if(NOT POSTGRESQL)
    return()
  endif()

  find_library(PQXX_LIBRARY NAMES pqxx libpqxx)

  if(NOT PQXX_LIBRARY)
    message(FATAL_ERROR "pqxx library not found")
  endif()

  target_link_libraries(${target} PRIVATE ${PQXX_LIBRARY})
  target_compile_definitions(${target} PRIVATE POSTGRESQL_SUPPORT)
endfunction()

function(cerberus_link_boost_regex target)
  if(CERBERUS_BOOST_ROOT)
    get_filename_component(CERBERUS_BOOST_ROOT_ABS "${CERBERUS_BOOST_ROOT}" ABSOLUTE)
    set(CERBERUS_BOOST_REGEX_HEADER "${CERBERUS_BOOST_ROOT_ABS}/boost/regex.hpp")
    set(CERBERUS_BOOST_REGEX_CMAKE "${CERBERUS_BOOST_ROOT_ABS}/libs/regex/CMakeLists.txt")

    if(NOT EXISTS "${CERBERUS_BOOST_REGEX_HEADER}")
      message(FATAL_ERROR
        "CERBERUS_BOOST_ROOT is set to '${CERBERUS_BOOST_ROOT_ABS}', but boost/regex.hpp was not found"
      )
    endif()

    if(EXISTS "${CERBERUS_BOOST_REGEX_CMAKE}")
      message(STATUS "Using explicit Boost root: ${CERBERUS_BOOST_ROOT_ABS}")

      set(BOOST_SUPERPROJECT_VERSION "1.82.0")
      set(BOOST_REGEX_STANDALONE ON CACHE BOOL "Build Boost.Regex without Boost CMake dependency targets" FORCE)
      add_subdirectory(
        "${CERBERUS_BOOST_ROOT_ABS}/libs/regex"
        "${CMAKE_CURRENT_BINARY_DIR}/boost-regex"
      )

      target_include_directories(${target} PUBLIC "${CERBERUS_BOOST_ROOT_ABS}")
      target_link_libraries(${target} PUBLIC Boost::regex)
      return()
    endif()

    message(STATUS "Using explicit Boost include root: ${CERBERUS_BOOST_ROOT_ABS}")
    target_include_directories(${target} PUBLIC "${CERBERUS_BOOST_ROOT_ABS}")
    return()
  endif()

  if(POLICY CMP0167)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0167 OLD)
  endif()

  find_package(Boost QUIET COMPONENTS regex)

  if(POLICY CMP0167)
    cmake_policy(POP)
  endif()

  if(NOT Boost_FOUND)
    message(FATAL_ERROR
      "Boost.Regex was not found. Set CERBERUS_BOOST_ROOT to a Boost source/install root "
      "or install Boost.Regex for this toolchain."
    )
  endif()

  if(TARGET Boost::regex)
    target_link_libraries(${target} PUBLIC Boost::regex)
  else()
    target_include_directories(${target} PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(${target} PUBLIC ${Boost_REGEX_LIBRARY})
  endif()
endfunction()
