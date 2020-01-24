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

#include <pthread.h>

#include <vector>
#include <chrono>
#include <string>
#include <map>

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

typedef std::map<std::string, float> tMetaData;
typedef std::map<std::string, ANTDeviceDatum> tData;
typedef std::map<std::string, std::vector<ANTDeviceDatum>> tTsData;

class ANTDevice {
 public:
    ANTDevice(void);
    explicit ANTDevice(const ANTDeviceID &id);
    ~ANTDevice(void);

    friend bool operator== (
            const ANTDevice &a, const ANTDevice &b) {
        return a.devID == b.devID;
    }
    friend bool operator== (
            const ANTDevice &a, const ANTDeviceID &b) {
        return a.devID == b;
    }

    void lock(void) {
        pthread_mutex_lock(&thread_lock);
    }
    void unlock(void) {
        pthread_mutex_unlock(&thread_lock);
    }

    void parseMessage(ANTMessage *message) {
        pthread_mutex_lock(&thread_lock);
        processMessage(message);
        pthread_mutex_unlock(&thread_lock);
    }

    ANTDeviceID  getDeviceID(void)   { return devID; }
    std::string& getDeviceName(void) { return deviceName; }

    tTsData& getTsData(void) {
        return tsData;
    }
    tData& getData(void) {
        return data;
    }
    tMetaData& getMetaData(void) {
        return metaData;
    }

 private:
    tData           data;
    tTsData         tsData;
    tMetaData       metaData;
    ANTDeviceID     devID;
    pthread_mutex_t thread_lock;

 protected:
    virtual void processMessage(ANTMessage *message);

    void addDatum(std::string name, ANTDeviceDatum val);
    void addDatum(const char *name, ANTDeviceDatum val);
    void addMetaDatum(std::string name, float val);
    void addMetaDatum(const char *name, float val);

    std::vector<std::string> valueNames;
    std::vector<std::string> metaNames;
    std::string              deviceName;
};

class ANTDeviceNONE : public ANTDevice {
 public:
    explicit ANTDeviceNONE(const ANTDeviceID &id);
    virtual ~ANTDeviceNONE(void) {}
};

class ANTDeviceFEC : public ANTDevice {
 public:
    explicit ANTDeviceFEC(const ANTDeviceID &id);
    virtual ~ANTDeviceFEC(void) {}
    void processMessage(ANTMessage *message);

 private:
    uint8_t lastCommandSeq;
};

class ANTDevicePWR : public ANTDevice {
 public:
    explicit ANTDevicePWR(const ANTDeviceID &id);
    virtual ~ANTDevicePWR(void) {}
    void processMessage(ANTMessage *message);
};

class ANTDeviceHR : public ANTDevice {
 public:
    explicit ANTDeviceHR(const ANTDeviceID &id);
    virtual ~ANTDeviceHR(void) {}
    void processMessage(ANTMessage *message);

 private:
    uint16_t hbEventTime;
    uint16_t previousHbEventTime;
    uint8_t hbCount;
    bool toggled;
    uint8_t lastToggleBit;
};

#endif  // SRC_ANTDEVICE_H_
