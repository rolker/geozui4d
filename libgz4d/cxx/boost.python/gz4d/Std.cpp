// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <string>

#include <gz4d/gz4d.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Std)
{
    class_<std::vector<std::string> >("VectorString")
            .def(vector_indexing_suite<std::vector<std::string> >())
            ;

    class_<std::vector<uint32_t> >("VectorUInt32")
            .def(vector_indexing_suite<std::vector<uint32_t> >())
            ;

}
