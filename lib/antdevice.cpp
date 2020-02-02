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

#include <iostream>
#include <algorithm>

#include "ant.h"
#include "antdevice.h"
#include "debug.h"

ANTDevice::ANTDevice(void) {
    pthread_mutex_init(&thread_lock, NULL);

    data = std::make_shared<tData>();
    tsData = std::make_shared<tTsData>();
    metaData = std::make_shared<tMetaData>();

    storeTsData = true;
}

ANTDevice::ANTDevice(const ANTDeviceID &id)
    : ANTDevice() {
    devID = id;
}

ANTDevice::~ANTDevice(void) {
    pthread_mutex_destroy(&thread_lock);
}

void ANTDevice::addMetaDatum(std::string name, float val) {
    (*metaData)[name] = val;
}

void ANTDevice::addMetaDatum(const char* name, float val) {
    addMetaDatum(std::string(name), val);
}

void ANTDevice::addDatum(std::string name, ANTDeviceDatum val) {
    if (storeTsData) {
        if (tsData->count(name)) {
            (*tsData)[name]->push_back(val);
        } else {
            // Create the vector
            (*tsData)[name] = std::make_shared<std::vector<ANTDeviceDatum>>();
            (*tsData)[name]->push_back(val);
        }
    }
    (*data)[name] = val;
}

void ANTDevice::addDatum(const char *name, ANTDeviceDatum val) {
    addDatum(std::string(name), val);
}

void ANTDevice::processMessage(ANTMessage *message) {
    auto data = message->getData();
    int dataLen = message->getDataLen();

    if (dataLen < 8) {
        return;
    }

    if (data[0] == ANT_DEVICE_COMMON_DATA) {
        uint8_t hwRevision;
        uint16_t manufacturerID;
        uint16_t modelNumber;

        hwRevision      = data[3];
        manufacturerID  = data[4];
        manufacturerID |= (data[5] << 8);
        modelNumber     = data[6];
        modelNumber    |= data[7];

        addMetaDatum("HW_REVISION", hwRevision);
        addMetaDatum("MANUFACTURER_ID", manufacturerID);
        addMetaDatum("MODEL_NUMBER", modelNumber);

        DEBUG_PRINT("COMMON_DATA, %d, %d, %d\n",
                hwRevision, manufacturerID, modelNumber);
    } else if (data[0] == ANT_DEVICE_COMMON_INFO) {
        uint32_t serialNumber;

        serialNumber  = data[4];
        serialNumber |= (data[5] << 8);
        serialNumber |= (data[6] << 16);
        serialNumber |= (data[7] << 24);

        addMetaDatum("SERIAL_NUMBER", serialNumber);

        DEBUG_PRINT("COMMON_INFO, %d\n", serialNumber);
    }
}

ANTDeviceNONE::ANTDeviceNONE(const ANTDeviceID &id)
    : ANTDevice(id) {
    deviceName = std::string("NONE");
}

ANTDeviceFEC::ANTDeviceFEC(const ANTDeviceID &id)
     : ANTDevice(id) {
    deviceName = std::string("FE-C");
    lastCommandSeq = 0xFF;
}


void ANTDeviceFEC::processMessage(ANTMessage *message) {
    ANTDevice::processMessage(message);

    auto data = message->getData();
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

        addDatum("GENERAL_INST_SPEED", ANTDeviceDatum(instSpeed, ts));

        DEBUG_PRINT("FE-C General, %f\n", instSpeed);

    } else if (data[0] == ANT_DEVICE_FEC_GENERAL_SETTINGS) {
        float cycleLength = (float)data[3] * 0.01;
        int16_t _incline;
        _incline  = data[4];
        _incline |= (data[5] << 8);
        float incline = (float)_incline * 0.01;
        float resistance = (float)data[6] * 0.5;

        addDatum("SETTINGS_CYCLE_LENGTH", ANTDeviceDatum(cycleLength, ts));
        addDatum("SETTINGS_RESISTANCE", ANTDeviceDatum(resistance, ts));
        addDatum("SETTINGS_INCLINE", ANTDeviceDatum(incline, ts));

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

        addDatum("TRAINER_CADENCE", ANTDeviceDatum((float)cadence, ts));
        addDatum("TRAINER_ACC_POWER", ANTDeviceDatum((float)accPower, ts));
        addDatum("TRAINER_INST_POWER", ANTDeviceDatum((float)instPower, ts));
        addDatum("TRAINER_STATUS", ANTDeviceDatum((float)trainerStatus, ts));
        addDatum("TRAINER_FLAGS", ANTDeviceDatum((float)trainerFlags, ts));

        DEBUG_PRINT("FE-C Trainer Data, %d, %d, %d, 0x%02X, 0x%02X\n",
                cadence, accPower, instPower, trainerStatus, trainerFlags);

    } else if (data[0] == ANT_DEVICE_COMMON_STATUS) {
        // This gives us the requested control.
        if (data[3] == 0x00) {
            // Now check if sequence has changed
            uint8_t commandSeq = data[2];
            if (commandSeq != lastCommandSeq) {
                lastCommandSeq = commandSeq;

                if (data[1] == ANT_DEVICE_FEC_COMMAND_RESISTANCE) {
                    float resistance = (float)data[7] * 0.5;

                    addDatum("TRAINER_TARGET_RESISTANCE",
                            ANTDeviceDatum(resistance, ts));

                    DEBUG_PRINT("FE-C Target Resistance, %f, %d\n",
                            resistance, commandSeq);

                } else if (data[1] == ANT_DEVICE_FEC_COMMAND_POWER) {
                    uint16_t _pwr;
                    _pwr  = data[7] << 8;
                    _pwr |= data[6];
                    float pwr = _pwr * 0.25;

                    addDatum("TRAINER_TARGET_POWER",
                            ANTDeviceDatum(pwr, ts));

                    DEBUG_PRINT("FE-C Target Power, %f, %d\n",
                            pwr, commandSeq);
                }
            } else {
                DEBUG_PRINT("FE-C No new command, %d, %d\n",
                        commandSeq, lastCommandSeq);
            }
        } else {
            DEBUG_COMMENT("FE-C Last command invalid or uninitialized\n");
        }
    } else {
        DEBUG_PRINT("Unknown FEC Page 0x%02X\n", data[0]);
    }
}

