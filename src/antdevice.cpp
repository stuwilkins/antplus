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
#include "debug.h"

AntDevice::AntDevice(int nMeas) {
    // Create the list of values
    data = new std::list<AntDeviceDatum>[nMeas];
}

AntDevice::~AntDevice(void) {
    delete [] data;
}

void AntDevice::addDatum(int i, AntDeviceDatum val) {
    data[i].push_back(val);
}

AntDeviceFEC::AntDeviceFEC(void)
     : AntDevice(7) {
}

void AntDeviceFEC::parseMessage(AntMessage *message) {
    uint8_t *data = message->getData();
    int dataLen = message->getDataLen();
    time_point<Clock> ts = message->getTimestamp();

    if (dataLen != 9) {
        DEBUG_COMMENT("Invalid FEC Message\n");
        return;
    }

    if (data[0] == ANT_DEVICE_FEC_GENERAL) {
            uint16_t instSpeed = data[4];
            instSpeed |= (data[5] << 8);
            addDatum(INST_SPEED, AntDeviceDatum(instSpeed, ts));
    } else if (data[0] == ANT_DEVICE_FEC_GENERAL_SETTINGS) {
            uint8_t cycleLength = data[3];
            uint8_t resistance = data[6];
            uint16_t incline = data[4];
            incline |= (data[5] << 8);
            addDatum(CYCLE_LENGTH, AntDeviceDatum(cycleLength, ts));
            addDatum(RESISTANCE, AntDeviceDatum(resistance, ts));
            addDatum(INCLINE, AntDeviceDatum(incline, ts));
            DEBUG_PRINT("FE-C General Data, %d, %d, %d\n",
                    cycleLength, resistance, incline);
    } else if (data[0] == ANT_DEVICE_FEC_TRAINER) {
            uint8_t cadence = data[2];
            uint16_t accPower = data[3];
            accPower |= (data[4] << 8);
            uint16_t instPower = data[5];
            instPower |= (data[6] << 8);
            addDatum(CADENCE, AntDeviceDatum(cadence, ts));
            addDatum(ACC_POWER, AntDeviceDatum(accPower, ts));
            addDatum(INST_POWER, AntDeviceDatum(instPower, ts));
            DEBUG_PRINT("FE-C Trainer Data, %d, %d, %d\n", cadence, accPower,
                    instPower);
    } else {
        DEBUG_PRINT("Unknown FEC Page 0x%02X\n", data[0]);
    }
}

AntDevicePWR::AntDevicePWR(void) {
    instPower = 0;
    accPower = 0;
    cadence = 0;
    balance = 0;
    leftTE = 0;
    rightTE = 0;
    leftPS = 0;
    rightPS = 0;
    nBatteries = 0;
    operatingTime = 0;
    batteryVoltage = 0;
    crankLength = 0;
    crankStatus = 0;
    sensorStatus = 0;
    peakTorqueThresh = 0;
}

void AntDevicePWR::parseMessage(AntMessage *message) {
    uint8_t *data = message->getData();
    int dataLen = message->getDataLen();

    if (dataLen != 9) {
        DEBUG_COMMENT("Invalid Power Message\n");
        return;
    }

    switch (data[0]) {
        case ANT_DEVICE_POWER_STANDARD:
            if ((data[2] & 0x80) && (data[2] != 0xFF)) {
                // We have balance data
                balance = data[2] & 0x7F;
            } else {
                balance = 0xFF;
            }
            cadence = data[3];
            accPower = data[4];
            accPower |= (data[5] << 8);
            instPower = data[6];
            instPower |= (data[7] << 8);
            DEBUG_PRINT("POWER Standard, %d, %d, %d, %d\n", balance, cadence,
                    accPower, instPower);
            break;
        case ANT_DEVICE_POWER_TEPS:
            leftTE = data[2];
            rightTE = data[3];
            leftPS = data[4];
            rightPS = data[5];
            DEBUG_PRINT("POWER TEPS, %d, %d, %d, %d\n", leftTE, rightTE,
                    leftPS, rightPS);
            break;
        case ANT_DEVICE_POWER_BATTERY:
            nBatteries = data[2] & 0x0F;
            operatingTime = data[3];
            operatingTime |= (data[4] << 8);
            operatingTime |= (data[5] << 16);
            batteryVoltage = data[6];
            DEBUG_PRINT("POWER Battery, %d, %d, %d\n", nBatteries,
                    operatingTime, batteryVoltage);
            break;
        case ANT_DEVICE_POWER_PARAMS:
            switch (data[0]) {
                case ANT_DEVICE_POWER_PARAMS_CRANK:
                    crankLength = data[4];
                    crankStatus = data[5] & 0x03;
                    sensorStatus = (data[6] >> 3) & 0x01;
                    DEBUG_PRINT("POWER Params Crank, %d, %d, %d\n",
                            crankLength, crankStatus, sensorStatus);
                    break;
                case ANT_DEVICE_POWER_PARAMS_TORQUE:
                    peakTorqueThresh = data[7];
                    DEBUG_PRINT("POWER Params Torque, %d\n", peakTorqueThresh);
                    break;
                default:
                    DEBUG_COMMENT("Unknown power Paramaters Page\n");
                    break;
            }
            break;
        default:
            DEBUG_PRINT("Unknown Power Page 0x%02X\n", data[0]);
            break;
    }
}

AntDeviceHR::AntDeviceHR(void)
     : AntDevice(2) {
    hbEventTime = 0;
    previousHbEventTime = 0;
    hbCount = 0;
    toggled = false;
    lastToggleBit = 0xFF;
}

void AntDeviceHR::parseMessage(AntMessage *message) {
    uint8_t *data = message->getData();
    int dataLen = message->getDataLen();
    time_point<Clock> ts = message->getTimestamp();

    if (dataLen != 8) {
        DEBUG_COMMENT("Invalid HR Message\n");
        return;
    }

    // Do the common section (independant of page no)

    hbEventTime = data[4];
    hbEventTime |= (data[5] << 8);
    hbCount = data[6];
    uint8_t heartRate = data[7];

    uint8_t toggleBit = (data[0] & 0x80);

    if ((lastToggleBit != toggleBit) && (lastToggleBit != 0xFF)) {
        toggled = true;
    }

    DEBUG_PRINT("HR Common, %d, %d, %d, 0x%02X, 0x%02X, %d\n",
            hbEventTime, hbCount, heartRate, lastToggleBit, toggleBit, toggled);

    lastToggleBit = toggleBit;

    addDatum(HEARTRATE, AntDeviceDatum(heartRate, ts));

    if ((data[0] & 0x7F)  == ANT_DEVICE_HR_PREVIOUS) {
        previousHbEventTime = data[2];
        previousHbEventTime |= (data[3] << 8);
        float rrInterval = (hbEventTime - previousHbEventTime);
        rrInterval *= (1000 / 1024);
        addDatum(RR_INTERVAL, AntDeviceDatum(rrInterval, ts));
        DEBUG_PRINT("HR Previous, %d, %d, %f\n", previousHbEventTime,
                hbEventTime, rrInterval);
    }
}

