// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/IMU/Filter.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Filter)
{
    class_<gz4d::IMU::Filter, bases<gz4d::Producer<gz4d::IMU::State> > >("Filter")
            .def(init<optional<gz4d::IMU::Calibration const&> >())
            .def("Consume", &gz4d::IMU::Filter::Consume)
            .def("__call__", &gz4d::IMU::Filter::operator())

            .def_readwrite("mQ", &gz4d::IMU::Filter::mQ)
            .def_readwrite("aQ", &gz4d::IMU::Filter::aQ)
            .def_readwrite("gQ", &gz4d::IMU::Filter::gQ)
            .def_readwrite("mR", &gz4d::IMU::Filter::mR)
            .def_readwrite("aR", &gz4d::IMU::Filter::aR)
            .def_readwrite("gR", &gz4d::IMU::Filter::gR)
            ;
}
