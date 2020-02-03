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

#include <unistd.h>

#include "antinterface.h"
#include "antmessage.h"
#include "debug.h"

#include "ant_network_key.h"

int ANTInterface::reset(void) {
    DEBUG_COMMENT("Sending ANT_SYSTEM_RESET\n");
    ANTMessage resetMessage(ANT_SYSTEM_RESET, 0);
    sendMessage(&resetMessage);

    usleep(RESET_DURATION);

    ANTMessage reply;

    return 0;
}

int ANTInterface::setNetworkKey(uint8_t net) {
    uint8_t key[] = ANT_NETWORK_KEY;

    DEBUG_COMMENT("Sending ANT_SET_NETWORK\n");
    ANTMessage netkey(ANT_SET_NETWORK, net, key, ANT_NETWORK_KEY_LEN);
    return sendMessage(&netkey);
}

int ANTInterface::assignChannel(uint8_t chanNum, uint8_t chanType,
        uint8_t net, uint8_t ext) {
    DEBUG_COMMENT("Sending ANT_UNASSIGN_CHANNEL\n");
    ANTMessage unassign(ANT_UNASSIGN_CHANNEL, chanNum);
    sendMessage(&unassign);

    DEBUG_COMMENT("Sending ANT_ASSIGN_CHANNEL\n");
    ANTMessage assign(ANT_ASSIGN_CHANNEL, chanNum, chanType, net, ext);
    return sendMessage(&assign);
}

int ANTInterface::setChannelID(uint8_t chan, uint16_t device,
        uint8_t type, bool master) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_ID\n");
    ANTMessage setid(ANT_CHANNEL_ID, chan,
            (uint8_t)(device & 0xFF), (uint8_t)(device>>8),
            (uint8_t)type,
            (uint8_t)(master ? ANT_TX_TYPE_MASTER : ANT_TX_TYPE_SLAVE));

    return sendMessage(&setid);
}

int ANTInterface::setSearchTimeout(uint8_t chan, uint8_t timeout) {
    int rc;

    DEBUG_COMMENT("Sending ANT_SEARCH_TIMEOUT\n");

    ANTMessage hpTimeout(ANT_SEARCH_TIMEOUT, chan, 0);
    if (!(rc = sendMessage(&hpTimeout))) {
        return rc;
    }

    DEBUG_COMMENT("Sending ANT_LP_SEARCH_TIMEOUT\n");
    ANTMessage lpTimeout(ANT_LP_SEARCH_TIMEOUT, chan, timeout);
    return sendMessage(&lpTimeout);
}

int ANTInterface::setChannelPeriod(uint8_t chan, uint16_t period) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_PERIOD\n");
    ANTMessage chanPeriod(ANT_CHANNEL_PERIOD, chan,
            period & 0xFF, (period >> 8) & 0xFF);
    return sendMessage(&chanPeriod);
}

int ANTInterface::setChannelFreq(uint8_t chan, uint8_t frequency) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_FREQUENCY\n");
    ANTMessage chanFreq(ANT_CHANNEL_FREQUENCY, chan, frequency);
    return sendMessage(&chanFreq);
}

int ANTInterface::setLibConfig(uint8_t chan, uint8_t config) {
    DEBUG_COMMENT("Sending ANT_LIB_CONFIG\n");
    (void)chan;  // Ignore channel ....
    ANTMessage libConfig(ANT_LIB_CONFIG, 0x00, config);
    return sendMessage(&libConfig);
}

int ANTInterface::requestDataPage(uint8_t chan, uint8_t page) {
    DEBUG_COMMENT("Sending ANT_ACK_DATA for \"Request Data Page\"\n");
    uint8_t req[8] = { 0x46, 0xFF, 0xFF, 0xFF, 0xFF, 0x81, page, 0x01};
    ANTMessage request(ANT_ACK_DATA, chan, req, sizeof(req));
    return sendMessage(&request);
}

int ANTInterface::openChannel(uint8_t chan, bool extMessages) {
    if (extMessages) {
        // Set the LIB Config before opening channel
        // if we want to get extended messages
        setLibConfig(chan, 0x80);
    }

    DEBUG_COMMENT("Sending ANT_OPEN_CHANNEL\n");
    ANTMessage open(ANT_OPEN_CHANNEL, chan);
    return sendMessage(&open);
}

int ANTInterface::requestMessage(uint8_t chan, uint8_t message) {
    DEBUG_PRINT("Sending ANT_REQ_MESSAGE 0x%02X\n", message);
    ANTMessage req(ANT_REQ_MESSAGE, chan, message);
    return sendMessage(&req);
}

