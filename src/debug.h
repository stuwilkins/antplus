/*
 * =====================================================================================
 *
 *       Filename:  debug.h
 *
 *    Description:  Debug output routines
 *
 *        Version:  1.0
 *        Created:  01/02/2020 08:14:52 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Stuart B. Wilkins (sbw), stuart@stuwilkins.org
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef _DEBUG_H
#define _DEBUG_H 1

#include <cstdio>

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#ifdef DEBUG_OUTPUT
#define DEBUG_PRINT(fmt, ...) \
  fprintf(stderr, "%s:%d:%s(): " fmt, \
          __FILENAME__, __LINE__, __func__, __VA_ARGS__);
#define DEBUG_COMMENT(fmt) \
  fprintf(stderr, "%s:%d:%s(): " fmt, \
          __FILENAME__, __LINE__, __func__);
#else
#define DEBUG_PRINT(fmt, ...) \
    do {} while (0)
#define DEBUG_COMMENT(fmt) \
    do {} while (0)
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif
