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

int ant_create_message(unsigned char *data, int *data_len,
        unsigned char type, unsigned char *msg, int len)
{
    data[0] = ANT_SYNC_BYTE;
    data[1] = len;
    data[2] = type;

    for(int i=0;i<len;i++)
    {
        data[i+3] = msg[i];
    }
    for(int i=len;i<(ANT_MAX_MESSAGE_SIZE-3);i++)
    {
        data[i+3] = 0x0;
    }

    unsigned char crc = 0;
    for(int i=0;i<(len+3);i++)
    {
        crc ^= data[i];
    }

    data[len+3] = crc;

    *data_len = len + 4;

#ifdef DEBUG_OUTPUT
    char dbmsg[256];
    sprintf(dbmsg, "ANT Message : 0x");
    char *_dbmsg = dbmsg + strlen(dbmsg);
    for(int i=0;i<*data_len;i++)
    {
        snprintf(_dbmsg, 3, "%02X", data[i]);
        _dbmsg += 2;
    }
    DEBUG_PRINT("%s\n", dbmsg);
#endif
}



