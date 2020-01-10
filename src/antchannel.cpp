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

AntChannel::AntChannel(void) {
    chanNum = 1;
    master = false;
    network = 0x00;
    deviceType = 0x00;
    deviceId = 0x00;
    devicePeriod = 0x00;
    deviceFrequency = 0x00;
    searchTimeout = 0x00;

    // The state of this channel
    currentState = STATE_IDLE;

    type      = AntDevice::TYPE_HR;
    deviceFEC = new AntDeviceFEC;
    deviceHR  = new AntDeviceHR;
    devicePWR = new AntDevicePWR;
}

AntChannel::AntChannel(uint8_t chan, int type) {
    setChannel(chan);
    setType(type);
}

void AntChannel::setType(int type) {
    type = type;

    int i = 0;
    while (AntDevice::params[i].type != AntDevice::TYPE_NONE) {
        if (AntDevice::params[i].type == type) {
            deviceType = AntDevice::params[i].deviceType;
            devicePeriod = AntDevice::params[i].devicePeriod;
            deviceFrequency = AntDevice::params[i].deviceFrequency;
            break;
        }
    }
}


void AntChannel::setState(int state) {
    DEBUG_PRINT("Channel %d state changed to %d\n",
        chanNum, state);
    currentState = state;
}

void AntChannel::addDeviceId(uint16_t devid) {
    deviceIdSet.insert(devid);
}
