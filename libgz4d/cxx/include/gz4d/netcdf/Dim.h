// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_NETCDF_DIM_H
#define GZ4D_NETCDF_DIM_H

#include <gz4d/gz4d.h>
#include <gz4d/netcdf/netcdf.h>

namespace gz4d
{
    namespace netcdf
    {
        class File;
        class Var;
        class Dim
        {
            friend class File;
            friend class Var;

            netCDF::NcDim dim;

            Dim(netCDF::NcDim d):dim(d){}

            bool IsValid()
            {
                return !dim.isNull();
            }
            void CheckValid(){if (!IsValid()) throw Exception("Invalid Dimension");}

            public:
                Dim(){}
                Dim(Dim const &d):dim(d.dim){}

                operator bool(){return IsValid();}

                std::string Name() {CheckValid(); return dim.getName();}
                long Size() {CheckValid(); return dim.getSize();}
                bool IsUnlimited() {CheckValid(); return dim.isUnlimited();}
                void Rename(std::string const &name) {CheckValid(); dim.rename(name);}
                void Sync() {CheckValid(); /* no-op in netcdf-c++4 */}
        };
    }
}

#endif
