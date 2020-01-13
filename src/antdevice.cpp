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

AntDeviceParams AntDevice::params[] = {
    {  AntDevice::TYPE_HR,   0x78, 0x1F86, 0x39 },
    {  AntDevice::TYPE_PWR,  0x0B, 0x1FF6, 0x39 },
    {  AntDevice::TYPE_FEC,  0x11, 0x2000, 0x39 },
    {  AntDevice::TYPE_PAIR, 0x00, 0x0000, 0x39 },
    {  AntDevice::TYPE_NONE, 0x00, 0x0000, 0x00 }
};

AntDevice::AntDevice(int n) {
    // Create the list of values
    nValues += n;
    data = new std::vector<AntDeviceDatum>[nValues];
}

AntDevice::~AntDevice(void) {
    if (data != nullptr) {
        delete [] data;
    }
}

void AntDevice::addDatum(int i, AntDeviceDatum val) {
    data[i].push_back(val);
}

std::vector<AntDeviceDatum>& AntDevice::getData(int i) {
    return data[i];
}

std::string AntDevice::getDeviceName(void) {
    return deviceName;
}

AntDeviceFEC::AntDeviceFEC(void)
     : AntDevice(11) {
    deviceName = std::string("FE-C");
    valueNames.push_back("GENERAL_INST_SPEED");
    valueNames.push_back("SETTINGS_CYCLE_LENGTH");
    valueNames.push_back("SETTINGS_RESISTANCE");
    valueNames.push_back("SETTINGS_INCLINE");
    valueNames.push_back("TRAINER_CADENCE");
    valueNames.push_back("TRAINER_ACC_POWER");
    valueNames.push_back("TRAINER_INST_POWER");
    valueNames.push_back("TRAINER_STATUS");
    valueNames.push_back("TRAINER_FLAGS");
    valueNames.push_back("TRAINER_TARGET_RESISTANCE");
    valueNames.push_back("TRAINER_TARGET_POWER");

    lastCommandSeq = 0xFF;
}


