cmake_minimum_required(VERSION 3.14)

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
option(POSTGRESQL "postgresql backend support")
option(GOOGLETEST "googletest support")

set(CMAKE_DEBUG_POSTFIX -d)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "../dist" CACHE PATH "framework build output directory" FORCE)
endif()

# =======================Cerberus library=========================

file(GLOB_RECURSE cerb_sources CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.h"
)

add_library(Cerberus SHARED EXCLUDE_FROM_ALL
  ${cerb_sources}
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

find_package(Boost 1.70 REQUIRED COMPONENTS regex CONFIG)

# =======================test executable==========================

file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS "test/*")
add_executable(test EXCLUDE_FROM_ALL ${test_sources})

target_include_directories(test PRIVATE "src/")
target_link_libraries(test LINK_PRIVATE Cerberus)

# ================================================================

if(MSVC)

    message(STATUS "On Windows system")

    set_target_properties(Cerberus test PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
    )

    target_compile_definitions(Cerberus PRIVATE WINDOWS_SYSTEM)

    if(GOOGLETEST)
        target_include_directories(test PUBLIC "./gtest/include")

        find_library(GTEST_LIBRARY NAMES gtest PATHS "./gtest/lib" PATH_SUFFIXES lib)
        find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "./gtest/lib" PATH_SUFFIXES lib)

        if((GTEST_LIBRARY STREQUAL "GTEST_LIBRARY-NOTFOUND") OR (GTEST_MAIN_LIBRARY STREQUAL "GTEST_MAIN_LIBRARY-NOTFOUND"))
            message(FATAL_ERROR "Pre-compiled GTest library needed on Windows system! Please Read README.md")
        endif()
    endif()

    #postgres support
    if(POSTGRESQL)
        target_link_libraries(Cerberus LINK_PRIVATE libpqxx)
    endif()

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

    target_include_directories(Cerberus PUBLIC
        "/usr/include"          #for Linux
        "/opt/homebrew/include" #for MACOS
    )

    target_link_directories(Cerberus PUBLIC
        "/usr/lib"              #for Linux
        "/opt/homebrew/lib"     #for MACOS
    )

    #postgres support
    if(POSTGRESQL)
        target_link_libraries(Cerberus LINK_PRIVATE pqxx)
        target_compile_definitions(Cerberus PRIVATE POSTGRESQL_SUPPORT)
    endif()

    #OPEN-SSL
    target_link_libraries(Cerberus LINK_PRIVATE ssl crypto)

    if(GOOGLETEST)
        find_library(GTEST_LIBRARY NAMES gtest PATHS "/usr/lib" "/opt/homebrew/lib" PATH_SUFFIXES lib)
        find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "/usr/lib" "/opt/homebrew/lib" PATH_SUFFIXES lib)

        if((GTEST_LIBRARY STREQUAL "GTEST_LIBRARY-NOTFOUND") OR (GTEST_MAIN_LIBRARY STREQUAL "GTEST_MAIN_LIBRARY-NOTFOUND"))
            message(FATAL_ERROR "GTest library not found!")
        endif()
    endif()
endif()

if(GOOGLETEST)
    target_link_libraries(test LINK_PRIVATE ${GTEST_LIBRARY})
    target_link_libraries(test LINK_PRIVATE ${GTEST_MAIN_LIBRARY})
endif()

if(UNIX AND NOT APPLE)
  target_link_options(Cerberus PRIVATE -Wl,-z,defs -Wl,--no-undefined)
endif()
