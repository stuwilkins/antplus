find_package(pybind11 REQUIRED)

set_property(SOURCE ${CMAKE_BINARY_DIR}/version.cpp PROPERTY GENERATED 1)

pybind11_add_module(_pyant EXCLUDE_FROM_ALL
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

target_link_libraries(_pyant PUBLIC
	Threads::Threads
	${HDF5_LIBRARIES} 
	${LIBUSB1_LIBRARIES}
)

target_include_directories(_pyant PUBLIC
	${CMAKE_SOURCE_DIR}/lib)

add_dependencies(_pyant version_info)
add_dependencies(_pyant ${CPPLINT_TARGET})