void AntDeviceFEC::parseMessage(AntMessage *message) {
    uint8_t *data = message->getData();
    int dataLen = message->getDataLen();
    time_point<Clock> ts = message->getTimestamp();

    if (dataLen < 8) {
        DEBUG_COMMENT("Invalid FEC Message\n");
        return;
    }

    if (data[0] == ANT_DEVICE_FEC_GENERAL) {
        uint16_t _instSpeed;
        _instSpeed  = data[4];
        _instSpeed |= (data[5] << 8);
        float instSpeed = (float)_instSpeed * 0.001;

        addDatum(GENERAL_INST_SPEED, AntDeviceDatum(instSpeed, ts));

        DEBUG_PRINT("FE-C General, %f\n", instSpeed);

    } else if (data[0] == ANT_DEVICE_FEC_GENERAL_SETTINGS) {
        float cycleLength = (float)data[3] * 0.01;
        int16_t _incline;
        _incline  = data[4];
        _incline |= (data[5] << 8);
        float incline = (float)_incline * 0.01;
        float resistance = (float)data[6] * 0.5;

        addDatum(SETTINGS_CYCLE_LENGTH, AntDeviceDatum(cycleLength, ts));
        addDatum(SETTINGS_RESISTANCE, AntDeviceDatum(resistance, ts));
        addDatum(SETTINGS_INCLINE, AntDeviceDatum(incline, ts));

        DEBUG_PRINT("FE-C General Data, %f, %f, %f\n",
                cycleLength, resistance, incline);

    } else if (data[0] == ANT_DEVICE_FEC_TRAINER) {
        uint8_t cadence = data[2];
        uint16_t accPower;
        accPower  = data[3];
        accPower |= (data[4] << 8);
        uint16_t instPower;
        instPower  = data[5];
        instPower |= ((data[6] & 0x0F) << 8);
        uint8_t trainerStatus = (data[6] >> 4);
        uint8_t trainerFlags = data[7] & 0x0F;

        addDatum(TRAINER_CADENCE, AntDeviceDatum((float)cadence, ts));
        addDatum(TRAINER_ACC_POWER, AntDeviceDatum((float)accPower, ts));
        addDatum(TRAINER_INST_POWER, AntDeviceDatum((float)instPower, ts));
        addDatum(TRAINER_STATUS, AntDeviceDatum((float)trainerStatus, ts));
        addDatum(TRAINER_FLAGS, AntDeviceDatum((float)trainerFlags, ts));

        DEBUG_PRINT("FE-C Trainer Data, %d, %d, %d, 0x%02X, 0x%02X\n",
                cadence, accPower, instPower, trainerStatus, trainerFlags);

    } else if (data[0] == ANT_DEVICE_COMMON_STATUS) {
        // This gives us the requested control.
        if (data[3] == 0x00) {
            // Now check if sequence has changed
            if (data[2] != lastCommandSeq) {
                if (data[1] == ANT_DEVICE_FEC_COMMAND_RESISTANCE) {
                    uint8_t resistance = data[7];

                    addDatum(TRAINER_TARGET_RESISTANCE,
                            AntDeviceDatum(resistance, ts));

                    DEBUG_PRINT("FE-C Target Resistance, %d\n", resistance);

                } else if (data[1] == ANT_DEVICE_FEC_COMMAND_POWER) {
                    uint16_t pwr;
                    pwr  = data[7] << 8;
                    pwr |= data[6];

                    addDatum(TRAINER_TARGET_POWER,
                            AntDeviceDatum(pwr, ts));

                    DEBUG_PRINT("FE-C Target Power, %d\n", pwr);
                }
            } else {
                DEBUG_COMMENT("FE-C No new command\n");
            }
        } else {
            DEBUG_COMMENT("FE-C Last command invalid or uninitialized\n");
        }
    } else {
        DEBUG_PRINT("Unknown FEC Page 0x%02X\n", data[0]);
    }
}

AntDevicePWR::AntDevicePWR(void)
    : AntDevice(15) {
    deviceName = std::string("POWER");
    valueNames.push_back("BALANCE");
    valueNames.push_back("CADENCE");
    valueNames.push_back("ACC_POWER");
    valueNames.push_back("INST_POWER");
    valueNames.push_back("LEFT_TE");
    valueNames.push_back("RIGHT_TE");
    valueNames.push_back("LEFT_PS");
    valueNames.push_back("RIGHT_PS");
    valueNames.push_back("N_BATTERIES");
    valueNames.push_back("OPERATING_TIME");
    valueNames.push_back("BATTERY_VOLTAGE");
    valueNames.push_back("CRANK_LENGTH");
    valueNames.push_back("CRANK_STATUS");
    valueNames.push_back("SENSOR_STATUS");
    valueNames.push_back("PEAK_TORQUE_THRESHOLD");
}

