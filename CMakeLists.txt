cmake_minimum_required(VERSION 3.14)

set(CERBERUS_USE_REMOTE_ARM64_TOOLCHAIN_DEFAULT OFF)
if(CMAKE_CURRENT_BINARY_DIR MATCHES "remote[_-]arm64")
  set(CERBERUS_USE_REMOTE_ARM64_TOOLCHAIN_DEFAULT ON)
endif()

option(
  CERBERUS_USE_REMOTE_ARM64_TOOLCHAIN
  "Use the bundled remote ARM64 toolchain for cross-compilation"
  ${CERBERUS_USE_REMOTE_ARM64_TOOLCHAIN_DEFAULT}
)

if(CERBERUS_USE_REMOTE_ARM64_TOOLCHAIN)
  set(
    CMAKE_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains/remote-arm64.cmake"
    CACHE FILEPATH
    "Toolchain file for remote ARM64 cross-compilation"
    FORCE
  )
endif()

if(NOT DEFINED CMAKE_CONFIGURATION_TYPES AND (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL ""))
  message(WARNING "CMAKE_BUILD_TYPE is empty for a single-config generator. Falling back to Debug.")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

#[[ Cerberus versioning system:

    MAJOR.MINOR.PATCH.TYPE

    MAJOR:  Substantial breaking changes
    MINOR:  Substantial non-breaking changes
    PATCH:  Bug fixes, optimization changes
    TYPE:   Version type. This number is mapped to the
            following values when the Cerberus::cerberusVersion()
            function is called:
            0:  Alpha
            1:  Beta
            99: Release
            Using a different number will cause a build failure

            Code cleaning must not change the version in any of its numbers

                                        Version
                                          ||
#]]#                                      V
project(Cerberus LANGUAGES CXX VERSION 2.0.0.1)
message(STATUS ${CMAKE_PROJECT_VERSION})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(POSTGRESQL "postgresql backend support" OFF)
option(GOOGLETEST "googletest support" OFF)
option(CERBERUS_SYNC_REMOTE_SYSROOT "Synchronize the configured remote sysroot via SSH during configure when using the remote toolchain" OFF)

set(CERBERUS_REMOTE_SSH_TARGET "" CACHE STRING "SSH target used to refresh the remote sysroot, for example pi@target-host")
set(CERBERUS_SYSROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sysroot" CACHE PATH "Local ARM64 sysroot directory used for cross-compilation")
set(CERBERUS_REMOTE_EXTRA_SYNC_PATHS "" CACHE STRING "Semicolon-separated extra remote paths to synchronize into the remote sysroot")

set(CMAKE_DEBUG_POSTFIX -d)

if(CMAKE_CROSSCOMPILING)
  message(STATUS "Cross-compiling Cerberus for ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR} with sysroot: ${CMAKE_SYSROOT}")
elseif(CERBERUS_SYNC_REMOTE_SYSROOT)
  message(WARNING "CERBERUS_SYNC_REMOTE_SYSROOT is ON, but this configure is not using the remote toolchain. The sync step only runs when cmake/toolchains/remote-arm64.cmake is active.")
endif()

if(CMAKE_CROSSCOMPILING AND DEFINED CERBERUS_REMOTE_TOOLCHAIN_READY AND NOT CERBERUS_REMOTE_TOOLCHAIN_READY)
  message(STATUS
    "Remote ARM64 configure is in bootstrap mode because ${CERBERUS_SYSROOT_DIR} is not populated yet.\n"
    "Set CERBERUS_REMOTE_SSH_TARGET and reconfigure, or populate CERBERUS_SYSROOT_DIR manually."
  )
  return()
endif()

# =======================Cerberus library=========================

file(GLOB_RECURSE cerb_sources CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
)

add_library(Cerberus SHARED EXCLUDE_FROM_ALL
  ${cerb_sources}
)

target_compile_options(Cerberus PRIVATE $<$<CONFIG:Release>:-O3>)

set_property(   #delete install directory on clean
        TARGET Cerberus
        APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_INSTALL_PREFIX}
)

set_target_properties(Cerberus PROPERTIES PREFIX "lib" IMPORT_PREFIX "lib")

