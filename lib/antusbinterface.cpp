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

#include "ant.h"
#include "antusbinterface.h"

ANTUSBInterface::ANTUSBInterface(void) {
    usb_ctx       = NULL;
    usb_handle    = NULL;
    usb_config    = NULL;
    readEndpoint  = -1;
    writeEndpoint = -1;
    writeTimeout  = 256;
    readTimeout   = 256;
}

ANTUSBInterface::~ANTUSBInterface(void) {
    close();
}

int ANTUSBInterface::open(void) {
    ssize_t listCount;
    bool found;
    libusb_device **list;
    libusb_device_descriptor desc;
    libusb_device *dev;
    libusb_device_handle *handle;

    DEBUG_COMMENT("initializing USB\n");
    int rc = libusb_init(&usb_ctx);
    if (rc) {
        DEBUG_PRINT("Error initializing libusb. rc = %d\n", rc);
        return ERROR;
    }

    libusb_set_option(usb_ctx, LIBUSB_OPTION_LOG_LEVEL,
            LIBUSB_LOG_LEVEL_NONE);

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
        DEBUG_COMMENT("Failed to find USB Device (RESET)\n");
        return ERROR;
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
                    libusb_free_device_list(list, 1);
                    return ERROR;
                } else {
                    found = true;
                    break;
                }
            }
        }
    }

    if (!found) {
        // We never found the device
        DEBUG_COMMENT("Failed to find USB Device (OPEN)\n");
        return ERROR;
    }

    DEBUG_PRINT("bNumConfigurations = %d\n",
            desc.bNumConfigurations);
    if (!desc.bNumConfigurations) {
        DEBUG_COMMENT("No valid configurations\n");
        return ERROR;
    }

    libusb_config_descriptor *config;
    if (libusb_get_config_descriptor(dev, 0, &config)) {
        DEBUG_COMMENT("Unable to get usb config\n");
        libusb_free_config_descriptor(config);
        return ERROR;
    }

    DEBUG_PRINT("Number of Interfaces : %d\n",
            config->bNumInterfaces);

    if (config->bNumInterfaces != 1) {
        DEBUG_COMMENT("Invalid number of interfaces.\n");
        libusb_free_config_descriptor(config);
        return ERROR;
    }

    if (config->interface[0].num_altsetting != 1) {
        DEBUG_COMMENT("Invalid number of alt settings.\n");
        libusb_free_config_descriptor(config);
        return ERROR;
    }

    DEBUG_PRINT("bNumEndpoints = %d\n",
            config->interface[0].altsetting[0].bNumEndpoints);
    if (config->interface[0].altsetting[0].bNumEndpoints != 2) {
        DEBUG_COMMENT("Invalid Number of endpoints.\n");
        libusb_free_config_descriptor(config);
        return ERROR;
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
        return ERROR;
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

int ANTUSBInterface::close(void) {
    if (usb_config != NULL) {
        libusb_free_config_descriptor(usb_config);
    }

    if (usb_handle != NULL) {
        libusb_close(usb_handle);
    }

    if (usb_ctx != NULL) {
        libusb_exit(usb_ctx);
    }

    return NOERROR;
}

int ANTUSBInterface::bulkRead(uint8_t *bytes, int size, int timeout) {
    int actualSize;
    int rc = libusb_bulk_transfer(usb_handle, readEndpoint,
            bytes, size, &actualSize, timeout);

#ifdef DEBUG_OUTPUT
    if (actualSize > 0) {
        char sb[20000];
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

int ANTUSBInterface::bulkWrite(uint8_t *bytes, int size, int timeout) {
    int actualSize;
    int rc = libusb_bulk_transfer(usb_handle, writeEndpoint,
            bytes, size, &actualSize, timeout);

    if (rc < 0) {
        DEBUG_PRINT("libusb_bulk_transfer failed with rc=%d\n", rc);
        return rc;
    }

#ifdef DEBUG_OUTPUT
    char sb[20000];
    bytestream_to_string(sb, sizeof(sb), bytes, actualSize);
    DEBUG_PRINT("Wrote %d : %s\n", actualSize, sb);
#endif

    return actualSize;
}

int ANTUSBInterface::sendMessage(ANTMessage *message) {
    int msg_len;
    uint8_t msg[MAX_MESSAGE_SIZE];

    message->encode(msg, &msg_len);

    return bulkWrite(msg, msg_len, writeTimeout);
}

int ANTUSBInterface::readMessage(std::vector<ANTMessage> *message) {
    uint8_t bytes[MAX_MESSAGE_SIZE];
    int nbytes = bulkRead(bytes, MAX_MESSAGE_SIZE, readTimeout);

    if (nbytes > 0) {
        DEBUG_PRINT("Recieved %d bytes.\n", nbytes);
        // Now we walk the data looking for sync bytes
        int i = 0;
        while (i < nbytes) {
            // Search for sync
            if (bytes[i] == ANT_SYNC_BYTE) {
                // We have a message, second byte is length
                uint8_t len = bytes[i+1] + 4;
                message->push_back(ANTMessage(&bytes[i], len));
                message->back().setTimestamp();
                i += len;
            }
        }
    }

    return nbytes;
}