ANTDevicePWR::ANTDevicePWR(const ANTDeviceID &id)
    : ANTDevice(id) {
    deviceName = std::string("POWER");
}

void ANTDevicePWR::processMessage(ANTMessage *message) {
    ANTDevice::processMessage(message);

    auto data = message->getData();
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
            addDatum("BALANCE", ANTDeviceDatum(balance, ts));
        }
        uint8_t cadence = data[3];
        addDatum("CADENCE", ANTDeviceDatum(cadence, ts));

        uint16_t accPower = data[4];
        accPower |= (data[5] << 8);
        addDatum("ACC_POWER", ANTDeviceDatum(accPower, ts));

        uint16_t instPower = data[6];
        instPower |= (data[7] << 8);
        addDatum("INST_POWER", ANTDeviceDatum(instPower, ts));

        DEBUG_PRINT("POWER Standard, %d, %d, %d, %d\n", balance, cadence,
                accPower, instPower);
    } else if (data[0] == ANT_DEVICE_POWER_TEPS) {
        float leftTE = (float)data[2] * 0.5;
        float rightTE = (float)data[3] * 0.5;
        float leftPS = (float)data[4] * 0.5;
        float rightPS = (float)data[5] * 0.5;

        addDatum("LEFT_TE", ANTDeviceDatum(leftTE, ts));
        addDatum("RIGHT_TE", ANTDeviceDatum(rightTE, ts));
        addDatum("LEFT_PS", ANTDeviceDatum(leftPS, ts));
        addDatum("RIGHT_PS", ANTDeviceDatum(rightPS, ts));
        DEBUG_PRINT("POWER TEPS, %f, %f, %f, %f\n", leftTE, rightTE,
                leftPS, rightPS);
    } else if (data[0] == ANT_DEVICE_POWER_BATTERY) {
        uint8_t nBatteries = data[2] & 0x0F;
        uint32_t operatingTime = data[3];
        operatingTime |= (data[4] << 8);
        operatingTime |= (data[5] << 16);
        uint8_t batteryVoltage = data[6];

        addDatum("N_BATTERIES", ANTDeviceDatum(nBatteries, ts));
        addDatum("OPERATING_TIME", ANTDeviceDatum(operatingTime, ts));
        addDatum("BATTERY_VOLTAGE", ANTDeviceDatum(batteryVoltage, ts));

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

            addDatum("CRANK_LENGTH", ANTDeviceDatum(crankLength, ts));
            addDatum("CRANK_STATUS", ANTDeviceDatum(crankStatus, ts));
            addDatum("SENSOR_STATUS", ANTDeviceDatum(sensorStatus, ts));
        } else if (data[1] == ANT_DEVICE_POWER_PARAMS_TORQUE) {
            float peakTorqueThresh = (float)data[7] * 0.5;
            addDatum("PEAK_TORQUE_THRESHOLD",
                    ANTDeviceDatum(peakTorqueThresh, ts));
            DEBUG_PRINT("POWER Params Torque, %f\n", peakTorqueThresh);
        } else {
            DEBUG_COMMENT("Unknown power Paramaters Page\n");
        }
    } else {
        DEBUG_PRINT("Unknown Power Page 0x%02X\n", data[0]);
    }
}

ANTDeviceHR::ANTDeviceHR(const ANTDeviceID &id)
    : ANTDevice(id) {
    hbEventTime = 0;
    previousHbEventTime = 0;
    hbCount = 0;
    toggled = false;
    lastToggleBit = 0xFF;

    deviceName = std::string("HEARTRATE");
}

void ANTDeviceHR::processMessage(ANTMessage *message) {
    ANTDevice::processMessage(message);

    auto data = message->getData();
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

    addDatum("HEARTRATE", ANTDeviceDatum(heartRate, ts));

    uint8_t page = data[0] & 0x7F;

    if (page == ANT_DEVICE_HR_PREVIOUS) {
        previousHbEventTime = data[2];
        previousHbEventTime |= (data[3] << 8);
        float rrInterval = (hbEventTime - previousHbEventTime);
        rrInterval *= (1000 / 1024);
        addDatum("RR_INTERVAL", ANTDeviceDatum(rrInterval, ts));
        DEBUG_PRINT("HR Previous, %d, %d, %f\n", previousHbEventTime,
                hbEventTime, rrInterval);
    } else if (page == ANT_DEVICE_HR_INFO) {
        addMetaDatum("HR_HW_VERSION", data[1]);
        addMetaDatum("HR_SW_VERSION", data[2]);
        addMetaDatum("HR_MODEL_NUMBER", data[3]);
    } else if (page == ANT_DEVICE_HR_MF_INFO) {
        uint16_t serialnum;
        serialnum  = data[2];
        serialnum |= (data[3] << 8);
        addMetaDatum("HR_MANUFACTURER_ID", data[1]);
        addMetaDatum("HR_SERIAL_NUMBER", serialnum);
    } else if (page != ANT_DEVICE_HR_COMMON) {
        DEBUG_PRINT("Unknown HR Page 0x%02X\n", data[0] & 0x7F);
    }
}

