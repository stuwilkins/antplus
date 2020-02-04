//
// antplus : ANT+ Utilities
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

#include <unistd.h>

#include <algorithm>
#include <memory>

#include "antplus.h"
#include "antchannel.h"
#include "antdefs.h"
#include "antdebug.h"

ANTDeviceParams antDeviceParams[] = {
    {  ANTChannel::TYPE_HR,   0x78, 0x1F86, 0x39 },
    {  ANTChannel::TYPE_PWR,  0x0B, 0x1FF6, 0x39 },
    {  ANTChannel::TYPE_FEC,  0x11, 0x2000, 0x39 },
    {  ANTChannel::TYPE_PAIR, 0x00, 0x0000, 0x39 },
    {  ANTChannel::TYPE_NONE, 0x00, 0x0000, 0x00 },
};

ANTChannel::ANTChannel(int type, int num,
        std::shared_ptr<ANTInterface> interface) {
    network             = 0x00;
    searchTimeout       = 0xFF;
    channelNum          = num;
    channelType         = CHANNEL_TYPE_RX;
    channelTypeExtended = 0x00;
    currentState        = STATE_IDLE;
    deviceId            = 0x0000;
    extended            = 0x00;
    iface               = interface;
    threadRun           = true;
    channelStartTimeout = 60;  // seconds

    setType(type);

    // Setup the mutexes
    pthread_mutex_init(&message_lock, NULL);
    pthread_cond_init(&message_cond, NULL);

    // Start the thread
    startThread();
}

ANTChannel::~ANTChannel(void) {
    DEBUG_PRINT("chan = %d\n", channelNum);
    // Stop the thread
    stopThread();

    // for (auto dev : deviceList) {
    //     switch (dev->getDeviceID().getType()) {
    //         case ANT_DEVICE_NONE:
    //             delete (ANTDeviceNONE*)dev;
    //             break;
    //         case ANT_DEVICE_HR:
    //             delete (ANTDeviceHR*)dev;
    //             break;
    //         case ANT_DEVICE_FEC:
    //             delete (ANTDeviceFEC*)dev;
    //             break;
    //         case ANT_DEVICE_PWR:
    //             delete (ANTDevicePWR*)dev;
    //             break;
    //     }
    // }

    pthread_mutex_destroy(&message_lock);
    pthread_cond_destroy(&message_cond);
}

int ANTChannel::startThread(void) {
    threadRun = true;

    DEBUG_COMMENT("Starting channel thread ...\n");
    pthread_create(&threadId, NULL, callThread, (void *)this);

    return NOERROR;
}

int ANTChannel::stopThread(void) {
    DEBUG_COMMENT("Stopping threads.....\n");
    threadRun = false;

    // Wakeup the processor thread
    pthread_cond_signal(&message_cond);

    pthread_join(threadId, NULL);
    DEBUG_COMMENT("Channel Thread Joined.\n");

    return NOERROR;
}

void* ANTChannel::thread(void) {
    DEBUG_COMMENT("Thread started.....\n");
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

        ANTDeviceID devID = m.getDeviceID();

        if (!devID.isValid()) {
            DEBUG_COMMENT("Processing with no device id info\n");
            continue;
        }

        bool found = false;
        for (auto dev : deviceList) {
            if (*dev == devID) {
                dev->parseMessage(&m);
                found = true;
                break;
            }
        }

        if (!found) {
            addDevice(&devID)->parseMessage(&m);
        }
    }

    return NULL;
}

void ANTChannel::setType(int t) {
    DEBUG_PRINT("Setting type to %d\n", t);

    int i = 0;
    while (antDeviceParams[i].type != TYPE_NONE) {
        if (antDeviceParams[i].type == t) {
            deviceParams = antDeviceParams[i];
            type = t;
            DEBUG_PRINT("type = 0x%02X period = 0x%04X freq = 0x%02X\n",
                    deviceParams.deviceType,
                    deviceParams.devicePeriod,
                    deviceParams.deviceFrequency);
            break;
        }
        i++;
    }
}

std::shared_ptr<ANTDevice> ANTChannel::addDevice(ANTDeviceID *id) {
    ANTDevice *dev = nullptr;

    switch (id->getType()) {
        case ANT_DEVICE_NONE:
            dev = new ANTDeviceNONE(*id);
            break;
        case ANT_DEVICE_HR:
            dev = new ANTDeviceHR(*id);
            break;
        case ANT_DEVICE_PWR:
            dev = new ANTDevicePWR(*id);
            break;
        case ANT_DEVICE_FEC:
            dev = new ANTDeviceFEC(*id);
            break;
    }

    if (dev != nullptr) {
        DEBUG_PRINT("Adding device type = 0x%02X, %p\n",
                id->getType(), (void*)dev);
        std::shared_ptr<ANTDevice> sharedDev(dev);
        deviceList.push_back(sharedDev);
        return sharedDev;
    }

    return nullptr;
}

void ANTChannel::parseMessage(ANTMessage *message) {
    ANTMessage m = *message;

    pthread_mutex_lock(&message_lock);
    messageQueue.push(m);
    pthread_cond_signal(&message_cond);
    pthread_mutex_unlock(&message_lock);
}

