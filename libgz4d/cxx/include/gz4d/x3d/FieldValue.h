// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3D_FIELDVALUE_H
#define GZ4D_X3D_FIELDVALUE_H

#include <gz4d/gz4d.h>
#include <string>
#include <gz4d/time.h>
#include <gz4d/Matrix.h>

namespace gz4d
{
    namespace x3d
    {
        class GZ4DEXPORT FieldValue
        {
            public:
                virtual ~FieldValue();
                virtual void operator=(std::string const &) =0;
        };


        std::istream &operator>>(std::istream &str, Time &t);

        std::istream &operator>>(std::istream &str, Matrix<float,3,3> &m);
        std::istream &operator>>(std::istream &str, Matrix<float,4,4> &m);

        std::ostream &operator<<(std::ostream &str, Matrix<float,3,3> &m);
        std::ostream &operator<<(std::ostream &str, Matrix<float,4,4> &m);

    }
}

#endif
