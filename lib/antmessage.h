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
//

#ifndef ANTPLUS_LIB_ANTMESSAGE_H_
#define ANTPLUS_LIB_ANTMESSAGE_H_

#include <stdint.h>
#include <chrono>
#include <utility>
#include <memory>

#include "antdefs.h"

using Clock = std::chrono::steady_clock;
using std::chrono::time_point;

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
        for (int i=0; i < MAX_MESSAGE_SIZE; i++) {
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
    void         setTimestamp(void)          { ts = Clock::now(); }
    ANTDeviceID  getDeviceID(void)           { return antDeviceID; }
    time_point<Clock> getTimestamp(void)     { return ts; }
    std::shared_ptr<uint8_t[]> getData(void) { return antData;}

 private:
    uint8_t           antType;
    uint8_t           antChannel;
    int               antDataLen;
    ANTDeviceID       antDeviceID;
    time_point<Clock> ts;
    std::shared_ptr<uint8_t[]> antData;
};

#endif  // ANTPLUS_LIB_ANTMESSAGE_H_
