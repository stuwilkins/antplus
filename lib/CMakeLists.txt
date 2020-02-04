include(GNUInstallDirs)

set(SOURCE_FILES 
	${CMAKE_BINARY_DIR}/version.cpp
	ant.cpp    
	antchannel.cpp
	antdevice.cpp
	debug.cpp
	antmessage.cpp
	antinterface.cpp
	antusbinterface.cpp
)

set(PUBLIC_INCLUDE_FILES 
	ant.h
	antchannel.h
	antdevice.h 
	debug.h 
	antmessage.h 
	antinterface.h 
	antusbinterface.h 
)

set_property(SOURCE ${CMAKE_BINARY_DIR}/version.cpp 
	PROPERTY GENERATED 1)

add_library(antplus SHARED ${SOURCE_FILES})

set_target_properties(antplus PROPERTIES 
	OUTPUT_NAME antplus
	VERSION ${antplus_VERSION}
	SOVERSION ${antplus_VERSION_MAJOR}
	PUBLIC_HEADER ${PUBLIC_INCLUDE_FILES}
)

target_include_directories(antplus PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_dependencies(antplus version_info)

target_link_libraries(antplus PUBLIC
	Threads::Threads
	${HDF5_LIBRARIES} 
	${LIBUSB1_LIBRARIES}
)

target_include_directories(antplus PUBLIC 
	${HDF5_INCLUDE_DIRS} 
	${CONFIG++_INCLUDE_DIRS}
	${LIBUSB1_INCLUDE_DIRS}
)

add_dependencies(antplus ${CPPLINT_TARGET})

if (LIB_INSTALL)
	include(CMakePackageConfigHelpers)

    install(
        TARGETS antplus 
		EXPORT antplus-targets
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT Libraries
	)

	install(
		EXPORT antplus-targets
		FILE antplus-targets.cmake
		NAMESPACE antplus::
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/antplus"
	)

	configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/antplus-config.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/antplus-config.cmake"
		INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/antplus"
	)
	
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/antplus-config-version.cmake"
		VERSION ${antplus_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	install(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/antplus-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/antplus-config-version.cmake"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/antplus"
	)
endif()
