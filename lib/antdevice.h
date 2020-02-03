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

#ifndef ANT_RECORDER_LIB_ANTDEVICE_H_
#define ANT_RECORDER_LIB_ANTDEVICE_H_

using std::chrono::duration_cast;
using std::chrono::milliseconds;

#include <pthread.h>

#include <vector>
#include <chrono>
#include <string>
#include <map>
#include <memory>

#include "antmessage.h"
#include "debug.h"

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;

/**
 * @brief 
 * 
 */
class ANTDeviceData {
 public:
    ANTDeviceData(void) {
    }
    void addDatum(float v, time_point<Clock> t) {
        value.push_back(v);
        ts.push_back(t);
    }
    std::vector<float>& getValue(void) {
        return value;
    }
    std::vector<time_point<Clock>>& getTimestamp(void) {
        return ts;
    }
 private:
    std::vector<float> value;
    std::vector<time_point<Clock>> ts;
};

typedef std::map<std::string, float> tMetaData;
typedef std::map<std::string, ANTDeviceData> tTsData;

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

    tTsData getTsData(void) {
        return *tsData;
    }
    tMetaData getMetaData(void) {
        return *metaData;
    }

 private:
    std::shared_ptr<tTsData>        tsData;
    std::shared_ptr<tMetaData>      metaData;
    bool            storeTsData;
    ANTDeviceID     devID;
    pthread_mutex_t thread_lock;

 protected:
    virtual void processMessage(ANTMessage *message);

    void addDatum(std::string name, float val, time_point<Clock> t);
    void addDatum(const char *name, float val, time_point<Clock> t);
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

#endif  // ANT_RECORDER_LIB_ANTDEVICE_H_
