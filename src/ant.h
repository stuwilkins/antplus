/*
 * =====================================================================================
 *
 *       Filename:  ant.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/03/2020 11:29:14 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stuart B. Wilkins (sbw), stuart@stuwilkins.org
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef _ANT_H
#define _ANT_H

#include "ant.h"
#include "ant_network_key.h"

#define ANT_DECODE_LEN_ERROR            -1
#define ANT_DECODE_CRC_ERROR            -2
#define ANT_DECODE_OK                    0
#define ANT_DECODE_PROTO_ERROR          -3

#define ANT_SYNC_BYTE                   0xA4
#define ANT_MAX_MESSAGE_SIZE            13
#define ANT_MAX_DATA_SIZE               9
#define ANT_SYSTEM_RESET                0x4A
#define ANT_SET_NETWORK                 0x46
#define ANT_NETWORK_KEY_LEN             9

// ANT messages
#define ANT_UNASSIGN_CHANNEL            0x41
#define ANT_ASSIGN_CHANNEL              0x42
#define ANT_CHANNEL_ID                  0x51
#define ANT_CHANNEL_PERIOD              0x43
#define ANT_SEARCH_TIMEOUT              0x44
#define ANT_CHANNEL_FREQUENCY           0x45
#define ANT_SET_NETWORK                 0x46
#define ANT_TX_POWER                    0x47
#define ANT_ID_LIST_ADD                 0x59
#define ANT_ID_LIST_CONFIG              0x5A
#define ANT_CHANNEL_TX_POWER            0x60
#define ANT_LP_SEARCH_TIMEOUT           0x63
#define ANT_SET_SERIAL_NUMBER           0x65
#define ANT_ENABLE_EXT_MSGS             0x66
#define ANT_ENABLE_LED                  0x68
#define ANT_SYSTEM_RESET                0x4A
#define ANT_OPEN_CHANNEL                0x4B
#define ANT_CLOSE_CHANNEL               0x4C
#define ANT_OPEN_RX_SCAN_CH             0x5B
#define ANT_REQ_MESSAGE                 0x4D
#define ANT_BROADCAST_DATA              0x4E
#define ANT_ACK_DATA                    0x4F
#define ANT_BURST_DATA                  0x50
#define ANT_CHANNEL_EVENT               0x40
#define ANT_CHANNEL_STATUS              0x52
#define ANT_CHANNEL_ID                  0x51
#define ANT_VERSION                     0x3E
#define ANT_CAPABILITIES                0x54
#define ANT_SERIAL_NUMBER               0x61
#define ANT_NOTIF_STARTUP               0x6F
#define ANT_CW_INIT                     0x53
#define ANT_CW_TEST                     0x48

#define ANT_TX_TYPE_SLAVE               0x00
#define ANT_TX_TYPE_MASTER              0x05

#define CHANNEL_TYPE_QUICK_SEARCH       0x10
#define CHANNEL_TYPE_WAITING            0x20
#define CHANNEL_TYPE_RX                 0x0
#define CHANNEL_TYPE_TX                 0x10
#define CHANNEL_TYPE_PAIR               0x40

// Channel messages
#define RESPONSE_NO_ERROR                0
#define EVENT_RX_SEARCH_TIMEOUT          1
#define EVENT_RX_FAIL                    2
#define EVENT_TX                         3
#define EVENT_TRANSFER_RX_FAILED         4
#define EVENT_TRANSFER_TX_COMPLETED      5
#define EVENT_TRANSFER_TX_FAILED         6
#define EVENT_CHANNEL_CLOSED             7
#define EVENT_RX_BROADCAST              10
#define EVENT_RX_ACKNOWLEDGED           11
#define EVENT_RX_BURST_PACKET           12
#define CHANNEL_IN_WRONG_STATE          21
#define CHANNEL_NOT_OPENED              22
#define CHANNEL_ID_NOT_SET              24
#define TRANSFER_IN_PROGRESS            31
#define TRANSFER_SEQUENCE_NUMBER_ERROR  32
#define INVALID_MESSAGE                 40
#define INVALID_NETWORK_NUMBER          41

#define ANT_OFFSET_DATA                 0
#define ANT_OFFSET_CHANNEL_NUMBER       0
#define ANT_OFFSET_MESSAGE_ID           1
#define ANT_OFFSET_MESSAGE_CODE         2

#ifndef ANT_NETWORK_KEY
#define ANT_NETWORK_KEY { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#define ANT_NETWORK_KEY_LEN     8
#endif

void bytestream_to_string(char *out, unsigned char *bytes, int n_bytes);

class AntMessage {
    public:
        AntMessage(void);
        AntMessage(unsigned char *data, int data_len);
        AntMessage(unsigned char type, unsigned char *data, int len);
        AntMessage(unsigned char type, unsigned char b0);
        AntMessage(unsigned char type, unsigned char b0, unsigned char b1);
        AntMessage(unsigned char type, unsigned char b0, unsigned char b1,
                unsigned char b2);
        void encode(unsigned char *msg, int *len);
        int decode(unsigned char *data, int data_len);
    int parse(void);
    int parseChannelEvent(void);
    private:
        unsigned char antType;
        unsigned char antData[ANT_MAX_DATA_SIZE];
        int antDataLen;
};

//class AntMessage
//{
//    private:
//        int createMessage(unsigned char *msg, int len);
//};

#endif
