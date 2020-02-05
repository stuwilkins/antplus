//
// antplus : ANT+ Utilities
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
#ifndef ANTPLUS_LIB_ANTPLUS_H_
#define ANTPLUS_LIB_ANTPLUS_H_

#include <pthread.h>
#include <libusb-1.0/libusb.h>

#include <vector>
#include <queue>
#include <memory>
#include <chrono>
#include <utility>
#include <map>
#include <string>

#include "antinterface.h"
#include "antchannel.h"

#define ANTPLUS_MAX_MESSAGE_SIZE                    128
#define ANTPLUS_SLEEP_DURATION                      50000L

//
// Version / Debug info created by cmake
//

void antplus_set_debug(int d);

extern const char* ANTPLUS_GIT_REV;
extern const char* ANTPLUS_GIT_BRANCH;
extern const char* ANTPLUS_GIT_VERSION;

//
// Clocks defined ant_clocks
//

using ant_clock = std::chrono::steady_clock;
using std::chrono::time_point;

/**
 * @brief
 *
 */
class ANTDeviceID {
 public:
    ANTDeviceID(void) {
        antID   = 0x0000;
        antType = 0x00;
    }
    ANTDeviceID(uint16_t id, uint8_t type) {
        antID = id;
        antType = type;
    }
    uint16_t getID(void)   { return antID; }
    uint8_t  getType(void) { return antType; }
    bool     isValid(void) {
        return (antID != 0x000) && (antType != 0x00);
    }
    friend bool operator== (
            const ANTDeviceID &a, const ANTDeviceID &b) {
        return (a.antID == b.antID) && (a.antType == b.antType);
    }
    friend bool operator< (
            const ANTDeviceID &a, const ANTDeviceID &b) {
        return (a.antID < b.antID) && (a.antType < b.antType);
    }

 private:
    uint16_t antID;
    uint8_t antType;
};

/**
 * @brief
 *
 */
class ANTMessage {
 public:
    enum {
        NOERROR = 0,
        ERROR_LEN = -2,
        ERROR_CRC = -3,
        ERROR_PROTO = -4
    };

    ANTMessage(void);
    ANTMessage(uint8_t *data, int data_len);
    ANTMessage(uint8_t type, uint8_t chan, uint8_t *data, int len);
    ANTMessage(uint8_t type, uint8_t *data, int len);
    ANTMessage(uint8_t type, uint8_t chan);
    ANTMessage(uint8_t type, uint8_t chan, uint8_t b0);
    ANTMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1);
    ANTMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1,
            uint8_t b2);
    ANTMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1,
            uint8_t b2, uint8_t b3);
    ANTMessage(uint8_t type, uint8_t chan, uint8_t b0, uint8_t b1,
            uint8_t b2, uint8_t b3, uint8_t b4);

    // Copy Constructor
    ANTMessage(const ANTMessage& obj)
        : ANTMessage() {
        antType = obj.antType;
        antChannel = obj.antChannel;
        antDataLen = obj.antDataLen;
        antDeviceID = obj.antDeviceID;
        ts = obj.ts;
        for (int i=0; i < ANTPLUS_MAX_MESSAGE_SIZE; i++) {
            antData[i] = obj.antData[i];
        }
    }

    ANTMessage& operator=(ANTMessage other) {
        swap(*this, other);
        return *this;
    }

    friend void swap(ANTMessage& a, ANTMessage& b) {
        std::swap(a.antType, b.antType);
        std::swap(a.antChannel, b.antChannel);
        std::swap(a.antData, b.antData);
        std::swap(a.antDataLen, b.antDataLen);
        std::swap(a.antDeviceID, b.antDeviceID);
        std::swap(a.ts, b.ts);
    }

    ~ANTMessage(void);

    void         encode(uint8_t *msg, int *len);
    int          decode(uint8_t *data, int data_len);
    uint8_t      getType(void)               { return antType;}
    uint8_t      getChannel(void)            { return antChannel;}
    uint8_t      getData(int n)              { return antData[n];}
    int          getDataLen(void)            { return antDataLen;}
    void         setTimestamp(void)          { ts = ant_clock::now(); }
    ANTDeviceID  getDeviceID(void)           { return antDeviceID; }
    time_point<ant_clock> getTimestamp(void)     { return ts; }
    std::shared_ptr<uint8_t[]> getData(void) { return antData;}

 private:
    uint8_t           antType;
    uint8_t           antChannel;
    int               antDataLen;
    ANTDeviceID       antDeviceID;
    time_point<ant_clock> ts;
    std::shared_ptr<uint8_t[]> antData;
};

