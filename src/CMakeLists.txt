add_executable(ant-recorder antusb.cpp ant.cpp antdevice.cpp antchannel.cpp debug.cpp antmessage.cpp)
target_link_libraries(ant-recorder usb-1.0)
target_link_libraries(ant-recorder pthread)
