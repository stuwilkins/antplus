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

#include <string.h>
#include <cstdio>

#include "ant.h"
#include "antdevice.h"
#include "debug.h"

void bytestream_to_string(char *out, int n_out, uint8_t *bytes, int n_bytes) {
    if (n_bytes == 0) {
        out[0] = 0;
        return;
    }

    char *_dbmsg = out;

    for (int i=0; i < n_bytes; i++) {
        snprintf(_dbmsg, n_out - (3 * i), "%02X:", bytes[i]);
        _dbmsg += 3;
    }
    *(_dbmsg-1) = 0;
}

AntMessage::AntMessage(void) {
    antType = 0x00;
    antChannel = 0x00;
    for (int i=0; i < ANT_MAX_DATA_SIZE; i++) {
        antData[i] = 0x00;
    }
    antDataLen = 0;
}

AntMessage::AntMessage(uint8_t *data, int data_len)
    : AntMessage() {
    // Decode
    this->decode(data, data_len);
}

AntMessage::AntMessage(uint8_t type, uint8_t chan, uint8_t *data, int len)
    : AntMessage() {
    // Copy to internal structure
    antType = type;
    antChannel = chan;
    antDataLen = len;

    for (int i=0; i < len; i++) {
        antData[i] = data[i];
    }
}

AntMessage::AntMessage(uint8_t type, uint8_t chan)
    : AntMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 0;
}

AntMessage::AntMessage(uint8_t type, uint8_t chan, uint8_t b0)
    : AntMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 1;
    antData[0] = b0;
}

AntMessage::AntMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1)
    : AntMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 2;
    antData[0] = b0;
    antData[1] = b1;
}

AntMessage::AntMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1, uint8_t b2)
    : AntMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 3;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
}

AntMessage::AntMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1, uint8_t b2, uint8_t b3)
    : AntMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 4;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
    antData[3] = b3;
}

