// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/nav/Attitude.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Attitude)
{
    class_<gz4d::nav::Attitude<float>, bases<gz4d::nav::Heading<float> > >("Attitude")
            .def_readwrite("pitch", &gz4d::nav::Attitude<float>::pitch)
            .def_readwrite("roll", &gz4d::nav::Attitude<float>::roll)
            ;
}