void AntDevicePWR::parseMessage(AntMessage *message) {
    uint8_t *data = message->getData();
    int dataLen = message->getDataLen();
    time_point<Clock> ts = message->getTimestamp();

    if (dataLen < 8) {
        DEBUG_COMMENT("Invalid Power Message\n");
        return;
    }

    if (data[0] == ANT_DEVICE_POWER_STANDARD) {
        uint8_t balance = data[2];
        if ((balance & 0x80) && (balance != 0xFF)) {
            // We have balance data
            balance = balance & 0x7F;
            addDatum(BALANCE, AntDeviceDatum(balance, ts));
        }
        uint8_t cadence = data[3];
        addDatum(CADENCE, AntDeviceDatum(cadence, ts));

        uint16_t accPower = data[4];
        accPower |= (data[5] << 8);
        addDatum(ACC_POWER, AntDeviceDatum(accPower, ts));

        uint16_t instPower = data[6];
        instPower |= (data[7] << 8);
        addDatum(INST_POWER, AntDeviceDatum(instPower, ts));

        DEBUG_PRINT("POWER Standard, %d, %d, %d, %d\n", balance, cadence,
                accPower, instPower);
    } else if (data[0] == ANT_DEVICE_POWER_TEPS) {
        float leftTE = (float)data[2] * 0.5;
        float rightTE = (float)data[3] * 0.5;
        float leftPS = (float)data[4] * 0.5;
        float rightPS = (float)data[5] * 0.5;

        addDatum(LEFT_TE, AntDeviceDatum(leftTE, ts));
        addDatum(RIGHT_TE, AntDeviceDatum(rightTE, ts));
        addDatum(LEFT_PS, AntDeviceDatum(leftPS, ts));
        addDatum(RIGHT_PS, AntDeviceDatum(rightPS, ts));
        DEBUG_PRINT("POWER TEPS, %f, %f, %f, %f\n", leftTE, rightTE,
                leftPS, rightPS);
    } else if (data[0] == ANT_DEVICE_POWER_BATTERY) {
        uint8_t nBatteries = data[2] & 0x0F;
        uint32_t operatingTime = data[3];
        operatingTime |= (data[4] << 8);
        operatingTime |= (data[5] << 16);
        uint8_t batteryVoltage = data[6];

        addDatum(N_BATTERIES, AntDeviceDatum(nBatteries, ts));
        addDatum(OPERATING_TIME, AntDeviceDatum(operatingTime, ts));
        addDatum(BATTERY_VOLTAGE, AntDeviceDatum(batteryVoltage, ts));

        DEBUG_PRINT("POWER Battery, %d, %d, %d\n", nBatteries,
                operatingTime, batteryVoltage);
    } else if (data[0] == ANT_DEVICE_POWER_PARAMS) {
        if (data[1] == ANT_DEVICE_POWER_PARAMS_CRANK) {
            float crankLength = (float)data[4];
            crankLength = (crankLength * 0.5) + 110.0;
            uint8_t crankStatus = data[5] & 0x03;
            uint8_t sensorStatus = (data[6] >> 3) & 0x01;
            DEBUG_PRINT("POWER Params Crank, %f, %d, %d\n",
                    crankLength, crankStatus, sensorStatus);
            addDatum(CRANK_LENGTH, AntDeviceDatum(crankLength, ts));
            addDatum(CRANK_STATUS, AntDeviceDatum(crankStatus, ts));
            addDatum(SENSOR_STATUS, AntDeviceDatum(sensorStatus, ts));
        } else if (data[1] == ANT_DEVICE_POWER_PARAMS_TORQUE) {
            float peakTorqueThresh = (float)data[7] * 0.5;
            addDatum(PEAK_TORQUE_THRESHOLD,
                    AntDeviceDatum(peakTorqueThresh, ts));
            DEBUG_PRINT("POWER Params Torque, %f\n", peakTorqueThresh);
        } else {
            DEBUG_COMMENT("Unknown power Paramaters Page\n");
        }
    } else {
        DEBUG_PRINT("Unknown Power Page 0x%02X\n", data[0]);
    }
}

AntDeviceHR::AntDeviceHR(void)
     : AntDevice(2) {
    hbEventTime = 0;
    previousHbEventTime = 0;
    hbCount = 0;
    toggled = false;
    lastToggleBit = 0xFF;

    deviceName = std::string("HEARTRATE");
    valueNames.push_back("HEART_RATE");
    valueNames.push_back("RR_INTERVAL");
}

void AntDeviceHR::parseMessage(AntMessage *message) {
    uint8_t *data = message->getData();
    int dataLen = message->getDataLen();
    time_point<Clock> ts = message->getTimestamp();

    if (dataLen < 8) {
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
    } else {
        DEBUG_PRINT("Unknown HR Page 0x%02X\n", data[0] & 0x7F);
    }
}

