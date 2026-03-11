// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/axis/ocr.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Ocr)
{
    def("ocr",&gz4d::axis::ocr);
}
