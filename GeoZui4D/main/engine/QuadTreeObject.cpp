/* Copyright (c) 2009 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// QuadTreeObject.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for Objects that can be
 *  used in an Object QuadTree as the target of a QuadTreeTile.
 *
 *  This class is responsible for the following.
 *	  * Keep track of which QuadTreeTile it belongs to
 *	  * Be able to make a "shallow" copy of itself offset by
 *      specified x- and y-offsets
 *    * Be able to read new "deep" contents from a given file
 *      without obtaining new properties from the file (for
 *      example, skipping over the header in a GUTM to get
 *      to the depth values only).
 *    * Be able to generate a half-resolution copy of oneself
 *      from the children of the associated QuadTreeTile.
 *
 *	Created by Matt Plumlee (mdp) 9/19/2008
 */

#include "QuadTreeObject.h"
#include "engine/PosableStateBase.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
QuadTreeObject::QuadTreeObject(GLboolean allocatePosable, int pickingTypes)
: Object(allocatePosable, pickingTypes)
{
    quadtreeStatus = Quadtree_Not;
    childrenDrawn = NULL;
    drewLastTime = true;
}

/*---------------------------------------------------------------------------*/
void QuadTreeObject::actAsTemplate()
{
    quadtreeStatus |= Quadtree_Template;
//    if (lockToParent)
//    {
//        delete lockToParent;
//        lockToParent = NULL;
//    }
}

/*---------------------------------------------------------------------------*/
void QuadTreeObject::copyObjectComponents(QuadTreeObject *other, float xOffset, float yOffset)
{
    gz4d::Point<double> myGeoRef = pose->getPosition();
    other->SetPosition(myGeoRef[X] + xOffset, myGeoRef[Y] + yOffset, myGeoRef[Z]);

    other->spatialReference = spatialReference;
    other->projectionRequired = projectionRequired;
    other->reversedCoordinates = reversedCoordinates;
    other->parent = parent;
    other->lockToParent = lockToParent;
    other->requiresFullReload = requiresFullReload;

    other->showAsWireframe = showAsWireframe;
    other->degradationFlag = degradationFlag;
    other->transparencyLevel = transparencyLevel;
    other->scalePriority = scalePriority;
    other->selectedFlag = selectedFlag;
    other->isEditableFlag = isEditableFlag;
    other->usingInternalHandleFlag = usingInternalHandleFlag;
    other->canUseInternalHandleFlag = canUseInternalHandleFlag;
    other->allowBaseExternalEdit = allowBaseExternalEdit;
    other->respondToPress = respondToPress;
    other->respondToPress2 = respondToPress2;
    other->respondToClick = respondToClick;
    if (respondToClick)
    {
        other->respondTo(PICK_CLICK1);
        other->ignore(PICK_PRESS1);
    }
    else
        other->ignore(PICK_CLICK1);

    if (respondToPress)
    {
        other->respondTo(PICK_PRESS1);
        other->ignore(PICK_CLICK1);
    }
    else
        other->ignore(PICK_PRESS1);

    if (respondToPress2 == GL_TRUE)
        other->respondTo(PICK_PRESS2);
    else
        other->ignore(PICK_PRESS2);

    other->customtextureptr = customtextureptr;
    other->useTextures = useTextures;
    other->clutMode = clutMode;
    other->customtextureid = customtextureid;
    other->onlyWin = onlyWin;

    other->actAsPattern();
}

/*---------------------------------------------------------------------------*/
bool QuadTreeObject::setXMLattribute(std::string const &name, std::string const &value, std::string const &path)
{
    fs::path fn;

    if (name == "label")
    {
        SetLabel(value);
        return true;
    }
    if (name == "filename")
    {
        if(value[0] == '/')
            fn = value;
        else
            fn = fs::path(path)/value;
        SetFilename(fn.string());
        return true;
    }
    return false;
}
