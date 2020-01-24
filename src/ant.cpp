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

ANT::ANT(ANTInterface *interface, int nChannels) {
    threadRun     = false;
    pollTime      = 2000;  // ms
    extMessages   = true;

    iface = interface;

    DEBUG_PRINT("Creating %d channels.\n", nChannels);
    for (int i=0; i < nChannels; i++) {
        antChannel.push_back(ANTChannel(ANTChannel::TYPE_NONE, i));
    }

    // Set the start time
    startTime = Clock::now();
}

ANT::~ANT(void) {
    // if (antChannel != nullptr) {
    //     delete [] antChannel;
    // }
}

int ANT::reset(void) {
    DEBUG_COMMENT("Sending ANT_SYSTEM_RESET\n");
    ANTMessage resetMessage(ANT_SYSTEM_RESET, 0);
    iface->sendMessage(&resetMessage);

    usleep(RESET_DURATION);

    ANTMessage reply;
    // iface->readMessage(&reply);
    // reply.parse();

    return 0;
}

int ANT::setNetworkKey(uint8_t net) {
    uint8_t key[] = ANT_NETWORK_KEY;

    DEBUG_COMMENT("Sending ANT_SET_NETWORK\n");
    ANTMessage netkey(ANT_SET_NETWORK, net, key, ANT_NETWORK_KEY_LEN);
    return iface->sendMessage(&netkey);
}

int ANT::assignChannel(uint8_t chanNum, uint8_t chanType,
        uint8_t net, uint8_t ext) {
    DEBUG_COMMENT("Sending ANT_UNASSIGN_CHANNEL\n");
    ANTMessage unassign(ANT_UNASSIGN_CHANNEL, chanNum);
    iface->sendMessage(&unassign);

    DEBUG_COMMENT("Sending ANT_ASSIGN_CHANNEL\n");
    ANTMessage assign(ANT_ASSIGN_CHANNEL, chanNum, chanType, net, ext);
    return iface->sendMessage(&assign);
}

int ANT::setChannelID(uint8_t chan, uint16_t device,
        uint8_t type, bool master) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_ID\n");
    ANTMessage setid(ANT_CHANNEL_ID, chan,
            (uint8_t)(device & 0xFF), (uint8_t)(device>>8),
            (uint8_t)type,
            (uint8_t)(master ? ANT_TX_TYPE_MASTER : ANT_TX_TYPE_SLAVE));

    return iface->sendMessage(&setid);
}

int ANT::setSearchTimeout(uint8_t chan, uint8_t timeout) {
    int rc;

    DEBUG_COMMENT("Sending ANT_SEARCH_TIMEOUT\n");

    ANTMessage hpTimeout(ANT_SEARCH_TIMEOUT, chan, 0);
    if (!(rc = iface->sendMessage(&hpTimeout))) {
        return rc;
    }

    DEBUG_COMMENT("Sending ANT_LP_SEARCH_TIMEOUT\n");
    ANTMessage lpTimeout(ANT_LP_SEARCH_TIMEOUT, chan, timeout);
    return iface->sendMessage(&lpTimeout);
}

int ANT::setChannelPeriod(uint8_t chan, uint16_t period) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_PERIOD\n");
    ANTMessage chanPeriod(ANT_CHANNEL_PERIOD, chan,
            period & 0xFF, (period >> 8) & 0xFF);
    return iface->sendMessage(&chanPeriod);
}

int ANT::setChannelFreq(uint8_t chan, uint8_t frequency) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_FREQUENCY\n");
    ANTMessage chanFreq(ANT_CHANNEL_FREQUENCY, chan, frequency);
    return iface->sendMessage(&chanFreq);
}

int ANT::setLibConfig(uint8_t chan, uint8_t config) {
    DEBUG_COMMENT("Sending ANT_LIB_CONFIG\n");
    (void)chan;  // Ignore channel ....
    ANTMessage libConfig(ANT_LIB_CONFIG, 0x00, config);
    return iface->sendMessage(&libConfig);
}

