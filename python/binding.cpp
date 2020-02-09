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

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/stl_bind.h>
#include <vector>
#include <memory>

#include "antplus.h"

using std::shared_ptr;

namespace py = pybind11;
using pybind11::literals::operator""_a;

PYBIND11_MAKE_OPAQUE(std::vector<float>)
PYBIND11_MAKE_OPAQUE(ANTTsData)
PYBIND11_MAKE_OPAQUE(ANTMetaData)

PYBIND11_MODULE(_pyantplus, m) {
    m.doc() = "ANT+ Utilities";

    m.def("set_debug", &antplus_set_debug);

    py::bind_vector<std::vector<float>>(m, "VectorFloat",
        py::buffer_protocol());
    py::bind_map<ANTTsData>(m, "TimeSeriesMap",
        py::buffer_protocol());
    py::bind_map<ANTMetaData>(m, "MetaDataMap",
        py::buffer_protocol());

    py::class_<ANTUSBInterface,
        shared_ptr<ANTUSBInterface>>(m, "ANTUSBInterface")
        .def(py::init())
        .def("open", &ANTUSBInterface::open)
        .def("close", &ANTUSBInterface::close);

    py::class_<ANT>(m, "ANT")
        .def(py::init<shared_ptr<ANTUSBInterface>>())
        .def("init", &ANT::init)
        .def("getChannel", &ANT::getChannel)
        .def("getChannels", &ANT::getChannels);

    py::class_<ANTChannel, shared_ptr<ANTChannel>>
        antchannel(m, "ANTChannel");
        antchannel.def("open", &ANTChannel::open,
            "type"_a, "id"_a = 0x0000, "wait"_a = 1);
        antchannel.def("close", &ANTChannel::close);
        antchannel.def("getDeviceList", &ANTChannel::getDeviceList);

    py::enum_<ANTChannel::TYPE>(m, "TYPE")
        .value("NONE", ANTChannel::TYPE_NONE)
        .value("PAIR", ANTChannel::TYPE_PAIR)
        .value("HR", ANTChannel::TYPE_HR)
        .value("PWR", ANTChannel::TYPE_PWR)
        .value("FEC", ANTChannel::TYPE_FEC);

    py::class_<ANTDevice, shared_ptr<ANTDevice>>(m, "ANTDevice")
        .def(py::init<>())
        .def("getDeviceID", &ANTDevice::getDeviceID)
        .def("getDeviceName", &ANTDevice::getDeviceName)
        .def("getTsData", &ANTDevice::getTsData)
        // .def("getData", &ANTDevice::getData)
        .def("getMetaData", &ANTDevice::getMetaData);

    py::class_<ANTDeviceID>(m, "ANTDeviceID")
        .def(py::init<>())
        .def("getID", &ANTDeviceID::getID)
        .def("getType", &ANTDeviceID::getType)
        .def("isValid", &ANTDeviceID::isValid);

    py::class_<ANTDeviceData<float>, shared_ptr<ANTDeviceData<float>>>
        (m, "ANTDeviceData")
        .def(py::init<>())
        .def("getValue", &ANTDeviceData<float>::getValue)
        .def("getTimestamp", &ANTDeviceData<float>::getTimestamp);

     m.attr("__version__") = ANTPLUS_GIT_VERSION;
}
