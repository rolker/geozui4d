// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
        ///////////////////////////////////////////////////////////////////////////////
        //                                                                           //
        //  File:         PickInteractor.cpp                                         //
        //  Programmer:   Andy Foulks                                                //
        //  Created:      7.12.00                                                    //
        //  $Source$           //
        //                                                                           //
        ///////////////////////////////////////////////////////////////////////////////

#include "engine/Scene3D.h"
#include "engine/ViewManager.h"
#include "interactors/PickInteractor.h"
#include "engine/InputMediator.h"
#include "engine/Window3D.h"
#include "interactors/DoubleClickInteractor.h"

bool PickInteractor::enabled = true;

/* ========================================================================= */
/**
 *  Constructor.
 *      Here we register with the InputMediator, which detects events
 *      for us and passes them on to us. ??
 */
PickInteractor::PickInteractor()
    :Interactor("pickInteractor")
{
}

/* ========================================================================= */
/**
 *  Override the base class version so that mouse events are detected for
 *  picking.
 *
 *  @param      inputType     This is the kind of input that the user
 *                            has given to the system -- it could be
 *                            a key or a mouse press, etc.
 *  @param      inputInfo     This is a pointer to a class that stores
 *                            additional information about the events,
 *                            like the x,y position of the mouse, etc.
 *
 *  @return     PASS_EVENT_THROUGH - Lets the event pass through the stream
 *                                   so that other instances of Interactor
 *                                   objects can also handle it.
 *              SINK_EVENT         - "Sinks" the event so that no one else
 *                                   in the stream knows about it.
 *              REMOVE_FROM_STREAM - Removes this PickInteractor from the
 *                                   stream.  Others can handle this event,
 *                                   but this PickInteractor will not handle
 *                                   any events in the future.
 */
int PickInteractor::HandleInput(unsigned int inputType, InputInfo* inputInfo )
{
    Pickable::PickType type;

    if (!enabled)
        return PASS_EVENT_THROUGH;

    //First, determine what type
    if (inputType & EnumeratedAction)
    {
        if (inputType == CLICK_EVENT || inputType == DOUBLE_CLICK_EVENT)
        {
            inputInfo = ((InputInfo*)inputInfo->val.p);
            if (inputType == CLICK_EVENT)
                type = Pickable::PICK_CLICK_ANY;
            else
                type = Pickable::PICK_DOUBLE_ANY;
        }
        else
            return PASS_EVENT_THROUGH;
    }
    else if (inputType & MouseButtonDown)
        type = Pickable::PICK_PRESS_ANY;
    else
        return PASS_EVENT_THROUGH;

    //Next, determine which button it is
    if (inputInfo->val.i >= 4)
        return PASS_EVENT_THROUGH;
    if (inputInfo->type & MouseButton1)
        type &= Pickable::PICK_BUTTON1;
    else if (inputInfo->type & MouseButton2)
        type &= Pickable::PICK_BUTTON2;
    else if (inputInfo->type & MouseButton3)
        type &= Pickable::PICK_BUTTON3;

        //To get here, we must have a button press, a click, or a double-click.
    Window3D * currentWindow = Window3D::cast(inputInfo->window);
    if (!currentWindow)
        return PASS_EVENT_THROUGH;
    currentView = currentWindow->getViewInfo();

    currentWindow->startPickingRound(type);

    return rePick(type, inputInfo);
}

/* ========================================================================= */
/**
 *  This method performs the pick mechanism again (useful if the pickAction
 *	procedure of an object wants to find what object is picked underneath it).
 *
 *  @param      type     This is the kind of input that the user
 *                            has given to the system -- it could be
 *                            a key or a mouse press, etc.
 *  @param      info     This is a pointer to a class that stores
 *                            additional information about the events,
 *                            like the x,y position of the mouse, etc.
 */
int PickInteractor::rePick(Pickable::PickType type, InputInfo *info)
{
    GLuint selectBuffer[SELECT_BUFFER_SIZE] = { 0 };

    renderAllPickableObjects(type, selectBuffer, info);

    if (processPickedObjects(type, selectBuffer, info) && (info->type & MouseButton1))
        return SINK_EVENT;

    return PASS_EVENT_THROUGH;
}

/* ========================================================================= */
/**
 *  This method iterates through the linked list of all Pickable objects
 *  and calls the renderPickableObject() method, (which every instance
 *  of Pickable implements).  This code is written by the implementer
 *  of the class actually being rendered, so the PickInteracter actually
 *  has no idea what it is drawing.  Before drawing the object, we
 *  give openGL an identifier (unsigned integer) so that openGL knows
 *  how to tell us if this object has been picked.  The identifier
 *  that is used is the pointer to the object itself, cast to an
 *  unsigned integer.  After drawing the object, we discard the name,
 *  move onto the next object, and draw it.
 *
 *	@param      selectBuffer        Buffer to store which objects were picked.
 *	@param	type	Specify mouse button and click type we are looking for.
 *	@param	info	need more info....
    @return true if an object was rendered, false if no object got rendered.
 */
