/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// BareBonesObject.h
/*---------------------------------------------------------------------------*/
/** 
 *	This is a sample object instantiation, to show the minimum required to
 *	create a GeoZui object.
 *
 *	Created by mdp 4/29/2005
 */
/*---------------------------------------------------------------------------*/

#ifndef BAREBONESOBJECT_H
#define BAREBONESOBJECT_H

#include "engine/Object.h"

class BareBonesObject : public Object
{
  public:
    BareBonesObject();
	DERIVE_OBJECT(BareBonesObject); //This is necessary for generating an entry in the automatic catalogue of object types

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
};

#endif
