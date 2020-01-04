/*
 * =====================================================================================
 *
 *       Filename:  ant.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  01/03/2020 11:31:41 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stuart B. Wilkins (sbw), stuart@stuwilkins.org
 *   Organization:
 *
 * =====================================================================================
 */

#include <cstdio>
#include <string.h>

#include "ant.h"
#include "debug.h"

void bytestream_to_string(char *out, unsigned char *bytes, int n_bytes)
{
    if(n_bytes == 0)
    {
        out[0] = 0;
        return;
    }

    char *_dbmsg = out;
    for(int i=0;i<n_bytes;i++)
    {
        snprintf(_dbmsg, 4, "%02X:", bytes[i]);
        _dbmsg += 3;
    }
    *(_dbmsg-1) = 0;
}

AntMessage::AntMessage(void)
{
    antType = 0x00;
    for(int i=0;i<ANT_MAX_DATA_SIZE;i++)
    {
        antData[i] = 0x00;
    }
    antDataLen = 0;
}

AntMessage::AntMessage(unsigned char *data, int data_len)
    : AntMessage()
{
    // Decode
    this->decode(data, data_len);
}

AntMessage::AntMessage(unsigned char type, unsigned char *data, int len)
    : AntMessage()
{
    // Copy to internal structure
    antType = type;
    antDataLen = len;
    if(data == NULL)
    {
        return;
    }

    for(int i=0;i<len;i++)
    {
        antData[i] = data[i];
    }
}

AntMessage::AntMessage(unsigned char type, unsigned char b0)
    : AntMessage()
{
    antType = type;
    antDataLen = 1;
    antData[0] = b0;
}

AntMessage::AntMessage(unsigned char type, unsigned char b0,
        unsigned char b1)
    : AntMessage()
{
    antType = type;
    antDataLen = 2;
    antData[0] = b0;
    antData[1] = b1;
}

AntMessage::AntMessage(unsigned char type, unsigned char b0,
        unsigned char b1, unsigned char b2)
    : AntMessage()
{
    antType = type;
    antDataLen = 3;
    antData[0] = b0;
    antData[1] = b1;
    antData[2] = b2;
}

int AntMessage::decode(unsigned char *data, int data_len)
{
    if(data_len < 3)
    {
        DEBUG_COMMENT("Data too short (< 3)\n");
        return ANT_DECODE_LEN_ERROR;
    }

    if(data[0] != ANT_SYNC_BYTE)
    {
        DEBUG_COMMENT("First byte does not match SYNC\n");
        return ANT_DECODE_PROTO_ERROR;
    }

    if(data[1] != (data_len - 4))
    {
        DEBUG_COMMENT("Data length does not match length in payload.");
        return ANT_DECODE_LEN_ERROR;
    }

    // Check CRC

    unsigned char crc = 0;
    for(int i=0;i<(data_len-1);i++)
    {
        crc ^= data[i];
    }
    if(data[3+data[1]] != crc)
    {
        DEBUG_COMMENT("CRC MISMACH\n");
        return ANT_DECODE_CRC_ERROR;
    }

    // Now create the message structure
    // We have a copy but its probably safer ....

    antType = data[2];
    antDataLen = data[1];
    for(int i=0;i<data[1];i++)
    {
        antData[i] = data[3+i];
    }

    return ANT_DECODE_OK;
}

void AntMessage::encode(unsigned char *msg, int *len)
{
    msg[0] = ANT_SYNC_BYTE;
    msg[1] = antDataLen;
    msg[2] = antType;

    for(int i=0;i<antDataLen;i++)
    {
        msg[i+3] = antData[i];
    }

    unsigned char crc = 0;
    for(int i=0;i<(antDataLen+3);i++)
    {
        crc ^= msg[i];
    }

    msg[antDataLen+3] = crc;

    *len = antDataLen + 4;

#ifdef DEBUG_OUTPUT
    char bytes[100];
    bytestream_to_string(bytes, msg, *len);
    DEBUG_PRINT("ANT Message : %s\n",  bytes);
#endif
}

int AntMessage::parse(void)
{
    int rtn = 0;

    DEBUG_PRINT("Parsing antType = 0x%02X\n", antType);

    switch(antType)
    {
        case ANT_NOTIF_STARTUP:
            DEBUG_COMMENT("RESET OK\n");
            break;
        case ANT_ACK_DATA:
        case ANT_BROADCAST_DATA:
        case ANT_CHANNEL_STATUS:
        case ANT_CHANNEL_ID:
        case ANT_BURST_DATA:
            parseChannelEvent();
            break;
        case ANT_CHANNEL_EVENT:
            switch(antData[ANT_OFFSET_MESSAGE_CODE])
            {
                case EVENT_TRANSFER_TX_FAILED:
                    DEBUG_COMMENT("Transfer FAILED\n");
                    break;
                default:
                    parseChannelEvent();
            }
            break;
        default:
            DEBUG_COMMENT("Unknown ANT Type\n");
            rtn = -1;
            break;
    }

    return rtn;
}

int AntMessage::parseChannelEvent(void)
{
    int channel = antData[ANT_OFFSET_DATA];
    DEBUG_PRINT("Channel Event on channel %d\n", channel);
}
