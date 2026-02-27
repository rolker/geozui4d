/* Copyright (c) 2008 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef X3DFEATUREINSTANCE_H
#define X3DFEATUREINSTANCE_H

#include "X3DPrototypeInstance.h"
#include "Feature.h"

class X3DFeatureInstance: public X3DPrototypeInstance
{
public:
    X3DFeatureInstance();
    virtual void LoadStarted();
    virtual void LoadField( gz4d::SFString name, bpt::ptree const &x3d, gz4d::X3D* root);
    virtual void LoadCompleted();
    virtual Object *getObject() {return feature;}

protected:
    gz4d::MFNode children;
    Feature *feature;
};

#endif
