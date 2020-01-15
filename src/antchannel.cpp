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


#include "ant.h"
#include "antdevice.h"
#include "antchannel.h"
#include "debug.h"

ANTChannel::ANTChannel(void) {
    master = false;
    network = 0x00;
    deviceType = 0x00;
    deviceId = 0x00;
    devicePeriod = 0x00;
    deviceFrequency = 0x00;
    searchTimeout = 0x05;

    // The state of this channel
    currentState = STATE_IDLE;

    type      = ANTDevice::TYPE_NONE;
}

ANTChannel::ANTChannel(int type)
    : ANTChannel() {
    setType(type);
}

void ANTChannel::setType(int t) {
    type = t;

    int i = 0;
    while (ANTDevice::params[i].type != ANTDevice::TYPE_NONE) {
        if (ANTDevice::params[i].type == type) {
            deviceType = ANTDevice::params[i].deviceType;
            devicePeriod = ANTDevice::params[i].devicePeriod;
            deviceFrequency = ANTDevice::params[i].deviceFrequency;
            break;
        }
        i++;
    }
}

void ANTChannel::setState(int state) {
    currentState = state;
}

void ANTChannel::addDeviceId(uint16_t devid) {
    deviceIdSet.insert(devid);
}

ANTDevice* ANTChannel::getDevice(void) {
    switch (type) {
        case ANTDevice::TYPE_HR:
            return &deviceHR;
            break;
        case ANTDevice::TYPE_PWR:
            return &devicePWR;
            break;
        case ANTDevice::TYPE_FEC:
            return &deviceFEC;
            break;
    }

    return nullptr;
}

