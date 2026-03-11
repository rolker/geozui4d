// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
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
