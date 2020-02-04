find_package(pybind11 REQUIRED)

set_property(SOURCE ${CMAKE_BINARY_DIR}/version.cpp PROPERTY GENERATED 1)

pybind11_add_module(_pyantplus EXCLUDE_FROM_ALL
	binding.cpp
	${CMAKE_BINARY_DIR}/version.cpp
	${CMAKE_SOURCE_DIR}/lib/ant.cpp    
	${CMAKE_SOURCE_DIR}/lib/antchannel.cpp
	${CMAKE_SOURCE_DIR}/lib/antdevice.cpp
	${CMAKE_SOURCE_DIR}/lib/debug.cpp
	${CMAKE_SOURCE_DIR}/lib/antmessage.cpp
	${CMAKE_SOURCE_DIR}/lib/antinterface.cpp
	${CMAKE_SOURCE_DIR}/lib/antusbinterface.cpp
)

target_link_libraries(_pyantplus PUBLIC
	Threads::Threads
	${HDF5_LIBRARIES} 
	${LIBUSB1_LIBRARIES}
)

target_include_directories(_pyantplus PUBLIC
	${CMAKE_SOURCE_DIR}/lib)

add_dependencies(_pyantplus version_info)
add_dependencies(_pyantplus ${CPPLINT_TARGET})