target_compile_definitions(Cerberus
    PRIVATE CERBERUS_LIBRARY
    PRIVATE _CRT_SECURE_NO_WARNINGS
    PRIVATE _LARGEFILE64_SOURCE
    PRIVATE CERBERUS_VERSION="${CMAKE_PROJECT_VERSION}"
    PRIVATE CERBERUS_VERSION_TWEAK=${CMAKE_PROJECT_VERSION_TWEAK}
)

install(TARGETS Cerberus DESTINATION lib)
install(DIRECTORY src/ DESTINATION include/cerberus FILES_MATCHING PATTERN "*.h")

#postgres support switch
if(POSTGRESQL)
    find_library(PQXX_LIBRARY
        NAMES pqxx libpqxx
    )

    if(NOT PQXX_LIBRARY)
        message(FATAL_ERROR "pqxx library not found")
    endif()

    target_link_libraries(Cerberus PRIVATE ${PQXX_LIBRARY})
    target_compile_definitions(Cerberus PRIVATE POSTGRESQL_SUPPORT)
endif()

if(MSVC)

    message(STATUS "On Windows system")

    set_target_properties(Cerberus test PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
    )

    target_compile_definitions(Cerberus PRIVATE WINDOWS_SYSTEM)

    #OPEN-SSL
    target_include_directories(Cerberus PUBLIC "C:/Qt/Tools/OpenSSL/Win_x64/include")
    target_link_directories(Cerberus PUBLIC "C:/Qt/Tools/OpenSSL/Win_x64/lib")
    target_link_libraries(Cerberus LINK_PUBLIC libssl)
    target_link_libraries(Cerberus LINK_PUBLIC libcrypto)

elseif(UNIX)

    message(STATUS "On Linux/MACOSX system")

    if(APPLE)
        target_compile_definitions(Cerberus PRIVATE APPLE_SYSTEM)
    else()
        target_compile_definitions(Cerberus PRIVATE LINUX_SYSTEM)
    endif()

    # --- Required system libs on Linux (pthread, dl, timer_*) ---
    if(NOT APPLE)
        find_package(Threads REQUIRED)
        target_link_libraries(Cerberus PRIVATE Threads::Threads ${CMAKE_DL_LIBS} rt)
    endif()

    # --- OPENSSL (no hardcoded paths) ---
    find_package(OpenSSL REQUIRED)
    target_link_libraries(Cerberus PRIVATE OpenSSL::SSL OpenSSL::Crypto)

endif()

include(FetchContent)

#googletest support switch
if(GOOGLETEST)
  #automatically download and compile gtest library if not already installed in the system
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    # Prevent system gtest installation
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    enable_testing()

    file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS
        "test/*.cpp"
        "test/*.h"
    )

    add_executable(cerberus-test ${test_sources})
    target_link_libraries(cerberus-test PRIVATE Cerberus gtest_main)
    target_include_directories(cerberus-test PRIVATE "src/")

    include(GoogleTest)
    gtest_discover_tests(cerberus-test)
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost QUIET COMPONENTS regex CONFIG)
if(NOT Boost_FOUND)
  find_package(Boost REQUIRED COMPONENTS regex)  # old find boost
endif()

if(TARGET Boost::regex)
  target_link_libraries(Cerberus PUBLIC Boost::regex)
else()
  target_include_directories(Cerberus PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(Cerberus PUBLIC ${Boost_REGEX_LIBRARY})
endif()

if(UNIX AND NOT APPLE)
  target_link_options(Cerberus PRIVATE -Wl,--no-as-needed -Wl,-z,defs -Wl,--no-undefined)
endif()

# ======================= DOCUMENTATION =======================
find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
    message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

    include(FetchContent)

    FetchContent_Declare(
        doxygen-awesome-css
        URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(doxygen-awesome-css)

    FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

    set(DOXYFILE_IN  "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
    set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
    configure_file("${DOXYFILE_IN}" "${DOXYFILE_OUT}" @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} "${DOXYFILE_OUT}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Generating Cerberus HTML/LaTeX documentation with doxygen-awesome-css"
        VERBATIM
    )

else()
    message(STATUS "Doxygen not found. 'doc' target not available")
endif()
