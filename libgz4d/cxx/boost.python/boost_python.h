// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_BOOST_PYTHON_H
#define GZ4D_BOOST_PYTHON_H

#include <memory>

namespace boost
{

    namespace python
    {
//         template <class T> T* get_pointer(const std::shared_ptr<T> &ptr)
//         {
//             return ptr.get();
//         }
    }
}

#include <boost/python.hpp>

#define GZ4D_BOOST_PYTHON

#endif
