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

if(NOT CMAKE_GENERATOR MATCHES "Visual Studio|Xcode|Ninja Multi-Config" AND
   (NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL ""))
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
project(Cerberus LANGUAGES CXX VERSION 2.0.10)
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
set(CERBERUS_BOOST_ROOT "" CACHE PATH "Explicit Boost source/install root. Takes precedence over system Boost lookup.")
set(CERBERUS_OPENSSL_ROOT "" CACHE PATH "Explicit OpenSSL source/install root. Takes precedence over system lookup.")

set(CMAKE_DEBUG_POSTFIX -d)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CerberusDependencies)
include(CerberusPlatform)
include(CerberusTests)
include(CerberusDocs)

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

target_compile_options(Cerberus PRIVATE
  $<$<AND:$<CONFIG:Release>,$<NOT:$<CXX_COMPILER_ID:MSVC>>>:-O3>
)

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

cerberus_link_postgresql(Cerberus)
cerberus_configure_platform(Cerberus)
cerberus_link_boost_regex(Cerberus)
cerberus_add_tests(Cerberus)
cerberus_add_docs()