bool PickInteractor::renderAllPickableObjects(Pickable::PickType type, GLuint* selectBuffer, InputInfo *info)
{
    Window3D *win;
    GLdouble hintVal;
    bool drew = false;
    int winLevel;

    if (pickableList.empty())
        return false; //nobody to draw!

    glSelectBuffer( SELECT_BUFFER_SIZE, selectBuffer );
    glRenderMode( GL_SELECT );

    glInitNames();

    hintVal = Scene3D::interactivityHint;
    if (hintVal < 0.5)
        Scene3D::interactivityHint = 0.5;

    win = Window3D::cast(info->window);

    //Temporarily reset the window's transparency level so we pick real things.
	winLevel = win->getTransparencyLevel();
    win->setTransparencyLevel(0);

    for(PickableList::iterator i = pickableList.begin(); i != pickableList.end();i++)
    {
        if(i->first&type)
        {
            setUpMatrices();
                // the inputInfo contains information about the mouse events, 
                // but the x and y values are stored as unions, so it leads
                // to somewhat odd notation.  So info->x.i should be read
                // "the x value in the info object, as an integer".
            setUpPickWindow(info->x.i, info->y.i);
            if(pickableMap[i->second]->pickAsObject())
                setUpRenderingTransformations();
            else
                glMatrixMode( GL_MODELVIEW );

            glPushName(i->second);

            pickableMap[i->second]->renderPickableObject(win);

            glPopName();

            if(pickableMap[i->second]->pickAsObject())
                restoreRenderingTransformations();
            else
                glFlush();
            restoreMatrices();
            drew = true;
        }
    }

    win->setTransparencyLevel(winLevel);
    Scene3D::interactivityHint = hintVal;
    return drew; //we asked someone to draw

}

/* ========================================================================= */
/**
 *  This method sets up new projection and modelview matrices, and clears
 *  the new projection matrix by loading the identity matrix.
 */
void PickInteractor::setUpMatrices()
{
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
}

/* ========================================================================= */
/**
 *  This method sets up a 5x5 pixel pick window around a mouse centered
 *  at the x, y passed in to this function.  The objects will be rendered
 *  into this tiny window -- the ones that actually get drawn (i.e. are
 *  not clipped by openGL) get recorded and are associated with a name
 *  provided before the object is drawn.
 *
 *  @param      x        The x position of the current mouse location.
 *  @param      y        The y position of the current mouse location.
 */
void PickInteractor::setUpPickWindow( int x, int y )
{
    GLint viewport[4];

    glGetIntegerv( GL_VIEWPORT, viewport );

    gluPickMatrix( (GLdouble)x, (GLdouble)viewport[3]-y, PICK_WIDTH, PICK_HEIGHT, viewport );
}

/* ========================================================================= */
/**
 *  This method simply puts openGL into GL_MODELVIEW mode, and
 *  puts a fresh identity matrix on the stack.  Then, what we need
 *  to do is set up for rendering as if we were going through the
 *  normal process of rendering the objects.  To do this, we call
 *  Window3D's PushTransform, which sets up the proper projection
 *  matrix, etc, which gets composed with the pickWindow matrix that
 *  was previously established.  After this step, we are ready to
 *  render the objects that are Pickable.
 */
void PickInteractor::setUpRenderingTransformations()
{
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    currentView->pushView(GL_FALSE);
}

/* ========================================================================= */
/**
 *  This method simply does the "inverse" of setUpRenderingTransformations()
 *  by popping the normal transformations from the stack, popping the
 *  openGL matrix used to render our pickable objects into the pickWindow,
 *  and finally force the execution of all of the openGL commands that
 *  the state machine may be waiting on currently.
 */
void PickInteractor::restoreRenderingTransformations()
{
    currentView->popView();
    glPopMatrix();
    glFlush();
}

/* ========================================================================= */
/**
 *  This method does the "inverse" of setUpMatrices() by restoring the 
 *  projection and modelview matrices to what they were previously, before
 *  we did any pickable rendering.
 */
