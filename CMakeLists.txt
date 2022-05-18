cmake_minimum_required(VERSION 3.14)

project(Cerberus LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX -d)

file(GLOB_RECURSE src_files CONFIGURE_DEPENDS
    "src/*"
)

file(GLOB_RECURSE test_files CONFIGURE_DEPENDS
    "test/*"
)

add_library(Cerberus SHARED
  cpp.astylerc
  ${src_files}
)

target_compile_definitions(Cerberus PRIVATE CERBERUS_LIBRARY)

install(TARGETS Cerberus DESTINATION lib)
install(DIRECTORY src/. DESTINATION include/cerberus)

add_executable(test EXCLUDE_FROM_ALL
  ${test_files}
)

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

    if(EXISTS "../Dist/lib/Cerberus-d.lib")
        message(STATUS "Cerberus debug library found!")
        find_library(CERBERUS_LIBRARY NAMES Cerberus-d PATHS "../Dist/lib" PATH_SUFFIXES lib)
    endif()

    find_library(GTEST_LIBRARY NAMES gtest PATHS "./gtest/lib" PATH_SUFFIXES lib)
    find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "./gtest/lib" PATH_SUFFIXES lib)

    if((GTEST_LIBRARY STREQUAL "GTEST_LIBRARY-NOTFOUND") OR (GTEST_MAIN_LIBRARY STREQUAL "GTEST_MAIN_LIBRARY-NOTFOUND"))
        message(FATAL_ERROR "GTest library needed on Windows system! Please Read README.md")
    endif()

else()

    message(STATUS "On Linux system")

    target_include_directories(test PUBLIC
        "/usr/include"
    )

    if(EXISTS "../Dist/lib/libCerberus-d.so")
        message(STATUS "Cerberus debug library found!")
        find_library(CERBERUS_LIBRARY NAMES Cerberus-d PATHS "../Dist/lib" PATH_SUFFIXES lib)
    endif()

    find_library(GTEST_LIBRARY NAMES gtest PATHS "/usr/lib" PATH_SUFFIXES lib)
    find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "/usr/lib" PATH_SUFFIXES lib)

endif(MSVC)

target_link_libraries(test LINK_PUBLIC ${CERBERUS_LIBRARY})
target_link_libraries(test LINK_PUBLIC ${GTEST_LIBRARY})
target_link_libraries(test LINK_PUBLIC ${GTEST_MAIN_LIBRARY})

