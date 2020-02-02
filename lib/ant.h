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

#ifndef ANT_RECORDER_LIB_ANT_H_
#define ANT_RECORDER_LIB_ANT_H_

#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include <chrono>
#include <vector>
#include <queue>
#include <memory>

#include "antdefs.h"
#include "antmessage.h"
#include "antchannel.h"
#include "ant_network_key.h"

#define SLEEP_DURATION       50000L

extern const char* ANT_GIT_REV;
extern const char* ANT_GIT_BRANCH;
extern const char* ANT_GIT_VERSION;

class ANT {
 public:
    enum rtn {
        NOERROR = 0,
        ERROR = -1
    };
    explicit ANT(std::shared_ptr<ANTInterface> iface, int nChannels = 8);
    ~ANT(void);
    int init(void);

    std::shared_ptr<ANTChannel> getChannel(uint8_t chan) {
        return antChannel[chan];
    }
    // std::vector<ANTChannel>* getChannels(void) {
    //     return &antChannel;
    // }

    int  getPollTime(void)    { return pollTime; }
    void setPollTime(int t)   { pollTime = t; }
    time_point<Clock> getStartTime(void) { return startTime; }

 private:
    std::shared_ptr<ANTInterface> iface;
    std::vector<std::shared_ptr<ANTChannel>> antChannel;
    pthread_t listenerId;
    pthread_t pollerId;
    pthread_t processorId;
    bool threadRun;
    int pollTime;
    bool extMessages;
    time_point<Clock> startTime;

    std::queue<ANTMessage> messageQueue;
    pthread_mutex_t message_lock;
    pthread_cond_t message_cond;

    int startThreads(void);
    int stopThreads(void);
    void* listenerThread(void);
    void* pollerThread(void);
    void* processorThread(void);
    static void* callListenerThread(void *ctx) {
        return ((ANT*)ctx)->listenerThread();
    }
    static void* callPollerThread(void *ctx) {
        return ((ANT*)ctx)->pollerThread();
    }
    static void* callProcessorThread(void *ctx) {
        return ((ANT*)ctx)->processorThread();
    }
};

#endif  // ANT_RECORDER_LIB_ANT_H_
