/*
 * =====================================================================================
 *
 *       Filename:  antusb.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/02/2020 07:25:44 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stuart B. Wilkins (sbw), stuart@stuwilkins.org
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef _ANTUSB_H
#define _ANTUSB_H

#include <libusb-1.0/libusb.h>

#define GARMIN_USB2_VID   0x0fcf
#define GARMIN_USB2_PID   0x1008
#define GARMIN_OEM_PID    0x1009

class AntUsb {
    public:
        enum rtn
        {
            NOERROR = 0,
            ERROR = -1
        };
        AntUsb(void);
        ~AntUsb(void);
        int init(void);
        int setup(void);
        int bulk_read(unsigned char *bytes, int size, int timeout);
        int bulk_write(unsigned char *bytes, int size, int timeout);
    private:
        libusb_context *usb_ctx;
        libusb_device_handle *usb_handle;
        libusb_config_descriptor *usb_config;
        int readEndpoint;
        int writeEndpoint;
};

#endif
