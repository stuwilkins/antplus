find_package(Sphinx REQUIRED)

set(SPHINX_SOURCE_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in")
set(SPHINX_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/conf.py")
set(SPHINX_STATIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_static")
set(DOXYGEN_XML_DIR "${CMAKE_CURRENT_BINARY_DIR}/../doxygen/xml")

configure_file(${SPHINX_SOURCE_CONFIG_FILE} ${SPHINX_CONFIG_FILE} @ONLY)

message(STATUS "Configuring Sphinx Build")
add_custom_target(docs
	${SPHINX_EXECUTABLE}
	-b html
	-j auto
	-D html_static_path="${CMAKE_CURRENT_SOURCE_DIR}/_static"
	-c "${CMAKE_CURRENT_BINARY_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/"
	"${CMAKE_CURRENT_BINARY_DIR}/../build"
	COMMENT "Building HTML documentation with Sphinx"
	DEPENDS docs_doxygen)
