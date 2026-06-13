function(cerberus_configure_platform target)
  if(MSVC)
    message(STATUS "On Windows system")

    include(CheckCXXSourceCompiles)
    unset(CERBERUS_MSVC_STDLIB_AVAILABLE CACHE)
    check_cxx_source_compiles(
      "#include <atomic>
       #include <string>
       int main()
       {
         std::atomic<int> value{0};
         std::string text;
         return value.load() + static_cast<int>(text.size());
       }"
      CERBERUS_MSVC_STDLIB_AVAILABLE
    )

    if(NOT CERBERUS_MSVC_STDLIB_AVAILABLE)
      message(FATAL_ERROR
        "MSVC standard library headers were not found. When using Ninja with MSVC, "
        "run CMake from a Visual Studio Developer PowerShell/Command Prompt, or configure "
        "the IDE kit to load VsDevCmd.bat/vcvars64.bat before CMake. "
        "Compiler: ${CMAKE_CXX_COMPILER}"
      )
    endif()

    set_target_properties(${target} PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

    target_compile_definitions(${target} PUBLIC WINDOWS_SYSTEM NOMINMAX WIN32_LEAN_AND_MEAN)

    set(CERBERUS_EFFECTIVE_OPENSSL_ROOT "${CERBERUS_OPENSSL_ROOT}")
    if(NOT CERBERUS_EFFECTIVE_OPENSSL_ROOT)
      set(CERBERUS_OPENSSL_ROOT_CANDIDATES
        "$ENV{ProgramFiles}/OpenSSL-Win64"
        "C:/Program Files/OpenSSL-Win64"
        "C:/Qt/Tools/OpenSSL/Win_x64"
      )

      foreach(CERBERUS_OPENSSL_ROOT_CANDIDATE IN LISTS CERBERUS_OPENSSL_ROOT_CANDIDATES)
        if(EXISTS "${CERBERUS_OPENSSL_ROOT_CANDIDATE}/include/openssl/evp.h")
          set(CERBERUS_EFFECTIVE_OPENSSL_ROOT "${CERBERUS_OPENSSL_ROOT_CANDIDATE}")
          break()
        endif()
      endforeach()
    endif()

    if(NOT CERBERUS_EFFECTIVE_OPENSSL_ROOT)
      message(FATAL_ERROR
        "OpenSSL for MSVC was not found. Set CERBERUS_OPENSSL_ROOT to an OpenSSL root containing "
        "include/openssl/evp.h and lib/VC/x64/MD/libssl.lib/libcrypto.lib."
      )
    endif()

    if(NOT EXISTS "${CERBERUS_EFFECTIVE_OPENSSL_ROOT}/include/openssl/evp.h")
      message(FATAL_ERROR
        "CERBERUS_OPENSSL_ROOT='${CERBERUS_EFFECTIVE_OPENSSL_ROOT}' does not contain include/openssl/evp.h"
      )
    endif()

    set(CERBERUS_OPENSSL_LIB_DIR_RELEASE "${CERBERUS_EFFECTIVE_OPENSSL_ROOT}/lib/VC/x64/MD")
    set(CERBERUS_OPENSSL_LIB_DIR_DEBUG "${CERBERUS_EFFECTIVE_OPENSSL_ROOT}/lib/VC/x64/MDd")

    message(STATUS "Using OpenSSL root: ${CERBERUS_EFFECTIVE_OPENSSL_ROOT}")

    if(NOT EXISTS "${CERBERUS_OPENSSL_LIB_DIR_RELEASE}/libssl.lib" OR
       NOT EXISTS "${CERBERUS_OPENSSL_LIB_DIR_RELEASE}/libcrypto.lib")
      message(FATAL_ERROR
        "CERBERUS_OPENSSL_ROOT='${CERBERUS_EFFECTIVE_OPENSSL_ROOT}' does not contain "
        "lib/VC/x64/MD/libssl.lib and libcrypto.lib"
      )
    endif()

    if(NOT EXISTS "${CERBERUS_OPENSSL_LIB_DIR_DEBUG}/libssl.lib" OR
       NOT EXISTS "${CERBERUS_OPENSSL_LIB_DIR_DEBUG}/libcrypto.lib")
      set(CERBERUS_OPENSSL_LIB_DIR_DEBUG "${CERBERUS_OPENSSL_LIB_DIR_RELEASE}")
    endif()

    target_include_directories(${target} PUBLIC "${CERBERUS_EFFECTIVE_OPENSSL_ROOT}/include")
    target_link_libraries(${target} LINK_PUBLIC
      $<$<CONFIG:Debug>:${CERBERUS_OPENSSL_LIB_DIR_DEBUG}/libssl.lib>
      $<$<CONFIG:Debug>:${CERBERUS_OPENSSL_LIB_DIR_DEBUG}/libcrypto.lib>
      $<$<NOT:$<CONFIG:Debug>>:${CERBERUS_OPENSSL_LIB_DIR_RELEASE}/libssl.lib>
      $<$<NOT:$<CONFIG:Debug>>:${CERBERUS_OPENSSL_LIB_DIR_RELEASE}/libcrypto.lib>
      Shlwapi
      Ws2_32
    )
  elseif(UNIX)
    message(STATUS "On Linux/MACOSX system")

    if(APPLE)
      target_compile_definitions(${target} PUBLIC APPLE_SYSTEM)
    else()
      target_compile_definitions(${target} PUBLIC LINUX_SYSTEM)
    endif()

    if(NOT APPLE)
      find_package(Threads REQUIRED)
      target_link_libraries(${target} PRIVATE Threads::Threads ${CMAKE_DL_LIBS} rt)
      target_link_options(${target} PRIVATE -Wl,--no-as-needed -Wl,-z,defs -Wl,--no-undefined)
    endif()

    if(CERBERUS_OPENSSL_ROOT)
      get_filename_component(CERBERUS_OPENSSL_ROOT_ABS "${CERBERUS_OPENSSL_ROOT}" ABSOLUTE)
      if(NOT EXISTS "${CERBERUS_OPENSSL_ROOT_ABS}/include/openssl/evp.h")
        message(FATAL_ERROR
          "CERBERUS_OPENSSL_ROOT='${CERBERUS_OPENSSL_ROOT_ABS}' does not contain include/openssl/evp.h"
        )
      endif()

      set(OPENSSL_ROOT_DIR "${CERBERUS_OPENSSL_ROOT_ABS}")
      message(STATUS "Using explicit OpenSSL root: ${CERBERUS_OPENSSL_ROOT_ABS}")
    endif()

    find_package(OpenSSL REQUIRED)

    if(CERBERUS_OPENSSL_ROOT)
      file(TO_CMAKE_PATH "${CERBERUS_OPENSSL_ROOT_ABS}" CERBERUS_OPENSSL_ROOT_CMAKE)
      foreach(CERBERUS_OPENSSL_PATH IN ITEMS
          "${OPENSSL_INCLUDE_DIR}"
          "${OPENSSL_SSL_LIBRARY}"
          "${OPENSSL_CRYPTO_LIBRARY}"
      )
        file(TO_CMAKE_PATH "${CERBERUS_OPENSSL_PATH}" CERBERUS_OPENSSL_PATH_CMAKE)
        string(FIND "${CERBERUS_OPENSSL_PATH_CMAKE}" "${CERBERUS_OPENSSL_ROOT_CMAKE}/" CERBERUS_OPENSSL_PATH_MATCH)
        if(NOT CERBERUS_OPENSSL_PATH_MATCH EQUAL 0)
          message(FATAL_ERROR
            "CERBERUS_OPENSSL_ROOT='${CERBERUS_OPENSSL_ROOT_ABS}' is set, but OpenSSL resolved outside it: "
            "${CERBERUS_OPENSSL_PATH}"
          )
        endif()
      endforeach()
    endif()

    target_link_libraries(${target} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  endif()
endfunction()
