cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED CERBERUS_SOURCE_DIR)
    message(FATAL_ERROR "CERBERUS_SOURCE_DIR is required")
endif()

if(NOT DEFINED CERBERUS_PROJECT_VERSION)
    set(CERBERUS_PROJECT_VERSION 0.0.0)
endif()

project(CerberusDocs LANGUAGES NONE VERSION ${CERBERUS_PROJECT_VERSION})

find_package(Doxygen REQUIRED)
message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

include(FetchContent)

FetchContent_Declare(
    doxygen-awesome-css
    URL https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(doxygen-awesome-css)
FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

if(DOXYGEN_DOT_EXECUTABLE)
    set(HAVE_DOT_VALUE YES)
    get_filename_component(DOT_PATH "${DOXYGEN_DOT_EXECUTABLE}" DIRECTORY)
else()
    set(HAVE_DOT_VALUE NO)
    set(DOT_PATH "")
endif()

set(DOXYFILE_IN "${CERBERUS_SOURCE_DIR}/Doxyfile.in")
set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
configure_file("${DOXYFILE_IN}" "${DOXYFILE_OUT}" @ONLY)

add_custom_target(doc
    COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE_OUT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Generating Cerberus HTML/LaTeX documentation with doxygen-awesome-css"
    VERBATIM
)
