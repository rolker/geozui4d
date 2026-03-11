// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
        ///////////////////////////////////////////////////////////////////////////////
        //                                                                           //
        //  File:         PickInteractor.h                                           //
        //  Programmer:   Andy Foulks                                                //
        //  Created:      7.12.00                                                    //
        //  $Source$             //
        //                                                                           //
        ///////////////////////////////////////////////////////////////////////////////

#ifndef PICK_INTERACTOR_H
#define PICK_INTERACTOR_H

#include "SDV_types.h"

//#include "utilities/LinkedList.h"
#include "engine/Interactor.h"
#include "engine/InputInfo.h"
#include "interactors/Pickable.h"

#include <list>
#include <utility>
#include <map>

#define PICK_WIDTH 5
#define PICK_HEIGHT 5

class ViewManager;

/**
 *  The PickInteractor class is a class that handles picking events.
 *  In addition to zooming in and out, the user can select some
 *  objects in the virtual world and do things to them.  (One way
 *  way to handle this is to give the user a context sensitive pop
 *  up menu when an object is selected that gives some options
 *  about what the user can do with that object).  Examples include,
 *  possibly, changing the color, looking up information about the
 *  object in a database, loading the object from disk (as is
 *  is the case for the rov textures), or maybe removing the 
 *  object from the virtual world completely.
 *
 *  The way that this class works is that it stores a list of all
 *  the objects in the design that we consider pickable.  Then
 *  whenever a certain mouse button is pressed (either middle
 *  or right buttons, we haven't decided on a convention yet),
 *  the pickable objects are "scanned", and if any were picked
 *  an appropriate action is taken.  
 *
 *  The scanning process involves giving each pickable object an
 *  identifier, then letting openGL know about that identifier.
 *  Identifiers are unsigned integers.  The identifier that we
 *  use is the pointer to the object itself.  This guarantees a 
 *  unique identifier at runtime.  When openGL returns the identifier
 *  of an object that has been selected, since that identifier is a
 *  pointer to the object you can cast it to type (Pickable*) directly.
 *
 *  Next, the object is actually rendered into a tiny pick window, 
 *  using openGL commands to set up the polys, etc (each object
 *  knows how to draw its pickable parts).  OpenGL then remembers 
 *  which ID goes with which rendered object, and when you are done 
 *  drawing all of the objects, openGL returns to you information about
 *  which ones, if any, were in the vicinity of the mouse pointer
 *  while the button was pressed, by giving you back those id's.
 *
 *  This class implements the singleton design pattern.  Therefore
 *  the constructor is private and the single instance is obtained
 *  through the static function getInstance().
 */
class PickInteractor : public Interactor
{
    public:
        ~PickInteractor();
        int HandleInput (unsigned int inputType, InputInfo* inputInfo);
        static PickInteractor* getInstance ();
        static void enable (){enabled = true;};
        static void disable (){enabled = false;};

        void registerPickableObject(Pickable* pickableObject, Pickable::PickType type);
        void removePickableObject(Pickable* pickableObject);
        void removePickableObject(Pickable* pickableObject, Pickable::PickType type);
        int rePick(Pickable::PickType type, InputInfo *info);

    private:
        enum { SELECT_BUFFER_SIZE = 2048 };

        typedef std::map<Pickable::PickID,Pickable*> PickableMap;
        typedef std::pair<Pickable::PickType, Pickable::PickID> PickableListEntry;
        typedef std::list<PickableListEntry> PickableList;

        PickableMap pickableMap;
        PickableList pickableList;

        ViewManager *currentView;
        static bool enabled;

// methods:
        /* constructor: */
        PickInteractor();
        bool processPickedObjects(Pickable::PickType pickType, GLuint* selectBuffer, InputInfo *inputInfo);
        bool renderAllPickableObjects(Pickable::PickType type, GLuint* s, InputInfo *info);
        void setUpPickWindow( int x, int y );
        void setUpRenderingTransformations();
        void restoreRenderingTransformations();
        void setUpMatrices();
        void restoreMatrices();
};

#endif // !PICK_INTERACTOR_H
