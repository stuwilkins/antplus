//
// ant-recoder : ANT+ Utilities
//
// MIT License
//
// Copyright (c) 2020 Stuart Wilkins
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef ANTPLUS_LIB_ANTUSBINTERFACE_H_
#define ANTPLUS_LIB_ANTUSBINTERFACE_H_

#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include <vector>

#include "ant.h"
#include "antmessage.h"
#include "debug.h"

class ANTUSBInterface : public ANTInterface {
 public:
    enum rtn {
        NOERROR = 0,
        ERROR = -1
    };
    ANTUSBInterface(void);
    ~ANTUSBInterface(void);
    int open(void);
    int close(void);
    int sendMessage(ANTMessage *message);
    int readMessage(std::vector<ANTMessage> *message);
 private:
    int bulkRead(uint8_t *bytes, int size, int timeout);
    int bulkWrite(uint8_t *bytes, int size, int timeout);
    libusb_context *usb_ctx;
    libusb_device_handle *usb_handle;
    libusb_config_descriptor *usb_config;
    int readEndpoint;
    int writeEndpoint;
    int readTimeout;
    int writeTimeout;
};

#endif  // ANTPLUS_LIB_ANTUSBINTERFACE_H_
