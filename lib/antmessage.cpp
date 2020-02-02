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

#include "antmessage.h"
#include "debug.h"

ANTMessage::ANTMessage(void) {
    antType = 0x00;
    antChannel = 0x00;
    antDataLen = 0;

    antData = std::make_unique<uint8_t[]> (MAX_MESSAGE_SIZE);
    for (int i=0; i < MAX_MESSAGE_SIZE; i++) {
        antData[i] = 0x00;
    }
}

ANTMessage::~ANTMessage(void) {
}

ANTMessage::ANTMessage(uint8_t *data, int data_len)
    : ANTMessage() {
    decode(data, data_len);
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan, uint8_t *data, int len)
    : ANTMessage() {
    // Copy to internal structure
    antType = type;
    antChannel = chan;
    antDataLen = len;

    for (int i=0; i < len; i++) {
        antData[i] = data[i];
    }
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan)
    : ANTMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 0;
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan, uint8_t b0)
    : ANTMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 1;
    antData[0] = b0;
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1)
    : ANTMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 2;
    antData[0] = b0;
    antData[1] = b1;
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1, uint8_t b2)
    : ANTMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 3;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1, uint8_t b2, uint8_t b3)
    : ANTMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 4;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
    antData[3] = b3;
}

ANTMessage::ANTMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
    : ANTMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 5;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
    antData[3] = b3;
    antData[4] = b4;
}

int ANTMessage::decode(uint8_t *data, int data_len) {
    if (data_len < 5) {
        DEBUG_COMMENT("Data too short (< 5)\n");
        return ERROR_LEN;
    }

    if (data[0] != ANT_SYNC_BYTE) {
        DEBUG_COMMENT("First byte does not match SYNC\n");
        return ERROR_PROTO;
    }

    if (data[1] != (data_len - 4)) {
        DEBUG_COMMENT("Data length does not match length in payload.\n");
        return ERROR_LEN;
    }

    // Check CRC

    uint8_t crc = 0;
    for (int i=0; i < (data_len-1); i++) {
        crc ^= data[i];
    }
    if (data[3+data[1]] != crc) {
        DEBUG_COMMENT("CRC MISMACH\n");
        return ERROR_CRC;
    }

    // Now create the message structure
    // We have a copy but its probably safer ....

    antDataLen = data[1] - 1;
    antType = data[2];
    antChannel = data[3];

    for (int i=0; i < antDataLen; i++) {
        antData[i] = data[4+i];
    }

    DEBUG_PRINT("antDataLen = %d antType = 0x%02X antChannel = %d\n",
            antDataLen, antType, antChannel);

    if (antDataLen > 8) {
        // We have an extended format
        uint8_t ext = antData[8];
        if (ext & ANT_EXT_MSG_CHAN_ID) {
            uint16_t deviceID;
            deviceID  = antData[9];
            deviceID |= (antData[10] << 8);
            uint8_t deviceType = antData[11];
            uint8_t transType = antData[12];

            antDeviceID = ANTDeviceID(deviceID, deviceType);

            DEBUG_PRINT("Device ID = 0x%04X type = 0x%02X transType = 0x%02X\n",
                    deviceID, deviceType, transType);
        }
    }

    return NOERROR;
}

void ANTMessage::encode(uint8_t *msg, int *len) {
    msg[0] = ANT_SYNC_BYTE;
    msg[1] = antDataLen + 1;
    msg[2] = antType;
    msg[3] = antChannel;

    for (int i=0; i < antDataLen; i++) {
        msg[i+4] = antData[i];
    }

    uint8_t crc = 0;
    for (int i=0; i < (antDataLen+4); i++) {
        crc ^= msg[i];
    }

    msg[antDataLen+4] = crc;

    *len = antDataLen + 5;

#ifdef DEBUG_OUTPUT
    char bytes[20000];
    bytestream_to_string(bytes, sizeof(bytes), msg, antDataLen + 5);
    DEBUG_PRINT("ANT Message : %s\n",  bytes);
#endif
}