void PickInteractor::restoreMatrices()
{
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

/* ========================================================================= */
/**
 *  This method searches the selectBuffer to find out if anything was
*  was picked by the user.  The select buffer has the following format:
 *
 *  | 0          1        2        3        4       ...  N+2   |
 *  |----------+--------+--------+--------+--------+---+-------|----------+
 *  |NumofNames|   z1   |   z2   |  name1 |  name2 |...| nameN |NumofNames|...
 *  |----------+--------+--------+--------+--------+---+-------|----------+
 *  | hit #0                                                   | hit #1
 *
 *	@param	pickType	The type of pick?
 *  @param      selectBuffer      The buffer containing all objects picked
 *                                which was filled by openGL during
 *                                previous rendering.
 *	@param      inputInfo         Information about the generating event.
 *  @return     GL_FALSE    -- Nothing was picked
 *              GL_TRUE     -- Something was picked
 */
bool PickInteractor::processPickedObjects(Pickable::PickType pickType, GLuint* selectBuffer, InputInfo *inputInfo)
{
    int newHits = 0, lastPos = 0;
    GLuint names, *ptr, currentMin, previousMin = 0xffffffff;
    GLuint newBuffer[SELECT_BUFFER_SIZE];
    Pickable::PickID previousPickable = 0;

    bool retval = false;
    bool pickBasedOnDepth = true, shouldUpdate;

        // go back into normal rendering mode and get number of hits from openGL
    GLint hits = glRenderMode( GL_RENDER );

    ptr = selectBuffer;

    for( GLint i=0; i<hits; i++ )
    {
        names = *(ptr++); //get the number of names
        currentMin = *(ptr++);// get z1
//        cerr << i << " min: " << currentMin;
        if (currentMin == 0 && *ptr == 0)
            currentMin = 0xffffffff;
        ptr++;// skip z2
        if (currentMin <= previousMin)
            shouldUpdate = true;
        else
            shouldUpdate = false;
        if (previousPickable != *ptr)
        {
            if (!pickBasedOnDepth && previousPickable>0)
            {
                shouldUpdate = true;
                retval = retval || pickableMap[previousPickable]->pickAction(pickType, inputInfo, newBuffer, newHits);
            }
        }
        if (shouldUpdate)
        {
            if (previousMin == currentMin && previousPickable>0)
                retval = retval || pickableMap[previousPickable]->pickAction(pickType, inputInfo, newBuffer, newHits);
            previousPickable = *ptr;
            previousMin = currentMin;
            newHits = 0;
            lastPos = 0;
        }
        ptr++; names--;//skip to first non-pointer name
        if (shouldUpdate)
            newBuffer[lastPos++] = names; //record the number of extra names into the newBuffer
        for (GLuint j = 0; j < names; j++)
        {
            if (shouldUpdate)
                newBuffer[lastPos++] = *(ptr++);
            else
                ptr++;
        }
        newHits++;
    }
    if (previousPickable)
        retval = retval || pickableMap[previousPickable]->pickAction(pickType, inputInfo, newBuffer, newHits);

    return retval;
}

/* ========================================================================= */
/**
 *  This method "registers" a pickable object.  This means that the pickable
 *  object is added to the internal linked list, so that when the
 *  user tries to select an object, we can figure out which one was
 *  picked.  In order for your object to be pickable, you must register
 *  it first -- this is automatically handled by the Pickable constructor
 *  so you should probably never call this function directly.
 *
 *  @param      pickableObject        The new object that you want to 
 *                                    be pickable when the program runs.
 *	@param		type				  The type of click/press to respond to.
 */
void PickInteractor::registerPickableObject(Pickable* pickableObject, Pickable::PickType type)
{
    pickableMap[pickableObject->getPickID()]=pickableObject;
    pickableList.push_back(PickableListEntry(type,pickableObject->getPickID()));
}

/* ========================================================================= */
/**
 *  This method removes all matches to a pickable object from every list if
 *  it is there. If the element is not in the list, nothing happens.
 *
 *  @param      pickableObject      The object to be removed from the list.
 */
void PickInteractor::removePickableObject( Pickable* pickableObject )
{
    for(PickableList::iterator i = pickableList.begin(); i != pickableList.end();)
    {
        if (i->second == pickableObject->getPickID())
            i = pickableList.erase(i);
        else
            i++;
    }
    pickableMap.erase(pickableObject->getPickID());
}

/* ========================================================================= */
/**
 *  This method removes all matches to a pickable object from the appropriate
 *	list, if it is there. If the element is not in the list, nothing happens.
 *
 *  @param      pickableObject      The object to be removed from the list.
 *	@param		type				The type of click/press to ignore.
 */
void PickInteractor::removePickableObject(Pickable* pickableObject, Pickable::PickType type)
{
    for(PickableList::iterator i = pickableList.begin(); i != pickableList.end();)
    {
        if (i->second == pickableObject->getPickID())
        {
            if (i->first | type) // remove flag from potentials
                i->first = i->first & ~type;
            if (i->first == 0) // if all flags are zero, remove from list
                i = pickableList.erase(i);
            else
                i++;
        }
        else
            i++;
    }
}

/* ========================================================================= */
/**
 *  This class uses a "singleton" design pattern for the implementation.
 *  This means that there is only one instance, which is accessable
 *  only through this static (class) method.
 *
 *  @return                  The instance of the PickInteractor.
 */
PickInteractor* PickInteractor::getInstance()
{
    static PickInteractor* pickInteractor = NULL;

    if( pickInteractor == NULL )
        pickInteractor = new PickInteractor();

    return( pickInteractor );
}

/* ========================================================================= */
/**
 *  Destructor.
 */
PickInteractor::~PickInteractor()
{

}
