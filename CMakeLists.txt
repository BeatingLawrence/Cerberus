cmake_minimum_required(VERSION 3.14)

project(Cerberus LANGUAGES CXX VERSION 0.9.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX -d)

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS
    "src/*.cpp"
)
file(GLOB_RECURSE header_files CONFIGURE_DEPENDS
    "src/*.h"
)

file(GLOB_RECURSE test_files CONFIGURE_DEPENDS
    "test/*"
)

add_library(Cerberus SHARED
  cpp.astylerc
  ${src_files}
  ${header_files}
)

add_executable(test EXCLUDE_FROM_ALL
  ${test_files}
)

set_target_properties(Cerberus PROPERTIES PREFIX "lib" IMPORT_PREFIX "lib")

target_compile_definitions(Cerberus PRIVATE CERBERUS_LIBRARY PRIVATE _CRT_SECURE_NO_WARNINGS PRIVATE CERBERUS_VERSION="${CMAKE_PROJECT_VERSION}")
message(STATUS ${CMAKE_PROJECT_VERSION})

install(TARGETS Cerberus DESTINATION lib)
install(DIRECTORY src/ DESTINATION include/cerberus FILES_MATCHING PATTERN "*.h")

target_include_directories(test PUBLIC
    "../Dist/include"
)

set_target_properties(Cerberus test PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL"
)

if(MSVC)

    target_compile_definitions(Cerberus PRIVATE WINDOWS_SYSTEM)

    message(STATUS "On Windows system")

    target_include_directories(test PUBLIC
        "./gtest/include"
    )

    if(NOT EXISTS "../Dist/lib/libCerberus-d.lib")
        message(STATUS "Cerberus debug library NOT found!")
    endif()

    find_library(GTEST_LIBRARY NAMES gtest PATHS "./gtest/lib" PATH_SUFFIXES lib)
    find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "./gtest/lib" PATH_SUFFIXES lib)

    if((GTEST_LIBRARY STREQUAL "GTEST_LIBRARY-NOTFOUND") OR (GTEST_MAIN_LIBRARY STREQUAL "GTEST_MAIN_LIBRARY-NOTFOUND"))
        message(FATAL_ERROR "GTest library needed on Windows system! Please Read README.md")
    endif()

    #OPEN-SSL

    target_include_directories(Cerberus PUBLIC "C:/Qt/Tools/OpenSSL/Win_x64/include")
    target_link_directories(Cerberus PUBLIC "C:/Qt/Tools/OpenSSL/Win_x64/lib")
    target_link_libraries(Cerberus LINK_PUBLIC libssl)
    target_link_libraries(Cerberus LINK_PUBLIC libcrypto)

else()

    message(STATUS "On Linux system")

    target_include_directories(test PUBLIC
        "/usr/include"
    )

    if(NOT EXISTS "../Dist/lib/libCerberus-d.so")
        message(STATUS "Cerberus debug library NOT found!")
    endif()

    find_library(GTEST_LIBRARY NAMES gtest PATHS "/usr/lib" PATH_SUFFIXES lib)
    find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "/usr/lib" PATH_SUFFIXES lib)

    #OPEN-SSL

    target_link_libraries(Cerberus LINK_PUBLIC ssl)
    target_link_libraries(Cerberus LINK_PUBLIC crypto)
    target_link_libraries(test LINK_PUBLIC ssl)
    target_link_libraries(test LINK_PUBLIC crypto)

    #TO-DO ADD

endif(MSVC)

target_link_directories(test PUBLIC "../Dist/lib")

target_link_libraries(test LINK_PUBLIC Cerberus)
target_link_libraries(test LINK_PUBLIC ${GTEST_LIBRARY})
target_link_libraries(test LINK_PUBLIC ${GTEST_MAIN_LIBRARY})