int ANTChannel::processEvent(ANTMessage *m) {
    if (m->getChannel() != channelNum) {
        DEBUG_PRINT("Message is for channel %d but channelNum = %d\n",
                m->getChannel(), channelNum);
        return ERROR;
    }

    uint8_t commandCode = m->getData(0);
    if (commandCode != 0x01) {
        switch (commandCode) {
            case ANT_SET_NETWORK:
                DEBUG_COMMENT("ANT_SET_NETWORK Receieved\n");
                break;
            case ANT_UNASSIGN_CHANNEL:
                DEBUG_COMMENT("ANT_UNASSIGN_CHANNEL Recieved\n");
                break;
            case ANT_ASSIGN_CHANNEL:
                DEBUG_COMMENT("ANT_ASSIGN_CHANNEL Recieved\n");
                currentState = STATE_ASSIGNED;
                changeStateTo(STATE_ID_SET);
                break;
            case ANT_CHANNEL_ID:
                DEBUG_COMMENT("ANT_CHANNEL_ID Recieved\n");
                currentState = STATE_ID_SET;
                changeStateTo(STATE_SET_TIMEOUT);
                break;
            case ANT_SEARCH_TIMEOUT:
                // Do nothing
                DEBUG_COMMENT("ANT_SEARCH_TIMEOUT Recieved\n");
                break;
            case ANT_LP_SEARCH_TIMEOUT:
                DEBUG_COMMENT("ANT_LP_SEARCH_TIMEOUT Recieved\n");
                currentState = STATE_SET_TIMEOUT;
                changeStateTo(STATE_SET_PERIOD);
                break;
            case ANT_CHANNEL_PERIOD:
                DEBUG_COMMENT("ANT_CHANNEL_PERIOD Recieved\n");
                currentState = STATE_SET_PERIOD;
                changeStateTo(STATE_SET_FREQ);
                break;
            case ANT_CHANNEL_FREQUENCY:
                DEBUG_COMMENT("ANT_CHANNEL_FREQUENCY Recieved\n");
                currentState = STATE_SET_FREQ;
                changeStateTo(STATE_OPEN_UNPAIRED);
                break;
            case ANT_LIB_CONFIG:
                DEBUG_COMMENT("ANT_LIB_CONFIG Recieved\n");
                break;
            case ANT_OPEN_CHANNEL:
                DEBUG_COMMENT("ANT_OPEN_CHANNEL Recieved\n");
                // Do nothing, but set state
                currentState = STATE_OPEN_UNPAIRED;
                break;
            default:
                DEBUG_PRINT("Unknown command 0x%02X\n", commandCode);
                break;
        }
    } else {
        uint8_t eventCode = m->getData(1);
        switch (eventCode) {
            case EVENT_RX_SEARCH_TIMEOUT:
                DEBUG_PRINT("Search timeout on channel %d\n", channelNum);
                currentState = STATE_OPEN_UNPAIRED;
                break;
            case EVENT_RX_FAIL:
                DEBUG_PRINT("RX Failed on channel %d\n", channelNum);
                break;
            case EVENT_TX:
                DEBUG_PRINT("TX on channel %d\n", channelNum);
                break;
            case EVENT_TRANSFER_RX_FAILED:
                DEBUG_PRINT("RX Transfer Completed on channel %d\n",
                        channelNum);
                break;
            case EVENT_TRANSFER_TX_COMPLETED:
                DEBUG_PRINT("TX Transfer Completed on channel %d\n",
                        channelNum);
                break;
            default:
                DEBUG_PRINT("Unknown response 0x%02X\n", eventCode);
                break;
        }
    }

    DEBUG_PRINT("currentState = %d\n", currentState);
    return NOERROR;
}

int ANTChannel::changeStateTo(int state) {
    DEBUG_PRINT("Changing State to %d\n", state);

    switch (state) {
        case STATE_ASSIGNED:
            iface->assignChannel(channelNum, channelType,
                    network, channelTypeExtended);
            break;
        case STATE_ID_SET:
            iface->setChannelID(channelNum, deviceId,
                    deviceParams.deviceType, 0);
            break;
        case STATE_SET_TIMEOUT:
            iface->setSearchTimeout(channelNum, searchTimeout);
            break;
        case STATE_SET_PERIOD:
            iface->setChannelPeriod(channelNum,
                    deviceParams.devicePeriod);
            break;
        case STATE_SET_FREQ:
            iface->setChannelFreq(channelNum,
                    deviceParams.deviceFrequency);
            break;
        case STATE_OPEN_UNPAIRED:
            iface->openChannel(channelNum, true);
            break;
        default:
            DEBUG_PRINT("Unknown State %d\n", state);
            break;
    }
    return NOERROR;
}

int ANTChannel::processId(ANTMessage *m) {
    // Parse the ID
    uint16_t id;
    uint8_t type;
    uint8_t chan = m->getChannel();
    id  = (m->getData(1) << 8);
    id |= m->getData(0);
    type = m->getData(2);

    DEBUG_PRINT("Processed Device ID 0x%04X type 0x%02X on channel %d\n",
            id, type, chan);

    return NOERROR;
}

int ANTChannel::start(int type, uint16_t id,
        bool scanning, bool wait) {
    // Start a channel config

    deviceId = id;
    channelType = CHANNEL_TYPE_RX;
    setType(type);

    if (scanning) {
        channelTypeExtended = CHANNEL_TYPE_EXT_BACKGROUND_SCAN;
    }

    // Claim the channel.

    if (currentState != STATE_IDLE) {
        DEBUG_PRINT("Cannot start channel when not IDLE"
        "(current state = %d)\n", currentState);
        return ERROR;
    }

    // Start with ASSIGNING the channel

    changeStateTo(STATE_ASSIGNED);

    // Spinlock until timeout
    auto start = ant_clock::now();
    if (wait) {
        while ((currentState != STATE_OPEN_UNPAIRED)
                && (currentState != STATE_OPEN_PAIRED)) {
            usleep(ANTPLUS_SLEEP_DURATION);
            auto t = std::chrono::duration_cast<std::chrono::seconds>
                (ant_clock::now() - start).count();
            if (t > channelStartTimeout) {
                return ERROR;
            }
        }
    }

    return NOERROR;
}

