// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////



#ifndef SIMPLEOBJECT_H
#define SIMPLEOBJECT_H

class Window3D;

#include "SDV_types.h"
/** This class is a base for simple drawable items to be added to a list, but
 * don't need the baggage of a full-blown object. An object composed of
 * various components that normally don't make sense outside its collection
 * could be a SimpleObjectCollection, with the components being SimpleObjects. 
 */
class SimpleObject
{
public:
	SimpleObject();
	virtual ~SimpleObject() {};
	virtual void Draw(float degradation, Window3D *win) = 0;
	BBox getBBox(){return bbox;};
protected:
	BBox bbox;
};


#endif
