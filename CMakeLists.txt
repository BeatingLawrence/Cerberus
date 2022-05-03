cmake_minimum_required(VERSION 3.14)

project(Cerberus LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
)

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

find_library(CERBERUS_LIBRARY NAMES Cerberus PATHS "../Dist/lib" PATH_SUFFIXES lib)
target_link_libraries(test LINK_PUBLIC ${CERBERUS_LIBRARY})

find_library(GTEST_LIBRARY NAMES gtest PATHS "/usr/lib" PATH_SUFFIXES lib)
target_link_libraries(test LINK_PUBLIC ${GTEST_LIBRARY})

find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "/usr/lib" PATH_SUFFIXES lib)
target_link_libraries(test LINK_PUBLIC ${GTEST_MAIN_LIBRARY})