AntMessage::AntMessage(uint8_t type, uint8_t chan, uint8_t b0,
        uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
    : AntMessage() {
    antType = type;
    antChannel = chan;
    antDataLen = 5;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
    antData[3] = b3;
    antData[4] = b4;
}

int AntMessage::decode(uint8_t *data, int data_len) {
    if (data_len < 5) {
        DEBUG_COMMENT("Data too short (< 5)\n");
        return ANT_DECODE_LEN_ERROR;
    }

    if (data[0] != ANT_SYNC_BYTE) {
        DEBUG_COMMENT("First byte does not match SYNC\n");
        return ANT_DECODE_PROTO_ERROR;
    }

    if (data[1] != (data_len - 4)) {
        DEBUG_COMMENT("Data length does not match length in payload.");
        return ANT_DECODE_LEN_ERROR;
    }

    // Check CRC

    uint8_t crc = 0;
    for (int i=0; i < (data_len-1); i++) {
        crc ^= data[i];
    }
    if (data[3+data[1]] != crc) {
        DEBUG_COMMENT("CRC MISMACH\n");
        return ANT_DECODE_CRC_ERROR;
    }

    // Now create the message structure
    // We have a copy but its probably safer ....

    antDataLen = data[1] - 1;
    antType = data[2];
    antChannel = data[3];

    for (int i=0; i < antDataLen; i++) {
        antData[i] = data[4+i];
    }

    return ANT_DECODE_OK;
}

void AntMessage::encode(uint8_t *msg, int *len) {
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
    char bytes[100];
    bytestream_to_string(bytes, sizeof(bytes), msg, *len);
    DEBUG_PRINT("ANT Message : %s\n",  bytes);
#endif
}

AntUsb::AntUsb(void) {
    usb_ctx = NULL;
    usb_handle = NULL;
    usb_config = NULL;
    readEndpoint = -1;
    writeEndpoint = -1;
    writeTimeout = 500;
    readTimeout = 256;
}

AntUsb::~AntUsb(void) {
    libusb_free_config_descriptor(usb_config);
}

int AntUsb::init(void) {
    DEBUG_COMMENT("initializing USB\n");
    int rc = libusb_init(&usb_ctx);
    if (rc) {
        DEBUG_PRINT("Error initializing libusb. rc = %d\n", rc);
        return ERROR;
    }
    libusb_set_option(usb_ctx, LIBUSB_OPTION_LOG_LEVEL,
            LIBUSB_LOG_LEVEL_NONE);

    return NOERROR;
}

int AntUsb::setup(void) {
    ssize_t listCount;
    bool found;
    libusb_device **list;
    libusb_device_descriptor desc;
    libusb_device *dev;
    libusb_device_handle *handle;

    // First go through list and reset the device
    listCount = libusb_get_device_list(usb_ctx, &list);
    found = false;
    for (int i = 0; i < listCount; i++) {
        dev = list[i];
        int rc = libusb_get_device_descriptor(dev, &desc);
        if (!rc) {
            // We got a valid descriptor.
            if (desc.idVendor == GARMIN_USB2_VID &&
                    (desc.idProduct == GARMIN_USB2_PID
                     || desc.idProduct == GARMIN_OEM_PID)) {
                if (!libusb_open(dev, &handle)) {
                    DEBUG_PRINT("Found Device ... 0x%04X 0x%04X\n",
                            desc.idVendor, desc.idProduct);
                    libusb_reset_device(handle);
                    libusb_close(handle);
                    found = true;
                    break;
                }
            }
        }
    }

    libusb_free_device_list(list, 1);

    if (!found) {
        // We never found the device
        // TODO(swilkins) : Change returns
        DEBUG_COMMENT("Failed to find USB Device (RESET)");
        return 0;
    }

    // Now lets search again and open the device

    listCount = libusb_get_device_list(usb_ctx, &list);
    found = false;
    for (int i = 0; i < listCount; i++) {
        dev = list[i];
        if (!libusb_get_device_descriptor(dev, &desc)) {
            // We got a valid descriptor.
            if (desc.idVendor == GARMIN_USB2_VID &&
                    (desc.idProduct == GARMIN_USB2_PID
                     || desc.idProduct == GARMIN_OEM_PID)) {
                DEBUG_PRINT("Found Device ... 0x%04X 0x%04X\n",
                        desc.idVendor, desc.idProduct);
                if (libusb_open(dev, &handle)) {
                    DEBUG_PRINT("Failed to open device 0x%04X 0x%04X\n",
                            desc.idVendor, desc.idProduct);
                    // TODO(swilkins) : Returns
                    libusb_free_device_list(list, 1);
                    return 0;
                } else {
                    found = true;
                    break;
                }
            }
        }
    }

    if (!found) {
        // We never found the device
        // TODO(swilkins) : Change returns
        DEBUG_COMMENT("Failed to find USB Device (OPEN)");
        return 0;
    }

    DEBUG_PRINT("bNumConfigurations = %d\n",
            desc.bNumConfigurations);
    if (!desc.bNumConfigurations) {
        DEBUG_COMMENT("No valid configurations\n");
        // TODO(swilkins) : Returns
        return 0;
    }

    libusb_config_descriptor *config;
    if (libusb_get_config_descriptor(dev, 0, &config)) {
        DEBUG_COMMENT("Unable to get usb config\n");
        libusb_free_config_descriptor(config);
        return 0;
    }

    DEBUG_PRINT("Number of Interfaces : %d\n",
            config->bNumInterfaces);

    if (config->bNumInterfaces != 1) {
        DEBUG_COMMENT("Invalid number of interfaces.\n");
        libusb_free_config_descriptor(config);
        return 0;
    }

    if (config->interface[0].num_altsetting != 1) {
        DEBUG_COMMENT("Invalid number of alt settings.\n");
        libusb_free_config_descriptor(config);
        return 0;
    }

    DEBUG_PRINT("bNumEndpoints = %d\n",
            config->interface[0].altsetting[0].bNumEndpoints);
    if (config->interface[0].altsetting[0].bNumEndpoints != 2) {
        DEBUG_COMMENT("Invalid Number of endpoints.\n");
        libusb_free_config_descriptor(config);
        return 0;
    }

    for (int i=0; i < 2; i++) {
        int ep = config->interface[0].altsetting[0]
            .endpoint[i].bEndpointAddress;
        if (ep & LIBUSB_ENDPOINT_DIR_MASK) {
            DEBUG_PRINT("Read Endpoint = 0x%02X (%d)\n", ep, i);
            readEndpoint = ep;
        } else {
            DEBUG_PRINT("Write Endpoint = 0x%02X (%d)\n", ep, i);
            writeEndpoint = ep;
        }
    }

    // Now we need to close the kernel driver
    // and claim the interface for ourselves

    libusb_detach_kernel_driver(handle,
            config->interface[0].altsetting[0].bInterfaceNumber);
    if (libusb_claim_interface(handle,
                config->interface[0].altsetting[0].bInterfaceNumber)) {
        DEBUG_COMMENT("Unable to claim interface.\n");
        // TODO(swilkins) : Change returns
        return 0;
    }

    usb_config = config;
    usb_handle = handle;

    libusb_free_device_list(list, 1);

    if ((readEndpoint < 0) || (writeEndpoint < 0)) {
        DEBUG_COMMENT("Did not find valid device.\n");
        return ERROR;
    }

    return NOERROR;
}

int AntUsb::bulkRead(uint8_t *bytes, int size, int timeout) {
    int actualSize;
    int rc = libusb_bulk_transfer(usb_handle, readEndpoint,
            bytes, size, &actualSize, timeout);

#ifdef DEBUG_OUTPUT
    if (actualSize > 0) {
        char sb[100];
        bytestream_to_string(sb, sizeof(sb), bytes, actualSize);
        DEBUG_PRINT("Recieved %d : %s\n", actualSize, sb);
    }
#endif


    if (rc < 0 && rc != LIBUSB_ERROR_TIMEOUT) {
        DEBUG_PRINT("libusb_bulk_transfer failed with rc=%d\n", rc);
        return rc;
    }

    return actualSize;
}

int AntUsb::bulkWrite(uint8_t *bytes, int size, int timeout) {
    int actualSize;
    int rc = libusb_bulk_transfer(usb_handle, writeEndpoint,
            bytes, size, &actualSize, timeout);

    if (rc < 0) {
        DEBUG_PRINT("libusb_bulk_transfer failed with rc=%d\n", rc);
        return rc;
    }

#ifdef DEBUG_OUTPUT
    char sb[100];
    bytestream_to_string(sb, sizeof(sb), bytes, actualSize);
    DEBUG_PRINT("Wrote %d : %s\n", actualSize, sb);
#endif

    return actualSize;
}

int AntUsb::sendMessage(AntMessage *message, bool readReply) {
    int msg_len;
    uint8_t msg[ANT_MAX_MESSAGE_SIZE];

    message->encode(msg, &msg_len);

    int rc = bulkWrite(msg, msg_len, writeTimeout);

    if (readReply) {
        AntMessage reply;
        rc = readMessage(&reply);
        if (rc) {
            // reply.parse();
        }
    }

    return rc;
}

int AntUsb::readMessage(AntMessage *message) {
    uint8_t bytes[ANT_MAX_MESSAGE_SIZE];
    int nbytes = bulkRead(bytes, ANT_MAX_MESSAGE_SIZE, readTimeout);
    if (nbytes > 0) {
        DEBUG_PRINT("nbytes = %d\n", nbytes);
        message->decode(bytes, nbytes);
    }

    return nbytes;
}

int AntUsb::reset(void) {
    DEBUG_COMMENT("Sending ANT_SYSTEM_RESET\n");
    AntMessage resetMessage(ANT_SYSTEM_RESET, 0);
    sendMessage(&resetMessage);

    usleep(500000L);

    AntMessage reply;
    readMessage(&reply);
    // reply.parse();

    return 0;
}

int AntUsb::setNetworkKey(void) {
    uint8_t key[] = ANT_NETWORK_KEY;

    DEBUG_COMMENT("Sending ANT_SET_NETWORK\n");
    AntMessage netkey(ANT_SET_NETWORK, 0, key, ANT_NETWORK_KEY_LEN);
    return sendMessage(&netkey);
}

int AntUsb::assignChannel(int chanNum, bool master) {
    int rc;

    DEBUG_COMMENT("Sending ANT_UNASSIGN_CHANNEL\n");
    AntMessage unassign(ANT_UNASSIGN_CHANNEL, chanNum);
    sendMessage(&unassign);

    DEBUG_COMMENT("Sending ANT_ASSIGN_CHANNEL\n");
    AntMessage assign(ANT_ASSIGN_CHANNEL, (unsigned int)chanNum,
            master ? CHANNEL_TYPE_TX : CHANNEL_TYPE_RX,
            1);  // receive channel on network 1
    return sendMessage(&assign);
}

int AntUsb::setChannelID(uint8_t chan, uint16_t device,
        uint8_t type, bool master) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_ID\n");
    AntMessage setid(ANT_CHANNEL_ID, chan,
            (uint8_t)(device & 0xFF), (uint8_t)(device>>8),
            type, master ? ANT_TX_TYPE_MASTER : ANT_TX_TYPE_SLAVE);

    return sendMessage(&setid);
}

