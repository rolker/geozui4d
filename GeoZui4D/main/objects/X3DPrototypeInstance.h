/* Copyright (c) 2008 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef X3DPROTOTYPEINSTANCE_H
#define X3DPROTOTYPEINSTANCE_H

#include <gz4d/x3d/X3DPrototypeInstance.h>
class Object;

class X3DPrototypeInstance: public gz4d::X3DPrototypeInstance
{
public:
    X3DPrototypeInstance() {};
    virtual Object *getObject() = 0;
};

#endif
