// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/Exif.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Exif)
{
    class_<gz4d::Exif>("Exif")
    .def(init<std::string const &>())
    .def("Get", &gz4d::Exif::Get)
    .def("GetFloat", &gz4d::Exif::GetFloat)
    .def("GetShort", &gz4d::Exif::GetShort)
    .def("GetString", &gz4d::Exif::GetString)
    .def("GetFOV", &gz4d::Exif::GetFOV)
    ;
}
