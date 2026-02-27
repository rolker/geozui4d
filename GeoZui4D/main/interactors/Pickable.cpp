/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:         Pickable.cpp                                               //
//  Programmer:   Andy Foulks                                                //
//  Created:      7.17.00                                                    //
//  $Source$        	     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "interactors/Pickable.h"
#include "interactors/PickInteractor.h"

Pickable::PickID Pickable::nextPickID = 1;

/* ========================================================================= */
/**
 *  Constructor.
 *      When an object of type Pickable is created, we automatically
 *      register it with the PickInteractor.  This ensures that 
 *      when the user picks on the object it will be detected by the
 *      picking system.
 */
Pickable::Pickable(PickType type)
{
    pickID = nextPickID++;
    respondTo(type);
}

/* ========================================================================= */
/**
 *  Destructor.
 *      If this object is about to be removed from memory, then we need
 *      to notify the PickInteractor to remove ourself from the list
 *      of Pickable objects.  If we forgot to do this you would end up
 *      with one of those infamous dangling pointers.
 */
Pickable::~Pickable()
{
    PickInteractor::getInstance()->removePickableObject( this );
}

/* ========================================================================= */
void Pickable::respondTo(PickType type)
{
    PickInteractor::getInstance()->registerPickableObject(this, type);
}

/* ========================================================================= */
void Pickable::ignore(PickType type)
{
    PickInteractor::getInstance()->removePickableObject(this, type);
}
