// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NETCDF_NETCDF_H
#define GZ4D_NETCDF_NETCDF_H

//gz4d_cmake depends netcdf

#include <gz4d/gz4d.h>
#include <gz4d/Exception.h>
#include <ncFile.h>
#include <ncDim.h>
#include <ncVar.h>
#include <ncGroupAtt.h>
#include <ncVarAtt.h>
#include <ncType.h>
#include <ncException.h>
#include <memory>

namespace gz4d
{
    namespace netcdf
    {
        class Exception: public gz4d::Exception
        {
            public:
                Exception(std::string msg):gz4d::Exception(msg){}
        };
    }
}

#endif