int ANT::requestDataPage(uint8_t chan, uint8_t page) {
    DEBUG_COMMENT("Sending ANT_ACK_DATA for \"Request Data Page\"\n");
    uint8_t req[8] = { 0x46, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, page, 0x01};
    ANTMessage request(ANT_ACK_DATA, chan, req, sizeof(req));
    return iface->sendMessage(&request);
}

int ANT::openChannel(uint8_t chan) {
    if (extMessages) {
        // Set the LIB Config before opening channel
        // if we want to get extended messages
        setLibConfig(chan, 0x80);
    }

    DEBUG_COMMENT("Sending ANT_OPEN_CHANNEL\n");
    ANTMessage open(ANT_OPEN_CHANNEL, chan);
    return iface->sendMessage(&open);
}

int ANT::requestMessage(uint8_t chan, uint8_t message) {
    DEBUG_PRINT("Sending ANT_REQ_MESSAGE 0x%02X\n", message);
    ANTMessage req(ANT_REQ_MESSAGE, chan, message);
    return iface->sendMessage(&req);
}

int ANT::startThreads(void) {
    threadRun = true;

    DEBUG_COMMENT("Starting listener thread ...\n");
    pthread_create(&listenerId, NULL, callListenerThread, (void *)this);

    DEBUG_COMMENT("Starting Poller Thread ...\n");
    pthread_create(&pollerId, NULL, callPollerThread, (void *)this);
    return NOERROR;
}

