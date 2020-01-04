/*
 * =====================================================================================
 *
 *       Filename:  antusb.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/02/2020 07:24:17 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stuart B. Wilkins (sbw), stuart@stuwilkins.org
 *   Organization:
 *
 * =====================================================================================
 */

#include <unistd.h>

#include "debug.h"
#include "antusb.h"
#include "ant.h"

AntUsb::AntUsb(void)
{
    usb_ctx = NULL;
    usb_handle = NULL;
    usb_config = NULL;
    readEndpoint = -1;
    writeEndpoint = -1;
    writeTimeout = 256;
    readTimeout = 256;
}

AntUsb::~AntUsb(void)
{
    libusb_free_config_descriptor(usb_config);
}

int AntUsb::init(void)
{
    DEBUG_COMMENT("initializing USB\n");
    int rc = libusb_init(&usb_ctx);
    if(rc)
    {
        DEBUG_PRINT("Error initializing libusb. rc = %d\n", rc);
        return ERROR;
    }
    libusb_set_option(usb_ctx, LIBUSB_OPTION_LOG_LEVEL,
            LIBUSB_LOG_LEVEL_WARNING);

    return NOERROR;
}

int AntUsb::setup(void)
{
    DEBUG_COMMENT("Finding USB Busses\n");

    libusb_device **list;
    ssize_t listCount = libusb_get_device_list(usb_ctx, &list);
    DEBUG_PRINT("libusb_get_device_list returned rc=%ld\n", listCount);
    if(listCount < 0)
    {
        return 0;
    }

    for(int i = 0;i < listCount; i++)
    {
        libusb_device *dev = list[i];
        libusb_device_descriptor desc;
        libusb_device_handle *handle = NULL;
        unsigned char buffer[256];
        int rc;

        rc = libusb_get_device_descriptor(dev, &desc);
        DEBUG_PRINT("libusb_get_device_descriptor() returned %d\n", rc);
        if(rc < 0)
        {
            libusb_unref_device(dev);
            continue;
        }

        rc = libusb_open(dev, &handle);
        if(rc == LIBUSB_SUCCESS)
        {
            DEBUG_PRINT("USB Device %d vendor ID 0x%04x product ID 0x%04x\n",
                    i, desc.idVendor, desc.idProduct);
            if(libusb_get_string_descriptor_ascii(handle, desc.iManufacturer,
                        buffer, sizeof(buffer)))
            {
                DEBUG_PRINT("--- Manufacturer : %s\n", buffer);
            }

            if(libusb_get_string_descriptor_ascii(handle, desc.iProduct,
                        buffer, sizeof(buffer)))
            {
                DEBUG_PRINT("--- Product : %s\n", buffer);
            }

            if(libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber,
                        buffer, sizeof(buffer)))
            {
                DEBUG_PRINT("--- Serial : %s\n", buffer);
            }

            // We have a vaid device...

            if (desc.idVendor == GARMIN_USB2_VID &&
                (desc.idProduct == GARMIN_USB2_PID
                 || desc.idProduct == GARMIN_OEM_PID)) {
                DEBUG_COMMENT("Found Valid Device\n");
                this->usb_handle = handle;

                // Do a BUS Reset
                DEBUG_COMMENT("Resetting device ....\n");
                libusb_reset_device(handle);
                DEBUG_COMMENT("Closing device ....\n");
                libusb_close(handle);
                DEBUG_COMMENT("Reopening device ....\n");
                libusb_open(dev, &handle);

                DEBUG_PRINT("bNumConfigurations = %d\n", desc.bNumConfigurations);
                if(!desc.bNumConfigurations)
                {
                    DEBUG_COMMENT("No valid configurations\n");
                    continue;
                }

                libusb_config_descriptor *config;
                rc = libusb_get_config_descriptor(dev, 0, &config);
                if(rc != LIBUSB_SUCCESS)
                {
                    DEBUG_PRINT("Unable to get config rc=%d\n", rc);
                    libusb_free_config_descriptor(config);
                    continue;
                }

                DEBUG_PRINT("Number of Interfaces : %d\n", config->bNumInterfaces);
                if(config->bNumInterfaces != 1)
                {
                    DEBUG_COMMENT("Invalid number of interfaces.\n");
                    libusb_free_config_descriptor(config);
                    continue;
                }
                if(config->interface[0].num_altsetting != 1)
                {
                    DEBUG_COMMENT("Invalid number of alt settings.\n");
                    libusb_free_config_descriptor(config);
                    continue;
                }

                if(config->interface[0].altsetting[0].bNumEndpoints != 2)
                {
                    DEBUG_COMMENT("Invalid Number of endpoints.\n");
                    libusb_free_config_descriptor(config);
                    continue;
                }

                for (int i=0;i<2;i++)
                {
                    int ep = config->interface[0].altsetting[0].endpoint[i].bEndpointAddress;
                    if(ep & LIBUSB_ENDPOINT_DIR_MASK)
                    {
                        DEBUG_PRINT("Read Endpoint = 0x%02X (%d)\n", ep, i);
                        readEndpoint = ep;
                    } else {
                        DEBUG_PRINT("Write Endpoint = 0x%02X (%d)\n", ep, i);
                        writeEndpoint = ep;
                    }
                }

                DEBUG_COMMENT("Setting auto kernel detach.\n");
                if(rc = libusb_set_auto_detach_kernel_driver(handle, 1))
                {
                    DEBUG_PRINT("Unable to set auto kernel driver detach (rc=%d).\n", rc);
                    libusb_free_config_descriptor(config);
                    continue;
                }

                DEBUG_COMMENT("Claiming interface.....\n");
                if(rc = libusb_claim_interface(handle, 0))
                {
                    DEBUG_PRINT("Unable to claim interface (rc=%d).\n", rc);
                    libusb_free_config_descriptor(config);
                    continue;
                }

                usb_config = config;

                // Break out of loop
                break;
            }
        }
    }

    libusb_free_device_list(list, 0);

    if((readEndpoint < 0) || (writeEndpoint < 0))
    {
        DEBUG_COMMENT("Did not find valid device.\n");
        return ERROR;
    }

    return NOERROR;
}

