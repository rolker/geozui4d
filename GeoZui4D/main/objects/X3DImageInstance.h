/* Copyright (c) 2008 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef X3DIAMGEINSTANCE_H
#define X3DIMAGEINSTANCE_H

#include "X3DPrototypeInstance.h"
#include "TextureScreen.h"

class X3DImageInstance: public X3DPrototypeInstance
{
public:
    X3DImageInstance();
    virtual void LoadField( gz4d::SFString name, bpt::ptree const &x3d, gz4d::X3D* root);
    virtual void LoadCompleted();
    virtual Object *getObject() {return image;}

protected:
    TextureScreen *image;
};

#endif
