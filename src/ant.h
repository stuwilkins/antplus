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

#include <chrono>
#include <vector>

#include "antdefs.h"
#include "antmessage.h"
#include "antchannel.h"
#include "ant_network_key.h"

#define SLEEP_DURATION       50000L
#define RESET_DURATION      500000L

extern const char* ANT_GIT_REV;
extern const char* ANT_GIT_BRANCH;
extern const char* ANT_GIT_VERSION;

void* antusb_listener(void *ctx);
void* antusb_poller(void *ctx);

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
    int sendMessage(AntMessage *message);
    int readMessage(std::vector<AntMessage> *message);
    int reset(void);
    int setNetworkKey(uint8_t net);
    int assignChannel(uint8_t chanNum, bool master, uint8_t net);
    int setChannelID(uint8_t chan, uint16_t device,
            uint8_t type, bool master);
    int setSearchTimeout(uint8_t chan, uint8_t timeout);
    int setChannelPeriod(uint8_t chan, uint16_t period);
    int setChannelFreq(uint8_t chan, uint8_t frequency);
    int openChannel(uint8_t chan);
    int requestMessage(uint8_t chan, uint8_t message);
    int requestDataPage(uint8_t chan, uint8_t page);
    int setLibConfig(uint8_t chan, uint8_t config);
    int startThreads(void);
    int stopThreads(void);
    int changeStateTo(int state);
    int channelProcessID(AntMessage *m);
    int channelProcessEvent(AntMessage *m);
    int channelProcessBroadcast(AntMessage *m);
    int channelChangeStateTo(uint8_t chan, int state);
    int channelStart(uint8_t chan, int type,
            uint16_t id = 0x0000, bool wait = true);
    AntChannel *getChannel(uint8_t chan) {
        return &(antChannel[chan]);
    }
    int  getNumChannels(void) { return numChannels; }
    bool getThreadRun(void)   { return threadRun; }
    int  getPollTime(void)    { return pollTime; }
    void setPollTime(int t)   { pollTime = t; }
    time_point<Clock> getStartTime(void) { return startTime; }

 protected:
    time_point<Clock> startTime;

 private:
     libusb_context *usb_ctx;
     libusb_device_handle *usb_handle;
     libusb_config_descriptor *usb_config;
     int readEndpoint;
     int writeEndpoint;
     int readTimeout;
     int writeTimeout;
     int numChannels;
     AntChannel *antChannel;
     pthread_t listenerId;
     pthread_t pollerId;
     bool threadRun;
     int pollTime;
     bool extMessages;
};

#endif  // SRC_ANT_H_
