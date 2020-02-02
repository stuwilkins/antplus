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

#include <string.h>
#include <cstdio>
#include <chrono>

#include "ant.h"
#include "antdevice.h"
#include "antchannel.h"
#include "debug.h"

ANT::ANT(std::shared_ptr<ANTInterface> interface, int nChannels) {
    threadRun     = false;
    pollTime      = 2000;  // ms
    extMessages   = true;

    iface = interface;

    DEBUG_PRINT("Creating %d channels.\n", nChannels);
    for (int i=0; i < nChannels; i++) {
        antChannel.push_back(std::shared_ptr<ANTChannel>
            (new ANTChannel(ANTChannel::TYPE_NONE, i, iface)));
    }

    // Set the start time
    startTime = Clock::now();

    // Setup the mutexes
    pthread_mutex_init(&message_lock, NULL);
    pthread_cond_init(&message_cond, NULL);

    // Start the threads
    startThreads();
}

ANT::~ANT(void) {
    // Stop the threads
    stopThreads();
    pthread_mutex_destroy(&message_lock);
    pthread_cond_destroy(&message_cond);
}

int ANT::init(void) {
    int rtn = 0;
    rtn |= iface->reset();
    rtn |= iface->setNetworkKey(0);

    return rtn;
}

int ANT::startThreads(void) {
    threadRun = true;

    DEBUG_COMMENT("Starting listener thread ...\n");
    pthread_create(&listenerId, NULL, callListenerThread, (void *)this);

    DEBUG_COMMENT("Starting Poller Thread ...\n");
    pthread_create(&pollerId, NULL, callPollerThread, (void *)this);

    DEBUG_COMMENT("Starting Processor Thread ...\n");
    pthread_create(&processorId, NULL, callProcessorThread, (void *)this);

    return NOERROR;
}

int ANT::stopThreads(void) {
    DEBUG_COMMENT("Stopping threads.....\n");
    threadRun = false;

    // Wakeup the processor thread
    pthread_cond_signal(&message_cond);

    pthread_join(listenerId, NULL);
    DEBUG_COMMENT("Listener Thread Joined.\n");

    pthread_join(pollerId, NULL);
    DEBUG_COMMENT("Poller Thread Joined.\n");

    pthread_join(processorId, NULL);
    DEBUG_COMMENT("Processor Thread Joined.\n");

    return NOERROR;
}

void* ANT::pollerThread(void) {
    DEBUG_COMMENT("Poller Thread Started\n");

    time_point<Clock> pollStart = Clock::now();

    while (threadRun) {
        time_point<Clock> now = Clock::now();

        auto poll = std::chrono::duration_cast
            <std::chrono::milliseconds> (now - pollStart);

        if (poll.count() >= getPollTime()) {
            for (auto chan : antChannel) {
                int state = chan->getState();
                if ((state == ANTChannel::STATE_OPEN_UNPAIRED) ||
                        (state == ANTChannel::STATE_OPEN_PAIRED)) {
                    if (chan->getType() == ANTChannel::TYPE_FEC) {
                        iface->requestDataPage(chan->getChannelNum(),
                                ANT_DEVICE_COMMON_STATUS);
                        DEBUG_COMMENT("Polling completed\n");
                    }
                }
            }

            pollStart = Clock::now();

        } else {
            usleep(SLEEP_DURATION);  // be a nice thread ...
        }
    }

    return NULL;
}

void* ANT::listenerThread(void) {
    DEBUG_COMMENT("Listener Thread Started\n");

    while (threadRun) {
        std::vector<ANTMessage> message;
        iface->readMessage(&message);
        pthread_mutex_lock(&message_lock);
        for (ANTMessage& m : message) {
            messageQueue.push(m);
        }
        pthread_cond_signal(&message_cond);
        pthread_mutex_unlock(&message_lock);
    }

    return NULL;
}

void* ANT::processorThread(void) {
    while (threadRun) {
        pthread_mutex_lock(&message_lock);
        pthread_cond_wait(&message_cond, &message_lock);

        // Check if we woke up becuase of queued
        // messages
        if (messageQueue.empty()) {
            pthread_mutex_unlock(&message_lock);
            continue;
        }

        ANTMessage m = messageQueue.front();
        messageQueue.pop();

        pthread_mutex_unlock(&message_lock);

        switch (m.getType()) {
            case ANT_NOTIF_STARTUP:
                DEBUG_COMMENT("RESET OK\n");
                break;
            case ANT_CHANNEL_EVENT:
                antChannel[m.getChannel()]->processEvent(&m);
                break;
            case ANT_CHANNEL_ID:
                antChannel[m.getChannel()]->processId(&m);
                break;
            case ANT_BROADCAST_DATA:
            case ANT_ACK_DATA:
                antChannel[m.getChannel()]->parseMessage(&m);
                break;
            default:
                DEBUG_PRINT("UNKNOWN TYPE 0x%02X\n",
                        m.getType());
                break;
        }
    }

    return NULL;
}
