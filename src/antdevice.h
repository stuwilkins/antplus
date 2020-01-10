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

#ifndef SRC_ANTDEVICE_H_
#define SRC_ANTDEVICE_H_

#define ANT_DEVICE_FEC_GENERAL              0x10
#define ANT_DEVICE_FEC_GENERAL_SETTINGS     0x11
#define ANT_DEVICE_FEC_TRAINER              0x19

#define ANT_DEVICE_POWER_STANDARD           0x10
#define ANT_DEVICE_POWER_TEPS               0x13
#define ANT_DEVICE_POWER_BATTERY            0x52
#define ANT_DEVICE_POWER_PARAMS             0x02
#define ANT_DEVICE_POWER_PARAMS_CRANK       0x01
#define ANT_DEVICE_POWER_PARAMS_TORQUE      0x02

#define ANT_DEVICE_HR_PREVIOUS              0x04

using std::chrono::duration_cast;
using std::chrono::milliseconds;

#include <list>
#include <chrono>

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;

class AntDeviceDatum {
 public:
    AntDeviceDatum(float v,
            time_point<Clock> t) {
        val = v;
        ts = t;
    }
 private:
    float val;
    time_point<Clock> ts;
};

class AntDevice {
 public:
    explicit AntDevice(int nMeas);
    ~AntDevice(void);
    void addDatum(int i, AntDeviceDatum val);
    void parseMessage(AntMessage *message);
 private:
    std::list<AntDeviceDatum> *data;
};

class AntDeviceFEC : public AntDevice {
 public:
    enum {
        INCLINE      = 0,
        RESISTANCE   = 1,
        CADENCE      = 2,
        ACC_POWER    = 3,
        INST_POWER   = 4,
        INST_SPEED   = 5,
        CYCLE_LENGTH = 6
    };
    AntDeviceFEC(void);
     void parseMessage(AntMessage *message);
};

class AntDevicePWR {
 public:
    enum {
        BALLANCE              = 0,
        CADENCE               = 1,
        ACC_POWER             = 2,
        INST_POWER            = 3,
        LEFT_TE               = 4,
        RIGHT_TE              = 5,
        LEFT_PS               = 6,
        RIGHT_PS              = 7,
        N_BATTERIES           = 8,
        OPERATING_TIME        = 9,
        BATTERY_VOLTAGE       = 10,
        CRANK_LENGTH          = 11,
        CRANK_STATUS          = 12,
        SENSOR_STATUS         = 13,
        PEAK_TORQUE_THRESHOLD = 14
    };
    AntDevicePWR(void);
    void parseMessage(AntMessage *message);

 private:
    uint8_t balance;           // %
    uint8_t cadence;           // RPM
    uint16_t accPower;         // W
    uint16_t instPower;        // W
    uint8_t leftTE;            // 0.5%
    uint8_t rightTE;           // 0.5%
    uint8_t leftPS;            // 0.5%
    uint8_t rightPS;           // 0.5%
    uint8_t nBatteries;
    uint32_t operatingTime;    // 2s or 16s
    uint8_t batteryVoltage;    // 1/256V
    uint8_t crankLength;       // X*0.5 mm + 110.0 mm
    uint8_t crankStatus;       // 00 Invalid, 01 Default
                               // 10 Manually set, 11 Auto Set
    uint8_t sensorStatus;      // SW Missmach != 0
    uint8_t peakTorqueThresh;  // 0.5%
};

class AntDeviceHR : public AntDevice {
 public:
    enum {
        HEARTRATE = 0,
        RR_INTERVAL = 1,
    };
    AntDeviceHR(void);
    void parseMessage(AntMessage *message);

 private:
    uint16_t hbEventTime;
    uint16_t previousHbEventTime;
    uint8_t hbCount;
    bool toggled;
    uint8_t lastToggleBit;
};

#endif  // SRC_ANTDEVICE_H_
