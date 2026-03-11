// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/glo/MultMatrix.h>

using namespace boost::python;

void (*mmf)(gz4d::Matrix<float,4,4> const &) = &gz4d::glo::MultMatrix;
void (*mmd)(gz4d::Matrix<double,4,4> const &) = &gz4d::glo::MultMatrix;

BOOST_PYTHON_MODULE(MultMatrix)
{
    def("MultMatrix",mmf);
    def("MultMatrix",mmd);
}
