// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:         Pickable.h                                                 //                                                   //
//  $Source$          	 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef PICKABLE_H
#define PICKABLE_H

#include "SDV_types.h"

class InputInfo;
class Window3D;
/**
 *  This class represents any object in the visualizer that we want
 *  the user to be able to "pick" on by clicking the right mouse button.
 *  \par Use:
 *  To make your object Pickable, do the following things:
 *
 *  \li    (1)  Inherit this class (even if it results in multiple inheritance)
 *  \li    (2)  Implement the "renderPickableObject()"  method inherited from
 *           this interface.  In here you will draw ONLY those object
 *           that you want the user to be able to click on with the mouse.
 *           If you are not particular about it, and your class happens 
 *           to be a child class of "Object", then you could probably
 *           just call "DrawLowRes" and be done with it.  If, however,
 *           you have certain parts of your object that you don't want
 *           the user to be able to click on, then you would explicitly
 *           exclude those objects from the "renderPickableObject()" method.
 *           BE SURE TO TRANSLATE BY YOUR LOCAL OFFSET BEFORE RENDERING.
 *  \li    (3)  Implement the "pickAction()" method inherited from this
 *           interface.  This method will be called whenever your object
 *           has been selected by the user.  The code you write will
 *           be the response to that selection.
 *
 *  Your new Pickable object will automatically "register" itself
 *  with the PickInteractor object (it is handled by the base constructor).
 *  This class is called an "interface class", meaning that it only contains
 *  methods, but no members.  This means that inheriting
 *  from this class incurs no overhead at all (except for the extra
 *  constructor call at initialization) -- so don't worry about
 *  multiple inheritance here -- it won't cause any problems.
 * 
 *  \author Andy Foulks                                               
 *  \date 7/17/00 
 */
class Pickable
{
    public:
        enum {
            PICK_BUTTON1       = 0x00F,
            PICK_BUTTON2       = 0x0F0,
            PICK_BUTTON3       = 0xF00,

            PICK_CLICK_ANY     = 0x111,
            PICK_PRESS_ANY     = 0x222,
            PICK_DOUBLE_ANY    = 0x444,

            PICK_CLICK1        = 0x001,
            PICK_PRESS1        = 0x002,
            PICK_DOUBLE1       = 0x004,

            PICK_CLICK2        = 0x010,
            PICK_PRESS2        = 0x020,
            PICK_DOUBLE2       = 0x040,

            PICK_CLICK3        = 0x100,
            PICK_PRESS3        = 0x200,
            PICK_DOUBLE3       = 0x400,
        };

        typedef GLuint PickID;
        typedef unsigned short PickType;

        Pickable(PickType type = PICK_CLICK1);
        virtual ~Pickable();
        virtual int pickAction(PickType pickType, InputInfo *inputInfo, GLuint *names, int hits) = 0;

        virtual void renderPickableObject(Window3D *inWindow) = 0;
        virtual bool pickAsObject() {return true;}
        void respondTo(PickType type);
        void ignore(PickType type);
        PickID getPickID() {return pickID;};
    private:
        PickID pickID;
        static PickID nextPickID;
};

#endif // !PICKABLE_H
