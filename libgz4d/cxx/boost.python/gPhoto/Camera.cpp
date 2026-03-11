// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/gPhoto/Camera.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Camera)
{

    class_<gz4d::gphoto::Camera>("Camera")
            .def("Capture", &gz4d::gphoto::Camera::Capture)
            ;

}
