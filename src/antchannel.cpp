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

#include <algorithm>

#include "ant.h"
#include "antdevice.h"
#include "antchannel.h"
#include "debug.h"

ANTChannel::ANTChannel(void) {
    network = 0x00;
    deviceId = 0x00;
    searchTimeout = 0xFF;
    channelType = CHANNEL_TYPE_RX;
    extended = 0x00;

    // The state of this channel
    currentState = STATE_IDLE;

    type = TYPE_NONE;
}

ANTChannel::ANTChannel(int type)
    : ANTChannel() {
    setType(type);
}

ANTChannel::~ANTChannel(void) {
    for (auto dev : deviceList) {
        switch (dev->getDeviceID().getType()) {
            case ANT_DEVICE_NONE:
                delete (ANTDeviceNONE*)dev;
                break;
            case ANT_DEVICE_HR:
                delete (ANTDeviceHR*)dev;
                break;
            case ANT_DEVICE_FEC:
                delete (ANTDeviceFEC*)dev;
                break;
            case ANT_DEVICE_PWR:
                delete (ANTDevicePWR*)dev;
                break;
        }
    }
}

void ANTChannel::setType(int t) {
    channelType = t;

    DEBUG_PRINT("Setting type to %d\n", channelType);

    int i = 0;
    while (params[i].type != TYPE_NONE) {
        if (params[i].type == channelType) {
            deviceType = params[i].deviceType;
            devicePeriod = params[i].devicePeriod;
            deviceFrequency = params[i].deviceFrequency;
            DEBUG_PRINT("type = 0x%02X period = 0x%04X freq = 0x%02X\n",
                    deviceType, devicePeriod, deviceFrequency);
            break;
        }
        i++;
    }

    if (params[i].type == TYPE_NONE) {
        DEBUG_COMMENT("Failed to set channel type\n");
    }
}

ANTChannelParams ANTChannel::params[] = {
    {  ANTChannel::TYPE_HR,   0x78, 0x1F86, 0x39 },
    {  ANTChannel::TYPE_PWR,  0x0B, 0x1FF6, 0x39 },
    {  ANTChannel::TYPE_FEC,  0x11, 0x2000, 0x39 },
    {  ANTChannel::TYPE_PAIR, 0x00, 0x0000, 0x39 },
    {  ANTChannel::TYPE_NONE, 0x00, 0x0000, 0x00 },
};

void ANTChannel::setState(int state) {
    currentState = state;
}

ANTDevice* ANTChannel::addDevice(ANTDeviceID *id) {
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
        deviceList.push_back(dev);
    }

    return dev;
}

void ANTChannel::parseMessage(ANTMessage *message) {
    ANTDeviceID devID = message->getDeviceID();

    if (!devID.isValid()) {
        DEBUG_COMMENT("Processing with no device id info\n");
        return;
    }

    ANTDevice *device = nullptr;
    for (ANTDevice *dev : deviceList) {
        if (*dev == devID) {
            device = dev;
            break;
        }
    }

    if (device == nullptr) {
        device = addDevice(&devID);
    }

    device->parseMessage(message);
}