int AntUsb::setSearchTimeout(uint8_t chan, uint8_t timeout) {
    int rc;

    DEBUG_COMMENT("Sending ANT_SEARCH_TIMEOUT\n");

    AntMessage hpTimeout(ANT_SEARCH_TIMEOUT, chan, 0);
    if (!(rc = sendMessage(&hpTimeout))) {
        return rc;
    }

    DEBUG_COMMENT("Sending ANT_LP_SEARCH_TIMEOUT\n");
    AntMessage lpTimeout(ANT_LP_SEARCH_TIMEOUT, chan, timeout);
    return sendMessage(&lpTimeout);
}

int AntUsb::setChannelPeriod(uint8_t chan, uint16_t period) {
    AntMessage chanPeriod(ANT_CHANNEL_PERIOD, chan,
            period & 0xFF, (period >> 8) & 0xFF);
    return sendMessage(&chanPeriod);
}

int AntUsb::setChannelFreq(uint8_t chan, uint8_t frequency) {
    AntMessage chanFreq(ANT_CHANNEL_FREQUENCY, chan, frequency);
    return sendMessage(&chanFreq);
}

int AntUsb::openChannel(uint8_t chan) {
    AntMessage open(ANT_OPEN_CHANNEL, chan);
    return sendMessage(&open);
}

