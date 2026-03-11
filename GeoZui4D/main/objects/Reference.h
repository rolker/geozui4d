// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
#ifndef REFERENCE_H
#define REFERENCE_H

#include"engine/Object.h"

class Reference: public Object
{
public:
    Reference();
    DERIVE_OBJECT(Reference);

    void DrawLowRes(float degradation, Window3D *win);
    Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
    int perform(int argc, const char **argv, Tcl_Interp *interp);
private:
    Object * reference;

};


#endif