/**
 * @brief
 *
 */
template <class T> class ANTDeviceData {
 public:
    ANTDeviceData(void) {
        value = std::make_shared<std::vector<T>>();
        ts = std::make_shared<std::vector<time_point<ant_clock>>>();
    }
    void addDatum(T v, time_point<ant_clock> t) {
        value->push_back(v);
        ts->push_back(t);
    }
    std::vector<T>& getValue(void) {
        return *value;
    }
    std::vector<time_point<ant_clock>>&
        getTimestamp(void) {
        return *ts;
    }
 private:
    std::shared_ptr<std::vector<T>> value;
    std::shared_ptr<std::vector<time_point<ant_clock>>> ts;
};

typedef std::map<std::string, float> ANTMetaData;
typedef std::map<std::string, ANTDeviceData<float>> ANTTsData;

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

    ANTTsData getTsData(void) {
        return *tsData;
    }
    ANTMetaData getMetaData(void) {
        return *metaData;
    }

 private:
    std::shared_ptr<ANTTsData>        tsData;
    std::shared_ptr<ANTMetaData>      metaData;
    bool            storeTsData;
    ANTDeviceID     devID;
    pthread_mutex_t thread_lock;

 protected:
    virtual void processMessage(ANTMessage *message);

    void addDatum(std::string name, float val, time_point<ant_clock> t);
    void addDatum(const char *name, float val, time_point<ant_clock> t);
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


/**
 * @brief
 *
 */
class ANTInterface {
 public:
    int reset(void);
    int setNetworkKey(uint8_t net);
    int assignChannel(uint8_t chanNum, uint8_t chanType,
            uint8_t net, uint8_t ext = 0x00);
    int setChannelID(uint8_t chan, uint16_t device,
            uint8_t type, bool master);
    int setSearchTimeout(uint8_t chan, uint8_t timeout);
    int setChannelPeriod(uint8_t chan, uint16_t period);
    int setChannelFreq(uint8_t chan, uint8_t frequency);
    int openChannel(uint8_t chan, bool extMessages);
    int requestMessage(uint8_t chan, uint8_t message);
    int requestDataPage(uint8_t chan, uint8_t page);
    int setLibConfig(uint8_t chan, uint8_t config);
    virtual int open(void) = 0;
    virtual int close(void) = 0;
    virtual int sendMessage(ANTMessage *message) = 0;
    virtual int readMessage(std::vector<ANTMessage> *message) = 0;
};

/**
 * @brief
 *
 */
class ANTUSBInterface : public ANTInterface {
 public:
    enum rtn {
        NOERROR = 0,
        ERROR = -1
    };
    ANTUSBInterface(void);
    ~ANTUSBInterface(void);
    int open(void);
    int close(void);
    int sendMessage(ANTMessage *message);
    int readMessage(std::vector<ANTMessage> *message);
 private:
    int bulkRead(uint8_t *bytes, int size, int timeout);
    int bulkWrite(uint8_t *bytes, int size, int timeout);
    libusb_context *usb_ctx;
    libusb_device_handle *usb_handle;
    libusb_config_descriptor *usb_config;
    int readEndpoint;
    int writeEndpoint;
    int readTimeout;
    int writeTimeout;
};

/**
 * @brief
 *
 */
struct ANTDeviceParams {
    int      type;
    uint8_t  deviceType;
    uint16_t devicePeriod;
    uint8_t  deviceFrequency;
};

/**
 * @brief
 *
 */
