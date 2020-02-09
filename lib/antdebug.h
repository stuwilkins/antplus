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

#ifndef ANTPLUS_LIB_ANTDEBUG_H_
#define ANTPLUS_LIB_ANTDEBUG_H_

#include <stdint.h>
#include <cstdio>

extern int _debug_output;

void bytestream_to_string(char *out, size_t n_out,
        uint8_t *bytes, size_t n_bytes);

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#ifdef DEBUG_OUTPUT
#define DEBUG_PRINT(fmt, ...) \
  if (_debug_output) fprintf(stderr, "%-25s:%5d : %-20s: %p" fmt, \
          __FILENAME__, __LINE__, __func__, (void*)this, __VA_ARGS__);
#define DEBUG_COMMENT(fmt) \
  if (_debug_output) fprintf(stderr, "%-25s:%5d : %-20s: %p" fmt, \
          __FILENAME__, __LINE__, __func__, (void*)this);
#else
#define DEBUG_PRINT(fmt, ...) \
    do {} while (0)
#define DEBUG_COMMENT(fmt) \
    do {} while (0)
#endif

#define UNUSED(expr) do { (void)(expr); } while (0)

#endif  // ANTPLUS_LIB_ANTDEBUG_H_
