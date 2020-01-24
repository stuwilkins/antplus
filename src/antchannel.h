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

#ifndef SRC_ANTCHANNEL_H_
#define SRC_ANTCHANNEL_H_

#include <set>
#include <vector>
#include "antdevice.h"

class ANTChannelParams {
 public:
    int type;
    uint8_t deviceType;
    uint16_t devicePeriod;
    uint8_t deviceFrequency;
};

class ANTChannel {
 public:
    enum {
        // The return values
        NOERROR     = 0,
        ERROR       = -1,
        ERROR_STATE = 1
    };
    enum {
        // The channel types
        TYPE_NONE = 0,
        TYPE_HR   = 1,
        TYPE_PWR  = 2,
        TYPE_FEC  = 3,
        TYPE_PAIR = 4
    };
    enum {
        // The state machine to define
        // the various states for an ANT+ channel
        STATE_IDLE           = 0,
        STATE_ASSIGNED       = 1,
        STATE_ID_SET         = 2,
        STATE_SET_TIMEOUT    = 3,
        STATE_SET_PERIOD     = 4,
        STATE_SET_FREQ       = 5,
        STATE_SET_LIB_CONFIG = 6,
        STATE_OPEN_UNPAIRED  = 7,
        STATE_OPEN_PAIRED    = 8,
        STATE_CLOSED         = 9
    };

    ANTChannel(void);
    ANTChannel(int type, int num);
    ~ANTChannel(void);

    int        getChannelNum(void)          { return channelNum; }
    void       setType(int t);
    uint8_t    getNetwork(void)             { return network; }
    void       setExtended(uint8_t ext)     { extended = ext; }
    uint8_t    getExtended(void)            { return extended; }
    uint8_t    getChannelType(void)         { return channelType; }
    void       setChannelType(uint8_t type) { channelType = type; }
    uint16_t   getSearchTimeout(void)       { return searchTimeout; }
    int        getType(void)                { return type; }
    int        getState(void)               { return currentState; }
    uint16_t   getDeviceID(void)            { return deviceId; }
    void       setDeviceID(uint16_t id)     { deviceId = id; }
    uint8_t    getDeviceType(void)          { return deviceType; }
    uint16_t   getDevicePeriod(void)        { return devicePeriod; }
    uint16_t   getDeviceFrequency(void)     { return deviceFrequency; }

    void       setState(int state);

    void       parseMessage(ANTMessage *message);

    ANTDevice* addDevice(ANTDeviceID *id);
    std::vector<ANTDevice*> getDeviceList(void) {
        return deviceList;
    }

 private:
    uint8_t  network;
    int      currentState;
    uint8_t  channelType;
    int      channelNum;
    uint16_t deviceId;
    uint8_t  deviceType;
    uint16_t devicePeriod;
    uint8_t  deviceFrequency;
    uint8_t  searchTimeout;
    uint8_t  extended;
    int      type;

    std::vector<ANTDevice*> deviceList;

    static ANTChannelParams params[5];
};

#endif  // SRC_ANTCHANNEL_H_
