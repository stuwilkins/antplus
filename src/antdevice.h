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

using std::chrono::duration_cast;
using std::chrono::milliseconds;

#include <vector>
#include <chrono>
#include <string>

#include "antmessage.h"
#include "debug.h"

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;

class ANTDeviceDatum {
 public:
    ANTDeviceDatum(void) {
        value = 0;
    }
    ANTDeviceDatum(float v, time_point<Clock> t) {
        setDatum(v, t);
    }
    void setDatum(float v, time_point<Clock> t) {
        value = v;
        ts = t;
    }
    float             getValue(void)     { return value; }
    time_point<Clock> getTimestamp(void) { return ts; }
 private:
    float value;
    time_point<Clock> ts;
};

class ANTDevice {
 public:
    ANTDevice(void);
    ANTDevice(int nMeas, int nMetaMeas, const ANTDeviceID &id);
    ~ANTDevice(void);

    friend bool operator== (
            const ANTDevice &a, const ANTDevice &b) {
        return a.devID == b.devID;
    }
    friend bool operator== (
            const ANTDevice &a, const ANTDeviceID &b) {
        return a.devID == b;
    }

    void                         addDatum(int i, ANTDeviceDatum val);
    void                         addMetaDatum(int i, float val);
    virtual void                 parseMessage(ANTMessage *message);
    std::vector<ANTDeviceDatum>& getTsData(int i);
    ANTDeviceDatum               getData(int i);
    std::string                  getDeviceName(void);
    std::vector<std::string>&    getValueNames(void) { return valueNames; }
    int                          getNumValues(void)  { return nValues; }
    ANTDeviceID                  getDeviceID(void)   { return devID; }

 private:
    enum {
        HW_REVISION     = 0,
        MANUFACTURER_ID = 1,
        MODEL_NUMBER    = 2,
        SERIAL_NUMBER   = 3
    };

    int nValues;
    int nMetaValues;
    ANTDeviceDatum *data;
    std::vector<ANTDeviceDatum> *tsData;
    float *metaData;
    ANTDeviceID devID;

 protected:
    std::vector<std::string> valueNames;
    std::vector<std::string> metaNames;
    std::string              deviceName;
};

class ANTDeviceNONE : public ANTDevice {
 public:
    explicit ANTDeviceNONE(const ANTDeviceID &id);
    virtual ~ANTDeviceNONE(void) {}
    void parseMessage(ANTMessage *message) {
        UNUSED(message);
    }

 private:
    uint8_t lastCommandSeq;
};

class ANTDeviceFEC : public ANTDevice {
 public:
    enum {
        GENERAL_INST_SPEED        = 0,
        SETTINGS_CYCLE_LENGTH     = 1,
        SETTINGS_RESISTANCE       = 2,
        SETTINGS_INCLINE          = 3,
        TRAINER_CADENCE           = 4,
        TRAINER_ACC_POWER         = 5,
        TRAINER_INST_POWER        = 6,
        TRAINER_STATUS            = 7,
        TRAINER_FLAGS             = 8,
        TRAINER_TARGET_RESISTANCE = 9,
        TRAINER_TARGET_POWER      = 10
    };
    explicit ANTDeviceFEC(const ANTDeviceID &id);
    virtual ~ANTDeviceFEC(void) {}
    void parseMessage(ANTMessage *message);

 private:
    uint8_t lastCommandSeq;
};

class ANTDevicePWR : public ANTDevice {
 public:
    explicit ANTDevicePWR(const ANTDeviceID &id);
    virtual ~ANTDevicePWR(void) {}
    void parseMessage(ANTMessage *message);

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

class ANTDeviceHR : public ANTDevice {
 public:
    explicit ANTDeviceHR(const ANTDeviceID &id);
    virtual ~ANTDeviceHR(void) {}
    void parseMessage(ANTMessage *message);

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
