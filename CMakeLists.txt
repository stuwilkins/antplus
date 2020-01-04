cmake_minimum_required(VERSION 3.12)
project(ant-recorder VERSION 0.1)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(PreventInSourceBuilds)
include(cpplint)
#cpplint_add_subdirectory(src)

# Add __FILENAME__ with short path
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/version.cpp
	${CMAKE_BINARY_DIR}/_version.cpp
	COMMAND ${CMAKE_COMMAND} -P
	${CMAKE_SOURCE_DIR}/cmake/version.cmake
)	

add_compile_options(-DDEBUG_OUTPUT)

add_subdirectory(src)