class ANTChannel {
 public:
    enum ERROR {
        // The return values
        NOERROR     = 0,
        ERROR       = -1,
        ERROR_STATE = 1
    };
    enum TYPE {
        // The channel types
        TYPE_NONE = 0,
        TYPE_HR   = 1,
        TYPE_PWR  = 2,
        TYPE_FEC  = 3,
        TYPE_PAIR = 4
    };
    enum STATE {
        // The state machine to define
        // the various states for an ANT+ channel
        STATE_IDLE           = 0,
        STATE_ASSIGNED       = 1,
        STATE_ID_SET         = 2,
        STATE_SET_TIMEOUT    = 3,
        STATE_SET_PERIOD     = 4,
        STATE_SET_FREQ       = 5,
        STATE_SET_LIB_CONFIG = 6,
        STATE_OPEN_UNPAIRED  = 7,
        STATE_OPEN_PAIRED    = 8,
        STATE_CLOSED         = 9
    };

    ANTChannel(int type, int num, std::shared_ptr<ANTInterface> interface);
    ~ANTChannel(void);

    int        getChannelNum(void)          { return channelNum; }
    void       setType(int t);
    uint8_t    getNetwork(void)             { return network; }
    void       setExtended(uint8_t ext)     { extended = ext; }
    uint8_t    getExtended(void)            { return extended; }
    uint8_t    getChannelType(void)         { return channelType; }
    void       setChannelType(uint8_t type) { channelType = type; }
    uint16_t   getSearchTimeout(void)       { return searchTimeout; }
    int        getType(void)                { return type; }
    int        getState(void)               { return currentState; }
    // void       setDeviceId(uint16_t id)     { deviceId = id; }
    // uint16_t   getDeviceId(void)            { return deviceId; }

    int start(int type, uint16_t id = 0x0000, bool wait = true);
    ANTDeviceParams getDeviceParams(void)   { return deviceParams; }

    int  processEvent(ANTMessage *m);
    void parseMessage(ANTMessage *message);
    int  processId(ANTMessage *m);

    std::shared_ptr<ANTDevice> addDevice(ANTDeviceID *id);
    std::vector<std::shared_ptr<ANTDevice>> getDeviceList(void) {
        return deviceList;
    }

 private:
    int startThread(void);
    int stopThread(void);
    int  changeStateTo(int state);

    int      channelStartTimeout;
    uint8_t  network;
    int      currentState;
    uint8_t  channelType;
    uint8_t  channelTypeExtended;
    int      channelNum;
    uint8_t  searchTimeout;
    uint8_t  extended;
    int      type;
    uint16_t deviceId;
    std::shared_ptr<ANTInterface> iface;
    ANTDeviceParams deviceParams;
    std::vector<std::shared_ptr<ANTDevice>> deviceList;

    bool     threadRun;
    pthread_t       threadId;
    pthread_mutex_t message_lock;
    pthread_cond_t  message_cond;
    static void* callThread(void *ctx) {
        return ((ANTChannel*)ctx)->thread();
    }
    void *thread(void);

    std::queue<ANTMessage> messageQueue;
};

/**
 * @brief
 *
 */
class ANT {
 public:
    enum RETURN {
        NOERROR = 0,
        ERROR = -1
    };

    explicit ANT(std::shared_ptr<ANTInterface> iface, int nChannels = 8);
    ~ANT(void);

    int init(void);

    std::shared_ptr<ANTChannel> getChannel(uint8_t chan);
    int  getPollTime(void) {
        return pollTime;
    }
    void setPollTime(int t) {
        pollTime = t;
    }
    time_point<ant_clock> getStartTime(void) {
        return startTime;
    }

 private:
    bool extMessages;
    time_point<ant_clock> startTime;

    std::shared_ptr<ANTInterface> iface;
    std::vector<std::shared_ptr<ANTChannel>> antChannel;
    std::queue<ANTMessage> messageQueue;

    pthread_t listenerId;
    pthread_t pollerId;
    pthread_t processorId;
    pthread_mutex_t message_lock;
    pthread_cond_t message_cond;
    bool threadRun;
    int pollTime;

    int startThreads(void);
    int stopThreads(void);
    void* listenerThread(void);
    void* pollerThread(void);
    void* processorThread(void);
    static void* callListenerThread(void *ctx) {
        return ((ANT*)ctx)->listenerThread();
    }
    static void* callPollerThread(void *ctx) {
        return ((ANT*)ctx)->pollerThread();
    }
    static void* callProcessorThread(void *ctx) {
        return ((ANT*)ctx)->processorThread();
    }
};

#endif  // ANTPLUS_LIB_ANTPLUS_H_
