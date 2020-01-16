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

#ifndef SRC_ANTDEFS_H_
#define SRC_ANTDEFS_H_

#define MAX_MESSAGE_SIZE                    2048

// ANT Messages
#define ANT_SYNC_BYTE                       0xA4
#define ANT_SYSTEM_RESET                    0x4A
#define ANT_UNASSIGN_CHANNEL                0x41
#define ANT_ASSIGN_CHANNEL                  0x42
#define ANT_CHANNEL_PERIOD                  0x43
#define ANT_SEARCH_TIMEOUT                  0x44
#define ANT_CHANNEL_FREQUENCY               0x45
#define ANT_SET_NETWORK                     0x46
#define ANT_TX_POWER                        0x47
#define ANT_ID_LIST_ADD                     0x59
#define ANT_ID_LIST_CONFIG                  0x5A
#define ANT_CHANNEL_TX_POWER                0x60
#define ANT_LP_SEARCH_TIMEOUT               0x63
#define ANT_SET_SERIAL_NUMBER               0x65
#define ANT_ENABLE_EXT_MSGS                 0x66
#define ANT_ENABLE_LED                      0x68
#define ANT_LIB_CONFIG                      0x6E
#define ANT_SYSTEM_RESET                    0x4A
#define ANT_OPEN_CHANNEL                    0x4B
#define ANT_CLOSE_CHANNEL                   0x4C
#define ANT_OPEN_RX_SCAN_CH                 0x5B
#define ANT_REQ_MESSAGE                     0x4D
#define ANT_BROADCAST_DATA                  0x4E
#define ANT_ACK_DATA                        0x4F
#define ANT_BURST_DATA                      0x50
#define ANT_CHANNEL_EVENT                   0x40
#define ANT_CHANNEL_STATUS                  0x52
#define ANT_CHANNEL_ID                      0x51
#define ANT_VERSION                         0x3E
#define ANT_CAPABILITIES                    0x54
#define ANT_SERIAL_NUMBER                   0x61
#define ANT_NOTIF_STARTUP                   0x6F
#define ANT_CW_INIT                         0x53
#define ANT_CW_TEST                         0x48

#define ANT_TX_TYPE_SLAVE                   0x00
#define ANT_TX_TYPE_MASTER                  0x05

#define CHANNEL_TYPE_RX                     0x00
#define CHANNEL_TYPE_TX                     0x10
#define CHANNEL_TYPE_RX_SHARED              0x20
#define CHANNEL_TYPE_TX_SHARED              0x30
#define CHANNEL_TYPE_RX_ONLY                0x40
#define CHANNEL_TYPE_TX_ONLY                0x50

#define CHANNEL_TYPE_EXT_BACKGROUND_SCAN    0x01
#define CHANNEL_TYPE_EXT_FREQ_AGILITY       0x02
#define CHANNEL_TYPE_EXT_FAST_CHANNEL       0x10
#define CHANNEL_TYPE_EXT_ASYNC              0x20

// Channel messages
#define RESPONSE_NO_ERROR                   0x00
#define EVENT_RX_SEARCH_TIMEOUT             0x01
#define EVENT_RX_FAIL                       0x02
#define EVENT_TX                            0x03
#define EVENT_TRANSFER_RX_FAILED            0x04
#define EVENT_TRANSFER_TX_COMPLETED         0x05
#define EVENT_TRANSFER_TX_FAILED            0x06
#define EVENT_CHANNEL_CLOSED                0x07
#define EVENT_RX_BROADCAST                  0x0A
#define EVENT_RX_ACKNOWLEDGED               0x0B
#define EVENT_RX_BURST_PACKET               0x0C
#define CHANNEL_IN_WRONG_STATE              0x15
#define CHANNEL_NOT_OPENED                  0x16
#define CHANNEL_ID_NOT_SET                  0x17
#define TRANSFER_IN_PROGRESS                0x1F
#define TRANSFER_SEQUENCE_NUMBER_ERROR      0x20
#define INVALID_MESSAGE                     0x28
#define INVALID_NETWORK_NUMBER              0x29

#define ANT_OFFSET_DATA                     0
#define ANT_OFFSET_CHANNEL_NUMBER           0
#define ANT_OFFSET_MESSAGE_ID               1
#define ANT_OFFSET_MESSAGE_CODE             2

#define GARMIN_USB2_VID                     0x0FCF
#define GARMIN_USB2_PID                     0x1008
#define GARMIN_OEM_PID                      0x1009
//
// Channel Responses

#define ANT_DEVICE_HR_PREVIOUS              0x04

// Extended format settings

#define ANT_EXT_MSG_TIMESTAMP               0x20
#define ANT_EXT_MSG_RSSI                    0x40
#define ANT_EXT_MSG_CHAN_ID                 0x80

// ANT Device Settings

#define ANT_DEVICE_COMMON_STATUS            0x47

#define ANT_DEVICE_FEC_GENERAL              0x10
#define ANT_DEVICE_FEC_GENERAL_SETTINGS     0x11
#define ANT_DEVICE_FEC_TRAINER              0x19
#define ANT_DEVICE_FEC_COMMAND_RESISTANCE   0x30
#define ANT_DEVICE_FEC_COMMAND_POWER        0x31
#define ANT_DEVICE_FEC_COMMAND_WIND         0x32
#define ANT_DEVICE_FEC_COMMAND_TRACK        0x33

#define ANT_DEVICE_POWER_STANDARD           0x10
#define ANT_DEVICE_POWER_TEPS               0x13
#define ANT_DEVICE_POWER_BATTERY            0x52
#define ANT_DEVICE_POWER_PARAMS             0x02
#define ANT_DEVICE_POWER_PARAMS_CRANK       0x01
#define ANT_DEVICE_POWER_PARAMS_TORQUE      0x02

#define ANT_DEVICE_HR_PREVIOUS              0x04


#endif  // SRC_ANTDEFS_H_
