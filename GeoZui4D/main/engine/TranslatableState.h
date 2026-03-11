// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// TranslatableState.h
/*---------------------------------------------------------------------------*/


class TranslatableState;

#ifndef TRANSLATABLE_STATE_H
#define TRANSLATABLE_STATE_H


#include "engine/PosableStateBase.h"
/** 
 *	This is the implementation for classes that have position, but not
 *	necessarily orientation, and scale properties.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/13/2000
 */
/*---------------------------------------------------------------------------*/
class TranslatableState : public virtual PosableStateBase
{
    public:
        /*---Constructors/Destructors---*/
        TranslatableState(PosableStateUser *initialUser = NULL, std::string name = "");
        TranslatableState(PosableStateBase *toBeReplaced, GLboolean userCall = GL_TRUE);
        virtual ~TranslatableState();
    
        /*---Accessors---*/
        //Use the changeMask arguments to get at min, max, and preferred data
        virtual int supportMask(int changeMask = ChangeAll);
    
        /*---GL Transformations---*/
        virtual void applyTransforms(ViewManager * view = NULL);

    protected:
        virtual PosableStateInfo *pointerForMask(long changeMask);
    
        gz4d::Point<double> _position;
        Point3d _homePosition;
};

#endif
