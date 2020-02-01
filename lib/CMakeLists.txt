include(GNUInstallDirs)

set(LIB_SOURCE_FILES ${CMAKE_BINARY_DIR}/version.cpp
	ant.cpp    
	antchannel.cpp
	antdevice.cpp
	debug.cpp
	antmessage.cpp
	antinterface.cpp
	antusbinterface.cpp
	)

set_property(SOURCE ${CMAKE_BINARY_DIR}/version.cpp 
	PROPERTY GENERATED 1)

add_executable(ant-recorder ${SOURCE_FILES})

target_link_libraries(ant-recorder PUBLIC
	Threads::Threads
	${HDF5_LIBRARIES} 
	${CONFIG++_LIBRARIES}
	${LIBUSB1_LIBRARIES}
	)

target_include_directories(ant-recorder PUBLIC 
	${HDF5_INCLUDE_DIRS} 
	${CONFIG++_INCLUDE_DIRS}
	${LIBUSB1_INCLUDE_DIRS}
	)

add_dependencies(ant-recorder version_info)
