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

#include <vector>
#include <chrono>
#include <string>

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;

struct AntDeviceParams {
    int type;
    uint8_t deviceType;
    uint16_t devicePeriod;
    uint8_t deviceFrequency;
};

class AntDeviceDatum {
 public:
    AntDeviceDatum(float v,
            time_point<Clock> t) {
        value = v;
        ts = t;
    }
    float             getValue(void)     { return value; }
    time_point<Clock> getTimestamp(void) { return ts; }
 private:
    float value;
    time_point<Clock> ts;
};

class AntDevice {
 public:
    enum {
        // The channel types
        TYPE_NONE = 0,
        TYPE_HR   = 1,
        TYPE_PWR  = 2,
        TYPE_FEC  = 3,
        TYPE_PAIR = 4
    };
    static AntDeviceParams params[5];

    explicit AntDevice(int nMeas);
    ~AntDevice(void);

    void addDatum(int i, AntDeviceDatum val);
    void parseMessage(AntMessage *message);

    std::vector<AntDeviceDatum>& getData(int i);
    std::string getDeviceName(void);
    std::vector<std::string>& getValueNames(void) { return valueNames; }
    int                       getNumValues(void)  { return nValues; }

 private:
    // enum {
    //     // DATA From Common Pages
    // }
    int nValues;
    std::vector<AntDeviceDatum> *data;

 protected:
    std::vector<std::string> valueNames;
    std::vector<std::string> metaNames;
    std::string deviceName;
};

class AntDeviceFEC : public AntDevice {
 public:
    enum {
        GENERAL_INST_SPEED    = 0,
        SETTINGS_CYCLE_LENGTH = 1,
        SETTINGS_RESISTANCE   = 2,
        SETTINGS_INCLINE      = 3,
        TRAINER_CADENCE       = 4,
        TRAINER_ACC_POWER     = 5,
        TRAINER_INST_POWER    = 6,
        TRAINER_STATUS        = 7,
        TRAINER_FLAGS         = 8
    };
    AntDeviceFEC(void);
    void parseMessage(AntMessage *message);
};

class AntDevicePWR : public AntDevice {
 public:
    AntDevicePWR(void);
    void parseMessage(AntMessage *message);

 private:
    enum {
        BALANCE              = 0,
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
};

class AntDeviceHR : public AntDevice {
 public:
    AntDeviceHR(void);
    void parseMessage(AntMessage *message);

 private:
    enum {
        HEARTRATE = 0,
        RR_INTERVAL = 1,
    };

    uint16_t hbEventTime;
    uint16_t previousHbEventTime;
    uint8_t hbCount;
    bool toggled;
    uint8_t lastToggleBit;
};

#endif  // SRC_ANTDEVICE_H_
