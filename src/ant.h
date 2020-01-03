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

#define ANT_SYNC_BYTE           0xA4
#define ANT_MAX_MESSAGE_SIZE    12
#define ANT_SYSTEM_RESET        0x4A

int ant_create_message(unsigned char *data, int *data_len,
        unsigned char type, unsigned char *msg, int len);

//class AntMessage
//{
//    private:
//        int createMessage(unsigned char *msg, int len);
//};

#endif
