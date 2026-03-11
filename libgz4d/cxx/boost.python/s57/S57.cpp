// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/s57/S57.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(S57)
{
    class_<gz4d::s57::S57, bases<>, gz4d::s57::S57::Ptr >("S57",init<std::string const &>())
            ;
}
