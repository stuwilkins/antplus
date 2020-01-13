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
#include <H5Cpp.h>
#include <csignal>

#include "debug.h"
#include "ant.h"
#include "antchannel.h"
#include "antdevice.h"
#include "anthdf5.h"

const H5std_string  FILE_NAME("SDS.h5");

bool stop = false;

void signalHandler(int signum) {
    (void)signum;
    stop = true;
}

int AntUsbHDF5::writeData(std::string filename) {
    H5::H5File file(filename, H5F_ACC_TRUNC);

    file.createGroup("/DATA");
    file.createGroup("/TIMESTAMP");

    // Cycle through each channel
    for (int i=0; i < getNumChannels(); i++) {
        DEBUG_PRINT("Processing channel %d\n", i);
        AntChannel *chan = getChannel(i);
        AntDevice *dev = chan->getDevice();
        if (dev != nullptr) {
            DEBUG_PRINT("Channel %d Values %d\n",
                    i, dev->getNumValues());
            std::string devName = dev->getDeviceName() + '_';
            devName = devName + std::to_string(chan->getDeviceID());
            DEBUG_PRINT("Channel %d devName = %s\n", i, devName.c_str());

            file.createGroup("/DATA/" + devName);
            file.createGroup("/TIMESTAMP/" + devName);

            for (int j=0; j < dev->getNumValues(); j++) {
                // This is for each value
                DEBUG_PRINT("Channel %d Datapoints %ld\n",
                        i, dev->getData(j).size());

                // For the values create an array
                int size = dev->getData(j).size();
                if (size > 0) {
                    hsize_t dimsf[1];
                    dimsf[0] = size;

                    // First do the values

                    float *val = new float[size];
                    for (int i = 0; i < size; i++) {
                        val[i] = dev->getData(j)[i].getValue();
                    }

                    H5::DataSpace dataspace(1, dimsf);
                    H5::IntType datatype(H5::PredType::NATIVE_FLOAT);
                    datatype.setOrder(H5T_ORDER_LE);

                    std::string name = "/DATA/" + devName;
                    name = name + "/" + dev->getValueNames()[j];
                    DEBUG_PRINT("Writing node %s\n", name.c_str());

                    H5::DataSet dataset = file.createDataSet(name,
                            datatype, dataspace);
                    dataset.write(val, H5::PredType::NATIVE_FLOAT);

                    // Now do the timestamps

                    H5::DataSpace tdataspace(1, dimsf);
                    H5::IntType tdatatype(H5::PredType::NATIVE_UINT64);
                    tdatatype.setOrder(H5T_ORDER_LE);

                    uint64_t *tval = new uint64_t[size];
                    for (int i = 0; i < size; i++) {
                        auto ms = std::chrono::duration_cast
                            <std::chrono::milliseconds>
                            (dev->getData(j)[i].getTimestamp() - startTime);
                        tval[i] = ms.count();
                    }

                    name = "/TIMESTAMP/" + devName;
                    name = name + "/" + dev->getValueNames()[j];
                    DEBUG_PRINT("Writing node %s\n", name.c_str());

                    H5::DataSet tdataset = file.createDataSet(name,
                            tdatatype, tdataspace);
                    tdataset.write(tval, H5::PredType::NATIVE_UINT64);

                    delete [] tval;
                    delete [] val;
                }
            }
        } else {
            DEBUG_PRINT("No device on channel %d\n", i);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    AntUsbHDF5 antusb;

    antusb.init();
    antusb.setup();

    antusb.reset();

    antusb.startListener();

    antusb.setNetworkKey(0);

    // antusb.channelStart(0, AntDevice::TYPE_HR, 0x01E5);
    // antusb.channelStart(1, AntDevice::TYPE_PWR, 0xD42D);
    // antusb.channelStart(2, AntDevice::TYPE_PWR, 0x635E);
    antusb.channelStart(3, AntDevice::TYPE_FEC, 0x635E);

    signal(SIGINT, signalHandler);
    while (!stop) {
        sleep(1);
    }

    antusb.writeData("data.h5");

    return 0;
}

