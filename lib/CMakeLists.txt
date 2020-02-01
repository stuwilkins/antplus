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

add_library(antcxx SHARED ${SOURCE_FILES})

set_target_properties(antcxx PROPERTIES 
	OUTPUT_NAME antcxx
	VERSION ${antcxx_VERSION}
	SOVERSION ${antcxx_VERSION_MAJOR}
	PUBLIC_HEADER ${PUBLIC_INCLUDE_FILES}
)

target_include_directories(antcxx PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_dependencies(antcxx version_info)

target_link_libraries(antcxx PUBLIC
	Threads::Threads
	${HDF5_LIBRARIES} 
	${LIBUSB1_LIBRARIES}
)

target_include_directories(antcxx PUBLIC 
	${HDF5_INCLUDE_DIRS} 
	${CONFIG++_INCLUDE_DIRS}
	${LIBUSB1_INCLUDE_DIRS}
)

add_dependencies(antcxx ${CPPLINT_TARGET})

if (LIB_INSTALL)
	include(CMakePackageConfigHelpers)

    install(
        TARGETS antcxx 
		EXPORT antcxx-targets
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        COMPONENT Libraries
	)

	install(
		EXPORT antcxx-targets
		FILE antcxx-targets.cmake
		NAMESPACE antcxx::
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/antcxx"
	)

	configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/antcxx-config.cmake.in"
		"${CMAKE_CURRENT_BINARY_DIR}/antcxx-config.cmake"
		INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/antcxx"
	)
	
	write_basic_package_version_file(
		"${CMAKE_CURRENT_BINARY_DIR}/antcxx-config-version.cmake"
		VERSION ${antcxx_VERSION}
		COMPATIBILITY AnyNewerVersion
	)

	install(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/antcxx-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/antcxx-config-version.cmake"
		DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/antcxx"
	)
endif()
