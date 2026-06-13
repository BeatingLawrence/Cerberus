function(cerberus_add_docs)
  set(CERBERUS_DOC_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc-build")

  add_custom_target(doc
    COMMAND "${CMAKE_COMMAND}"
            -S "${CMAKE_CURRENT_SOURCE_DIR}/cmake/docs"
            -B "${CERBERUS_DOC_BUILD_DIR}"
            -DCERBERUS_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
            -DCERBERUS_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
    COMMAND "${CMAKE_COMMAND}" --build "${CERBERUS_DOC_BUILD_DIR}" --target doc
    COMMENT "Configuring and generating Cerberus documentation"
    VERBATIM
  )
endfunction()
