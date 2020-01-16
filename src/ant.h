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

class ANTInterface {
 public:
    virtual int open(void) = 0;
    virtual int close(void) = 0;
    virtual int sendMessage(ANTMessage *message) = 0;
    virtual int readMessage(std::vector<ANTMessage> *message) = 0;
};

class ANT {
 public:
    enum rtn {
        NOERROR = 0,
        ERROR = -1
    };
    explicit ANT(ANTInterface *iface);
    ~ANT(void);
    int reset(void);
    int setNetworkKey(uint8_t net);
    int assignChannel(uint8_t chanNum, uint8_t chanType, uint8_t net);
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
    int channelProcessID(ANTMessage *m);
    int channelProcessEvent(ANTMessage *m);
    int channelProcessBroadcast(ANTMessage *m);
    int channelChangeStateTo(uint8_t chan, int state);
    int channelStart(uint8_t chan, int type,
            uint16_t id = 0x0000, bool wait = true);
    ANTChannel *getChannel(uint8_t chan) {
        return &(antChannel[chan]);
    }
    int  getNumChannels(void) { return numChannels; }
    int  getPollTime(void)    { return pollTime; }
    void setPollTime(int t)   { pollTime = t; }
    time_point<Clock> getStartTime(void) { return startTime; }

 protected:
    time_point<Clock> startTime;

 private:
    ANTInterface *iface;
    int numChannels;
    ANTChannel *antChannel;
    pthread_t listenerId;
    pthread_t pollerId;
    bool threadRun;
    int pollTime;
    bool extMessages;

    void* listenerThread(void);
    void* pollerThread(void);
    static void* callListenerThread(void *ctx) {
        return ((ANT*)ctx)->listenerThread();
    }
    static void* callPollerThread(void *ctx) {
        return ((ANT*)ctx)->pollerThread();
    }
};

#endif  // SRC_ANT_H_
