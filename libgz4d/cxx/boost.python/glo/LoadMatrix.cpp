// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/glo/LoadMatrix.h>

using namespace boost::python;

void (*lmf)(gz4d::Matrix<float,4,4> const &) = &gz4d::glo::LoadMatrix;
void (*lmd)(gz4d::Matrix<double,4,4> const &) = &gz4d::glo::LoadMatrix;

BOOST_PYTHON_MODULE(LoadMatrix)
{
    def("LoadMatrix",lmf);
    def("LoadMatrix",lmd);
}