int AntUsb::bulkRead(unsigned char *bytes, int size, int timeout)
{
    int actualSize;
    int rc = libusb_bulk_transfer(usb_handle, readEndpoint,
            bytes, size, &actualSize, timeout);

#ifdef DEBUG_OUTPUT
    char sb[100];
    bytestream_to_string(sb, bytes, actualSize);
    DEBUG_PRINT("Recieved %d : %s\n", actualSize, sb);
#endif


    if (rc < 0 && rc != LIBUSB_ERROR_TIMEOUT)
    {
        DEBUG_PRINT("libusb_bulk_transfer failed with rc=%d\n", rc);
        return rc;
    }

    return actualSize;
}

int AntUsb::bulkWrite(unsigned char *bytes, int size, int timeout)
{
    int actualSize;
    int rc = libusb_bulk_transfer(usb_handle, writeEndpoint,
            bytes, size, &actualSize, timeout);

    if (rc < 0)
    {
        DEBUG_PRINT("libusb_bulk_transfer failed with rc=%d\n", rc);
        return rc;
    }

#ifdef DEBUG_OUTPUT
    char sb[100];
    bytestream_to_string(sb, bytes, actualSize);
    DEBUG_PRINT("Wrote %d : %s\n", actualSize, sb);
#endif

    return actualSize;
}

int AntUsb::sendMessage(AntMessage &message)
{
    unsigned char msg[ANT_MAX_MESSAGE_SIZE];
    int msg_len;
    message.encode(msg, &msg_len);
    return bulkWrite(msg, msg_len, writeTimeout);
}

int AntUsb::readMessage(AntMessage *message)
{
    unsigned char bytes[ANT_MAX_MESSAGE_SIZE];
    int nbytes = bulkRead(bytes, ANT_MAX_MESSAGE_SIZE, readTimeout);
    if(nbytes)
    {
        message->decode(bytes, nbytes);
    } else {
        return -1;
    }

    return nbytes;
}

int AntUsb::reset(void)
{
    DEBUG_COMMENT("Sending ANT_SYSTEM_RESET\n");
    AntMessage resetMessage(ANT_SYSTEM_RESET, NULL, 1);
    sendMessage(resetMessage);

    usleep(500000L);

    AntMessage reply;
    readMessage(&reply);
    reply.parse();

    return 0;
}

int AntUsb::setNetworkKey(void)
{
    unsigned char key[] = ANT_NETWORK_KEY;

    DEBUG_COMMENT("Sending ANT_SET_NETWORK\n");
    AntMessage netkey(ANT_SET_NETWORK, key, ANT_NETWORK_KEY_LEN);
    return sendMessage(netkey);
}

int AntUsb::assignChannel(int chanNum, bool master)
{
    int rc;
    AntMessage reply;

    DEBUG_COMMENT("Sending ANT_UNASSIGN_CHANNEL\n");
    AntMessage unassign(ANT_UNASSIGN_CHANNEL, chanNum);
    rc = sendMessage(unassign);
    rc = readMessage(&reply);
    reply.parse();

    DEBUG_COMMENT("Sending ANT_ASSIGN_CHANNEL\n");
    AntMessage assign(ANT_ASSIGN_CHANNEL, (unsigned int)chanNum,
            master ? CHANNEL_TYPE_TX : CHANNEL_TYPE_RX,
            0); // receive channel on network 1
    rc = sendMessage(assign);
    rc = readMessage(&reply);
    reply.parse();
}


int main(int argc, char *argv[])
{
    unsigned char bytes[256];
    int nbytes;
    AntUsb antusb;

    antusb.init();
    antusb.setup();

    antusb.reset();

    antusb.setNetworkKey();
    nbytes = antusb.bulkRead(bytes, 256, 256);

    antusb.assignChannel(0, 0);
//nbytes = antusb.bulkRead(bytes, 256, 256);
    return 0;
}
