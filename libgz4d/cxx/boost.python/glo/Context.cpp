// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/glo/Context.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Context)
{
    class_<gz4d::glo::Context, bases<>, std::shared_ptr<gz4d::glo::Context> >("Context",no_init)
    .def("create",&gz4d::glo::Context::create)
    .staticmethod("create")
    ;
}
