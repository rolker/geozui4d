/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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