int AntUsb::requestMessage(uint8_t chan, uint8_t message) {
    AntMessage req(ANT_REQ_MESSAGE, chan, message);
    return sendMessage(&req);
}

int AntUsb::startListener(void) {
    DEBUG_COMMENT("Starting listener thread ...\n");
    pthread_create(&listenerId, NULL, antusb_listener, (void *)this);

    return NOERROR;
}

static void* antusb_listener(void *ctx) {
    AntUsb *antusb = (AntUsb*) ctx;
    uint16_t counter = 0;
    AntDeviceFEC fec;
    AntDevicePower power;
    AntDeviceHeartrate hr;

    DEBUG_COMMENT("Started Listener Loop ....\n");

    // Recieve Loop

    AntMessage r;
    for (;;) {
        if (antusb->readMessage(&r) > 0) {
            switch (r.getType()) {
                case ANT_NOTIF_STARTUP:
                    DEBUG_COMMENT("RESET OK\n");
                    break;
                // case ANT_ACK_DATA:
                // case ANT_CHANNEL_STATUS:
                // case ANT_CHANNEL_ID:
                // case ANT_BURST_DATA:
                //     parseChannelEvent();
                //     break;
                case ANT_CHANNEL_EVENT:
                    switch (r.getData()[ANT_OFFSET_MESSAGE_CODE]) {
                        case EVENT_TRANSFER_TX_FAILED:
                            DEBUG_COMMENT("Transfer FAILED\n");
                            break;
                    }
                    break;
                case ANT_BROADCAST_DATA:
                    DEBUG_COMMENT("ANT_BROADCAST_DATA\n");
                    if (r.getData()[0] == 1) {
                        fec.parseMessage(&r);
                    }
                    if (r.getData()[0] == 2) {
                        power.parseMessage(&r);
                    }
                    if (r.getData()[0] == 3) {
                        hr.parseMessage(&r);
                    }
                    break;
                default:
                    DEBUG_COMMENT("Unknown ANT Type\n");
                    break;
            }
        }
        if (!(counter % 100)) {
            antusb->requestMessage(2, ANT_CHANNEL_ID);
        }
        if (!(counter % 57)) {
            antusb->requestMessage(1, ANT_CHANNEL_ID);
        }
        counter++;
    }

    return NULL;
}

// int AntMessage::parseChannelEvent(void) {
//     int rtn = -1;
//     int channel = antData[ANT_OFFSET_DATA];
//     int code = antData[ANT_OFFSET_MESSAGE_CODE];
//
//     DEBUG_PRINT("Channel Event on channel 0x%02X with code 0x%02X\n",
//             channel, code);
//
//     switch (code) {
//         case RESPONSE_NO_ERROR:
//             rtn = 0;
//             DEBUG_COMMENT("RESPONSE_NO_ERROR\n");
//             break;
//         case ANT_CHANNEL_EVENT:
//             rtn = 0;
//             DEBUG_COMMENT("ANT_CHANNEL_EVENT\n");
//             break;
//         case ANT_ACK_DATA:
//             rtn = 0;
//             DEBUG_COMMENT("ANT_ACK_DATA\n");
//             break;
//         default:
//             DEBUG_COMMENT("Unknown Channel Event Code\n");
//             break;
//     }
//
//     return rtn;
// }
//
