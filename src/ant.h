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

#ifndef SRC_ANT_H_
#define SRC_ANT_H_

#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include "antdefs.h"
#include "ant_network_key.h"

#define ANT_DECODE_LEN_ERROR            -1
#define ANT_DECODE_CRC_ERROR            -2
#define ANT_DECODE_OK                    0
#define ANT_DECODE_PROTO_ERROR          -3


void bytestream_to_string(char *out, int n_out,
        uint8_t *bytes, int n_bytes);
static void* antusb_listener(void *ctx);

class AntMessage {
 public:
    AntMessage(void);
    AntMessage(uint8_t *data, int data_len);
    AntMessage(uint8_t type, uint8_t chan, uint8_t *data, int len);
    AntMessage(uint8_t type, uint8_t *data, int len);
    AntMessage(uint8_t type, uint8_t chan);
    AntMessage(uint8_t type, uint8_t chan, uint8_t b0);
    AntMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1);
    AntMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1,
            uint8_t b2);
    AntMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1,
            uint8_t b2, uint8_t b3);
    AntMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1,
            uint8_t b2, uint8_t b3, uint8_t b4);

    void     encode(uint8_t *msg, int *len);
    int      decode(uint8_t *data, int data_len);
    uint8_t  getType(void)    { return antType;}
    uint8_t  getChannel(void) { return antChannel;}
    uint8_t* getData(void)    { return antData;}
    int      getDataLen(void) { return antDataLen;}

 private:
    uint8_t antType;
    uint8_t antChannel;
    uint8_t antData[ANT_MAX_DATA_SIZE];
    int     antDataLen;
};

class AntUsb {
 public:
    enum rtn {
        NOERROR = 0,
        ERROR = -1
    };
    AntUsb(void);
    ~AntUsb(void);
    int init(void);
    int setup(void);
    int bulkRead(uint8_t *bytes, int size, int timeout);
    int bulkWrite(uint8_t *bytes, int size, int timeout);
    int sendMessage(AntMessage *message, bool readReply = true);
    int readMessage(AntMessage *message);
    int reset(void);
    int setNetworkKey(void);
    int assignChannel(int chanNum, bool master);
    int setChannelID(uint8_t chan, uint16_t device,
            uint8_t type, bool master);
    int setSearchTimeout(uint8_t chan, uint8_t timeout);
    int setChannelPeriod(uint8_t chan, uint16_t period);
    int setChannelFreq(uint8_t chan, uint8_t frequency);
    int openChannel(uint8_t chan);
    int requestMessage(uint8_t chan, uint8_t message);
    int startListener(void);

 private:
     libusb_context *usb_ctx;
     libusb_device_handle *usb_handle;
     libusb_config_descriptor *usb_config;
     int readEndpoint;
     int writeEndpoint;
     int readTimeout;
     int writeTimeout;
     pthread_t listenerId;
};

#endif  // SRC_ANT_H_
