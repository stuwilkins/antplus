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
#include <chrono>

#include "ant.h"
#include "antdevice.h"
#include "antchannel.h"
#include "debug.h"

AntUsb::AntUsb(void) {
    usb_ctx = NULL;
    usb_handle = NULL;
    usb_config = NULL;
    readEndpoint = -1;
    writeEndpoint = -1;
    writeTimeout = 256;
    readTimeout = 256;
    numChannels = 8;

    DEBUG_PRINT("Creating %d channels.\n", numChannels);
    antChannel = new AntChannel[numChannels];
    for (int i=0; i < numChannels; i++) {
        antChannel[i].setChannel(i);
        antChannel[i].setType(AntDevice::TYPE_NONE);
    }

    // Set the start time
    startTime = Clock::now();
}

AntUsb::~AntUsb(void) {
    libusb_free_config_descriptor(usb_config);
    delete [] antChannel;
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
    uint8_t msg[USB_MAX_MESSAGE_SIZE];

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
    uint8_t bytes[USB_MAX_MESSAGE_SIZE];
    int nbytes = bulkRead(bytes, USB_MAX_MESSAGE_SIZE, readTimeout);
    if (nbytes > 0) {
        DEBUG_PRINT("Recieved %d bytes.\n", nbytes);
        message->setTimestamp();
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

int AntUsb::setNetworkKey(uint8_t net) {
    uint8_t key[] = ANT_NETWORK_KEY;

    DEBUG_COMMENT("Sending ANT_SET_NETWORK\n");
    AntMessage netkey(ANT_SET_NETWORK, net, key, ANT_NETWORK_KEY_LEN);
    return sendMessage(&netkey);
}

int AntUsb::assignChannel(uint8_t chanNum, bool master, uint8_t net) {
    DEBUG_COMMENT("Sending ANT_UNASSIGN_CHANNEL\n");
    AntMessage unassign(ANT_UNASSIGN_CHANNEL, chanNum);
    sendMessage(&unassign);

    DEBUG_COMMENT("Sending ANT_ASSIGN_CHANNEL\n");
    AntMessage assign(ANT_ASSIGN_CHANNEL, chanNum,
            master ? CHANNEL_TYPE_TX : CHANNEL_TYPE_RX, net);
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
    DEBUG_COMMENT("Sending ANT_CHANNEL_PERIOD\n");
    AntMessage chanPeriod(ANT_CHANNEL_PERIOD, chan,
            period & 0xFF, (period >> 8) & 0xFF);
    return sendMessage(&chanPeriod);
}

int AntUsb::setChannelFreq(uint8_t chan, uint8_t frequency) {
    DEBUG_COMMENT("Sending ANT_CHANNEL_FREQUENCY\n");
    AntMessage chanFreq(ANT_CHANNEL_FREQUENCY, chan, frequency);
    return sendMessage(&chanFreq);
}

int AntUsb::openChannel(uint8_t chan) {
    DEBUG_COMMENT("Sending ANT_OPEN_CHANNEL\n");
    AntMessage open(ANT_OPEN_CHANNEL, chan);
    return sendMessage(&open);
}

int AntUsb::requestMessage(uint8_t chan, uint8_t message) {
    DEBUG_PRINT("Sending ANT_REQ_MESSAGE 0x%02X\n", message);
    AntMessage req(ANT_REQ_MESSAGE, chan, message);
    return sendMessage(&req);
}

int AntUsb::startListener(void) {
    DEBUG_COMMENT("Starting listener thread ...\n");
    pthread_create(&listenerId, NULL, antusb_listener, (void *)this);

    return NOERROR;
}

void* antusb_listener(void *ctx) {
    AntUsb *antusb = (AntUsb*) ctx;
    uint16_t counter = 0;

    // Recieve Loop

    DEBUG_COMMENT("Started Listener Loop ....\n");
    for (;;) {
        AntMessage m;
        if (antusb->readMessage(&m) > 0) {
            switch (m.getType()) {
                case ANT_NOTIF_STARTUP:
                    DEBUG_COMMENT("RESET OK\n");
                    break;
                case ANT_CHANNEL_EVENT:
                    antusb->channelProcessEvent(&m);
                    break;
                case ANT_CHANNEL_ID:
                    antusb->channelProcessID(&m);
                    break;
                case ANT_BROADCAST_DATA:
                    antusb->channelProcessBroadcast(&m);
                    break;
                default:
                    DEBUG_COMMENT("Unknown ANT Type\n");
                    break;
            }
        }

        int r = counter % 40;
        if (r < antusb->getNumChannels()) {
            if (antusb->getChannel(r)->getState() ==
                    AntChannel::STATE_OPEN_UNPAIRED) {
                DEBUG_PRINT("Requesting Channel ID on channel %d\n", r);
                antusb->requestMessage(r, ANT_CHANNEL_ID);
            }
        }
        counter++;
    }

    return NULL;
}

int AntUsb::channelChangeStateTo(uint8_t chan, int state) {
    DEBUG_PRINT("chan = %d, state = %d\n", chan, state);
    switch (state) {
        case AntChannel::STATE_ASSIGNED:
            assignChannel(chan,
                    getChannel(chan)->getMaster(),
                    getChannel(chan)->getNetwork());
            break;
        case AntChannel::STATE_ID_SET:
            // TODO(swilkins) check why zero
            setChannelID(chan,
                    getChannel(chan)->getDeviceId(),
                    getChannel(chan)->getDeviceType(), 0);
            break;
        case AntChannel::STATE_SET_TIMEOUT:
            setSearchTimeout(chan,
                    getChannel(chan)->getSearchTimeout());
            break;
        case AntChannel::STATE_SET_PERIOD:
            setChannelPeriod(chan,
                    getChannel(chan)->getDevicePeriod());
            break;
        case AntChannel::STATE_SET_FREQ:
            setChannelFreq(chan,
                    getChannel(chan)->getDeviceFrequency());
            break;
        case AntChannel::STATE_OPEN_UNPAIRED:
            openChannel(chan);
            break;
        default:
            DEBUG_PRINT("Unknown State %d\n", state);
            break;
    }
    return NOERROR;
}

int AntUsb::channelStart(uint8_t chan, int type,
        uint16_t id, bool wait) {
    // Start a channel config

    AntChannel *antChannel = getChannel(chan);
    antChannel->setType(type);
    antChannel->setDeviceID(id);

    // Claim the channel.

    if (antChannel->getState() !=
            AntChannel::STATE_IDLE) {
        DEBUG_PRINT("Cannot start channel when not IDLE"
        "(current state = %d)\n",
                antChannel->getState());
        return ERROR;
    }

    // Ok the next level is ASSIGNED

    channelChangeStateTo(chan, AntChannel::STATE_ASSIGNED);

    // If we wait .. spinlock until the channel is open
    // TODO(swilkins) : We should add a timeout

    if (wait) {
        int state = antChannel->getState();
        while ((state != AntChannel::STATE_OPEN_UNPAIRED)
                && (state != AntChannel::STATE_OPEN_PAIRED)) {
            sleep(1);
            state = antChannel->getState();
        }
    }

    return NOERROR;
}

int AntUsb::channelProcessID(AntMessage *m) {
    // Parse the ID
    uint16_t id;
    uint8_t type;
    uint8_t chan = m->getChannel();
    id  = (m->getData(1) << 8);
    id |= m->getData(0);
    type = m->getData(2);

    DEBUG_PRINT("Processed Device ID 0x%04X type 0x%02X on channel %d\n",
            id, type, chan);

    getChannel(chan)->addDeviceId(id);

    return NOERROR;
}

int AntUsb::channelProcessEvent(AntMessage *m) {
    // 2nd Byte is event code
    uint8_t chan = m->getChannel();
    uint8_t commandCode = m->getData(0);
    uint8_t responseCode = m->getData(1);

    DEBUG_PRINT("Channel response (chan = %d"
            " type = 0x%02X code = 0x%02X response ="
            " 0x%02X)\n", m->getChannel(), m->getType(),
            commandCode, responseCode);

    switch (responseCode) {
        case RESPONSE_NO_ERROR:
            switch (commandCode) {
                case ANT_ASSIGN_CHANNEL:
                    getChannel(chan)->setState(AntChannel::STATE_ASSIGNED);
                    channelChangeStateTo(chan, AntChannel::STATE_ID_SET);
                    break;
                case ANT_CHANNEL_ID:
                    getChannel(chan)->setState(AntChannel::STATE_ID_SET);
                    channelChangeStateTo(chan, AntChannel::STATE_SET_TIMEOUT);
                    break;
                case ANT_LP_SEARCH_TIMEOUT:
                    getChannel(chan)->setState(
                            AntChannel::STATE_SET_TIMEOUT);
                    channelChangeStateTo(chan, AntChannel::STATE_SET_PERIOD);
                    break;
                case ANT_CHANNEL_PERIOD:
                    getChannel(chan)->setState(AntChannel::STATE_SET_PERIOD);
                    channelChangeStateTo(chan, AntChannel::STATE_SET_FREQ);
                    break;
                case ANT_CHANNEL_FREQUENCY:
                    getChannel(chan)->setState(AntChannel::STATE_SET_FREQ);
                    channelChangeStateTo(chan,
                            AntChannel::STATE_OPEN_UNPAIRED);
                    break;
                case ANT_OPEN_CHANNEL:
                    // Do nothing, but set state
                    getChannel(chan)->setState(
                            AntChannel::STATE_OPEN_UNPAIRED);
                    break;
            }
            break;
        case EVENT_RX_SEARCH_TIMEOUT:
            DEBUG_PRINT("Search timeout on channel %d\n", chan);
            getChannel(chan)->setState(AntChannel::STATE_OPEN_UNPAIRED);
            break;
        case EVENT_CHANNEL_CLOSED:
            getChannel(chan)->setState(AntChannel::STATE_CLOSED);
            channelChangeStateTo(chan, AntChannel::STATE_OPEN_UNPAIRED);
            break;
    }
    return NOERROR;
}

int AntUsb::channelProcessBroadcast(AntMessage *m) {
    // Parse the ID
    uint8_t chan = m->getChannel();
    switch (getChannel(chan)->getType()) {
        case AntDevice::TYPE_HR:
            getChannel(chan)->getDeviceHR()->parseMessage(m);
            break;
        case AntDevice::TYPE_PWR:
            getChannel(chan)->getDevicePWR()->parseMessage(m);
            break;
        case AntDevice::TYPE_FEC:
            getChannel(chan)->getDeviceFEC()->parseMessage(m);
            break;
    }

    return NOERROR;
}
