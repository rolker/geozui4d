// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// BareBonesObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a sample object instantiation, to show the minimum required to
 *	create a GeoZui object.
 *
 *	Created by mdp 4/29/2005
 */
/*---------------------------------------------------------------------------*/
#include "BareBonesObject.h"

//The following line is part of the entry-generating mechanism for the
//catalogue of object types.  If you want to be able to read the object from a
//non-XML file, a string indicating the desired file extension must be provided
//(in this case, "bbo").  If you're just going to instantiate the file from
//code or from XML, you can substitute the empty string (""), but the macro
//call with the class name is required.
REGISTER_OBJECT(BareBonesObject, "bbo");

//Note:  If you create a new object and it is compiled into a library (such as
//object.lib) rather than directly into the executable, it may be necessary to
//add a LINK_OBJECT(BareBonesObject) line to Scene3D.cpp (plus a #include line
//to let Scene3D.cpp know what your object is).

/*---------------------------------------------------------------------------*/
BareBonesObject::BareBonesObject()
{
}

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
 *	parameter informs the object to render itself more or less quickly.  A
 *	degradation of 0 indicates no degradation (high-res, for instance), while
 *  a degradation of 1 indicates maximum degradation.  The Window3D parameter
 *	can be used to determine information such as pixel-size-in-meters, etc.
 */
void BareBonesObject::DrawLowRes(float degradation, Window3D *win)
{
    glDisable(GL_LIGHTING); //This isn't usually desired, but since I'm just drawing lines here, I don't want to have to pay attention to lighting normals.

	glColor3f(0.9f, 0.9f, 0.5f);
	glBegin(GL_LINE_STRIP);
	glVertex3f(-1.0, -1.0, -1.0);
	glVertex3f(-1.0, -1.0, 1.0);
	glVertex3f(-1.0, 1.0, 1.0);
	glVertex3f(-1.0, 1.0, -1.0);
	glVertex3f(1.0, 1.0, -1.0);
	glVertex3f(1.0, -1.0, -1.0);
	glVertex3f(1.0, -1.0, 1.0);
	glVertex3f(1.0, 1.0, 1.0);
	glEnd();

    glEnable(GL_LIGHTING);        
}

