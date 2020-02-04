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

#ifndef ANTPLUS_LIB_ANTINTERFACE_H_
#define ANTPLUS_LIB_ANTINTERFACE_H_

#include <stdint.h>
#include <vector>
#include "antmessage.h"

#define RESET_DURATION      500000L

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

#endif  // ANTPLUS_LIB_ANTINTERFACE_H_
