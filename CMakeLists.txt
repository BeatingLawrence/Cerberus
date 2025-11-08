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
option(POSTGRESQL "postgresql backend support" OFF)
option(GOOGLETEST "googletest support" OFF)

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

    target_include_directories(Cerberus PUBLIC
        "/usr/include"          #for Linux
        "/opt/homebrew/include" #for MACOS
    )

    target_link_directories(Cerberus PUBLIC
        "/usr/lib"              #for Linux
        "/opt/homebrew/lib"     #for MACOS
    )

    #OPEN-SSL
    target_link_libraries(Cerberus LINK_PRIVATE ssl crypto)

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

