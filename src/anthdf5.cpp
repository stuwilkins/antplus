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
#include <getopt.h>
#include <csignal>
#include <iostream>
#include <libconfig.h++>

#include "debug.h"
#include "ant.h"
#include "antusbinterface.h"
#include "antchannel.h"
#include "antdevice.h"
#include "anthdf5.h"

const H5std_string  FILE_NAME("SDS.h5");

bool stop = false;

void signalHandler(int signum) {
    (void)signum;
    stop = true;
}

int write_data(ANT *antusb, std::string filename) {
    H5::H5File file(filename, H5F_ACC_TRUNC);

    file.createGroup("/DATA");
    file.createGroup("/TIMESTAMP");
    file.createGroup("/METADATA");

    // Cycle through each channel
    for (ANTChannel& chan : antusb->getChannels()) {
        // And cycle through each device
        for (auto dev : chan.getDeviceList()) {
            std::string devName = dev->getDeviceName();
            devName = '_' + devName + std::to_string(
                    dev->getDeviceID().getID());

            DEBUG_PRINT("Processing Channel %d "
                    "(devName = %s)\n",
                    chan.getChannelNum(), devName.c_str());

            file.createGroup("/DATA/" + devName);
            file.createGroup("/TIMESTAMP/" + devName);
            file.createGroup("/METADATA/" + devName);

            for (const auto& tsDataPair : dev->getTsData()) {
                auto values = tsDataPair.second;
                auto valueName = tsDataPair.first;
                if (values.size()) {
                    DEBUG_PRINT("Channel %d Name %s Datapoints %ld\n",
                            chan.getChannelNum(),
                            valueName.c_str(), values.size());

                    hsize_t dimsf[1];
                    dimsf[0] = values.size();

                    // First do the values

                    float *val = new float[values.size()];
                    for (uint64_t i = 0; i < values.size(); i++) {
                        val[i] = values[i].getValue();
                    }

                    H5::DataSpace dataspace(1, dimsf);
                    H5::IntType datatype(H5::PredType::NATIVE_FLOAT);
                    datatype.setOrder(H5T_ORDER_LE);

                    std::string name;
                    name = "/DATA/" + devName;
                    name += "/" + valueName;
                    DEBUG_PRINT("Writing node %s\n", name.c_str());

                    H5::DataSet dataset = file.createDataSet(name,
                            datatype, dataspace);
                    dataset.write(val, H5::PredType::NATIVE_FLOAT);

                    // Now do the timestamps

                    H5::DataSpace tdataspace(1, dimsf);
                    H5::IntType tdatatype(H5::PredType::NATIVE_UINT64);
                    tdatatype.setOrder(H5T_ORDER_LE);

                    uint64_t *tval = new uint64_t[values.size()];
                    for (uint64_t i=0; i < values.size(); i++) {
                        auto ms = std::chrono::duration_cast
                            <std::chrono::milliseconds>
                            (values[i].getTimestamp()
                             - antusb->getStartTime());
                        tval[i] = ms.count();
                    }

                    name = "/TIMESTAMP/" + devName;
                    name += "/" + valueName;
                    DEBUG_PRINT("Writing node %s\n", name.c_str());

                    H5::DataSet tdataset = file.createDataSet(name,
                            tdatatype, tdataspace);
                    tdataset.write(tval, H5::PredType::NATIVE_UINT64);

                    delete [] tval;
                    delete [] val;
                }
            }

            hsize_t dimsf[1];
            dimsf[0] = 1;
            H5::DataSpace mdataspace(1, dimsf);
            H5::IntType mdatatype(H5::PredType::NATIVE_FLOAT);
            mdatatype.setOrder(H5T_ORDER_LE);

            for (const auto& metaDataPair : dev->getMetaData()) {
                std::string name = "/METADATA/" + devName;
                name = name + "/" + metaDataPair.first;
                DEBUG_PRINT("Writing node %s\n", name.c_str());
                H5::DataSet mdataset = file.createDataSet(name,
                        mdatatype, mdataspace);
                mdataset.write(&metaDataPair.second,
                        H5::PredType::NATIVE_FLOAT);
            }
        }
    }

    return 0;
}

int read_config(ANT *usb, std::string filename) {
    libconfig::Config cfg;

    // Read the file. If there is an error, report it and exit.
    try {
        cfg.readFile(filename.c_str());
    }
    catch(const libconfig::FileIOException &fioex) {
        std::cerr << "I/O error while reading file." << std::endl;
        return -1;
    }
    catch(const libconfig::ParseException &pex) {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
            << " - " << pex.getError() << std::endl;
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    std::string config_file("config.cfg");

    int c;
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"verbose", no_argument,       0, 'v'},
            {"config",  required_argument, 0, 'c'},
            {0,         0,                 0, 0  }
        };

        c = getopt_long(argc, argv, "vc:", long_options,
                &option_index);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'c':
                config_file = optarg;
                std::cout << config_file << std::endl;
                break;
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    ANTUSBInterface iface;

    if (iface.open()) {
        return -127;
    }

    ANT antusb(&iface);

    if (antusb.reset()) {
        return -127;
    }

    antusb.startThreads();

    antusb.setNetworkKey(0);

    antusb.channelStart(0, ANTChannel::TYPE_PAIR, 0x0000, true);
    // antusb.channelStart(0, ANTDevice::TYPE_HR, 0x01E5);
    // antusb.channelStart(1, ANTDevice::TYPE_PWR, 0xD42D);
    // antusb.channelStart(2, ANTDevice::TYPE_PWR, 0x635E);
    // antusb.channelStart(3, ANTDevice::TYPE_FEC, 0x635E);

    signal(SIGINT, signalHandler);
    while (!stop) {
        usleep(100000L);
    }

    antusb.stopThreads();

    write_data(&antusb, "data.h5");

    return 0;
}