int ANT::stopThreads(void) {
    DEBUG_COMMENT("Stopping threads.....\n");
    threadRun = false;

    pthread_join(listenerId, NULL);
    DEBUG_COMMENT("Listener Thread Joined.\n");

    pthread_join(pollerId, NULL);
    DEBUG_COMMENT("Poller Thread Joined.\n");
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
            for (ANTChannel& chan : antChannel) {
                int state = chan.getState();
                if ((state == ANTChannel::STATE_OPEN_UNPAIRED) ||
                        (state == ANTChannel::STATE_OPEN_PAIRED)) {
                    if (chan.getType() == ANTChannel::TYPE_FEC) {
                        requestDataPage(chan.getChannelNum(),
                                ANT_DEVICE_COMMON_STATUS);
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
        for (auto & m : message) {
            switch (m.getType()) {
                case ANT_NOTIF_STARTUP:
                    DEBUG_COMMENT("RESET OK\n");
                    break;
                case ANT_CHANNEL_EVENT:
                    channelProcessEvent(&m);
                    break;
                case ANT_CHANNEL_ID:
                    channelProcessID(&m);
                    break;
                case ANT_BROADCAST_DATA:
                case ANT_ACK_DATA:
                    channelProcessBroadcast(&m);
                    break;
                default:
                    DEBUG_PRINT("UNKNOWN TYPE 0x%02X\n",
                            m.getType());
                    break;
            }
        }
    }

    return NULL;
}

int ANT::channelChangeStateTo(uint8_t chan, int state) {
    DEBUG_PRINT("chan = %d, state = %d\n", chan, state);
    ANTChannel& antChannel = getChannel(chan);
    switch (state) {
        case ANTChannel::STATE_ASSIGNED:
            assignChannel(chan,
                    antChannel.getChannelType(),
                    antChannel.getNetwork(),
                    antChannel.getExtended());
            break;
        case ANTChannel::STATE_ID_SET:
            setChannelID(chan, antChannel.getDeviceID(),
                    antChannel.getDeviceType(), 0);
            break;
        case ANTChannel::STATE_SET_TIMEOUT:
            setSearchTimeout(chan,
                    antChannel.getSearchTimeout());
            break;
        case ANTChannel::STATE_SET_PERIOD:
            setChannelPeriod(chan,
                    antChannel.getDevicePeriod());
            break;
        case ANTChannel::STATE_SET_FREQ:
            setChannelFreq(chan,
                    antChannel.getDeviceFrequency());
            break;
        case ANTChannel::STATE_OPEN_UNPAIRED:
            openChannel(chan);
            break;
        default:
            DEBUG_PRINT("Unknown State %d\n", state);
            break;
    }
    return NOERROR;
}

int ANT::channelStart(uint8_t chan, int type,
        uint16_t id, bool scanning, bool wait) {
    // Start a channel config

    ANTChannel& antChannel = getChannel(chan);
    antChannel.setDeviceID(id);
    antChannel.setType(type);
    antChannel.setChannelType(CHANNEL_TYPE_RX);

    if (scanning) {
        antChannel.setExtended(CHANNEL_TYPE_EXT_BACKGROUND_SCAN);
    }

    // Claim the channel.

    if (antChannel.getState() !=
            ANTChannel::STATE_IDLE) {
        DEBUG_PRINT("Cannot start channel when not IDLE"
        "(current state = %d)\n",
                antChannel.getState());
        return ERROR;
    }

    // Ok the next level is ASSIGNED

    channelChangeStateTo(chan, ANTChannel::STATE_ASSIGNED);

    // If we wait .. spinlock until the channel is open
    // TODO(swilkins) : We should add a timeout

    if (wait) {
        int state = antChannel.getState();
        while ((state != ANTChannel::STATE_OPEN_UNPAIRED)
                && (state != ANTChannel::STATE_OPEN_PAIRED)) {
            usleep(SLEEP_DURATION);
            state = antChannel.getState();
        }
    }

    return NOERROR;
}

int ANT::channelProcessID(ANTMessage *m) {
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

int ANT::channelProcessEvent(ANTMessage *m) {
    // 2nd Byte is event code
    uint8_t chan = m->getChannel();
    uint8_t commandCode = m->getData(0);

    ANTChannel &antChannel = getChannel(m->getChannel());

    if (commandCode != 0x01) {
        switch (commandCode) {
            case ANT_ASSIGN_CHANNEL:
                antChannel.setState(ANTChannel::STATE_ASSIGNED);
                channelChangeStateTo(chan, ANTChannel::STATE_ID_SET);
                break; case ANT_CHANNEL_ID:
                antChannel.setState(ANTChannel::STATE_ID_SET);
                channelChangeStateTo(chan, ANTChannel::STATE_SET_TIMEOUT);
                break;
            case ANT_LP_SEARCH_TIMEOUT:
                antChannel.setState(
                        ANTChannel::STATE_SET_TIMEOUT);
                channelChangeStateTo(chan, ANTChannel::STATE_SET_PERIOD);
                break;
            case ANT_CHANNEL_PERIOD:
                antChannel.setState(ANTChannel::STATE_SET_PERIOD);
                channelChangeStateTo(chan, ANTChannel::STATE_SET_FREQ);
                break;
            case ANT_CHANNEL_FREQUENCY:
                antChannel.setState(ANTChannel::STATE_SET_FREQ);
                channelChangeStateTo(chan,
                        ANTChannel::STATE_OPEN_UNPAIRED);
                break;
            case ANT_OPEN_CHANNEL:
                // Do nothing, but set state
                antChannel.setState(
                        ANTChannel::STATE_OPEN_UNPAIRED);
                break;
            default:
                DEBUG_PRINT("Unknown command 0x%02X\n", commandCode);
                break;
        }
    } else {
        uint8_t eventCode = m->getData(1);
        switch (eventCode) {
            case EVENT_RX_SEARCH_TIMEOUT:
                DEBUG_PRINT("Search timeout on channel %d\n", chan);
                antChannel.setState(ANTChannel::STATE_OPEN_UNPAIRED);
                break;
            case EVENT_RX_FAIL:
                DEBUG_PRINT("RX Failed on channel %d\n", chan);
                break;
            case EVENT_TX:
                DEBUG_PRINT("TX on channel %d\n", chan);
                break;
            case EVENT_TRANSFER_RX_FAILED:
                DEBUG_PRINT("RX Transfer Completed on channel %d\n", chan);
                break;
            case EVENT_TRANSFER_TX_COMPLETED:
                DEBUG_PRINT("TX Transfer Completed on channel %d\n", chan);
                break;
            default:
                DEBUG_PRINT("Unknown response 0x%02X\n", eventCode);
                break;
        }
    }

    return NOERROR;
}

int ANT::channelProcessBroadcast(ANTMessage *m) {
    // Parse the ID
    uint8_t chan = m->getChannel();
    getChannel(chan).parseMessage(m);
    return NOERROR;
}
