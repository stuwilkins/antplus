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

#include <unistd.h>
#include <pthread.h>

#include "debug.h"
#include "ant.h"
#include "antusb.h"

int main(int argc, char *argv[]) {
    AntUsb antusb;

    antusb.init();
    antusb.setup();

    antusb.reset();

    antusb.setNetworkKey();
    antusb.assignChannel(1, 0);
    antusb.setChannelID(1, 0, 0x11, 0);
    antusb.setSearchTimeout(1, 12);
    antusb.setChannelPeriod(1, 8192);
    antusb.setChannelFreq(1, 57);

    antusb.assignChannel(2, 0);
    antusb.setChannelID(2, 54317, 0x0B, 0);
    antusb.setSearchTimeout(2, 12);
    antusb.setChannelPeriod(2, 8182);
    antusb.setChannelFreq(2, 57);

    antusb.assignChannel(3, 0);
    antusb.setChannelID(3, 0, 0x78, 0);
    antusb.setSearchTimeout(3, 12);
    antusb.setChannelPeriod(3, 8070);
    antusb.setChannelFreq(3, 57);

    antusb.openChannel(1);
    antusb.openChannel(2);
    antusb.openChannel(3);

    antusb.startListener();

    for (;;) { }

    return 0;
}

