// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
