find_package(Doxygen REQUIRED)

if(DOXYGEN_FOUND)
	set(DOXYGEN_GENERATE_HTML NO)
	set(DOXYGEN_GENERATE_XML YES)
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	set(DOXYGEN_INPUT ${CMAKE_SOURCE_DIR}/lib)
	set(DOXYGEN_WARNINGS "NO")
	set(DOXYGEN_QUIET "YES")
	message(STATUS "Configuring Doxygen Build")

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

	add_custom_target(docs_doxygen
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating Doxygen API Documentation"
		VERBATIM)
else()
	message(FATAL_ERROR "Doxygen is needed to generate API documentation")
endif()
