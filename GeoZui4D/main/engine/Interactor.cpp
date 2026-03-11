// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*! \file 
    \brief parent class for input handling (Interactor and EventType)
    
    This is the parent class for the input handling of events over which it is
    given authority.

    Created by mdp 3/2/2000

    Added interface inlines to detect some of the events.  Will add more
    functions if time permits. --raf 6/12/00
*/



#include "engine/Interactor.h"
#include "engine/ToglWindow.h"
#include "engine/InputMediator.h"
#include "utilities/DottedAddress.h"
#include <string.h>

#define EVENT_TYPE_QUANTUM 50

EventType **Interactor::globalEvents = NULL;
int Interactor::numGlobalEventsAllocated = 0;
unsigned int Interactor::nextEnumeratedAction = Interactor::NextEnumAction;
const EventType *Interactor::interactorDestroyedInputType = Interactor::newEventType("interactorDestroyed", Interactor::DataAsAddress);
Addressable InteractorNeighborhoodAnchor, EventNeighborhoodAnchor, UserHandlerNeighborhoodAnchor;
DottedAddress *InteractorNeighborhood = NULL;
DottedAddress *EventNeighborhood = NULL;
DottedAddress *UserHandlerNeighborhood = NULL;
static Interactor defaultInteractorInstance;
Interactor *defaultInteractor = &defaultInteractorInstance;

//////////////////////////////////////////////////////////////////////
// Interactor Methods
//////////////////////////////////////////////////////////////////////
Interactor::Interactor(int retval)
{
	defaultReturn = retval;
}

/*---------------------------------------------------------------------------*/
Interactor::Interactor(std::string name, int retval)
:Addressable(name)
{
	defaultReturn = retval;

	if (!name.empty())
	{
		if (!InteractorNeighborhood)
		{
			InteractorNeighborhood = new DottedAddress(&InteractorNeighborhoodAnchor, "Interactor");
			UserHandlerNeighborhood = new DottedAddress(&UserHandlerNeighborhoodAnchor, "Handler");
			InteractorNeighborhood->addChild(UserHandlerNeighborhood);
		}
		else if (InteractorNeighborhoodAnchor.getAddress() == NULL)
		{
			InteractorNeighborhoodAnchor.moveTo(InteractorNeighborhood);
			UserHandlerNeighborhoodAnchor.moveTo(UserHandlerNeighborhood);
		}
		InteractorNeighborhood->addChild(getAddress());
	}
}

/*---------------------------------------------------------------------------*/
#if 0
void Interactor::getHtmlInfo(stringstream &html)
{
  Addressable::getHtmlInfo(html);
  html << "<h2>Interactor:</h2>";
}
#endif

/*---------------------------------------------------------------------------*/
Interactor::~Interactor()
{
//	if (interactorEventMediator)
//		interactorEventMediator->Trigger(interactorDestroyedInputType->value, this);
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** This function sinks inputs for use as a sentinel for InputMediators.
 *
 *  In the base class, always do nothing with the event and pretend its
 *  been handled, so that the InputMediator can pass the event onto other
 *  objects that really care about it.
 */
int Interactor::HandleInput(unsigned int inputType, InputInfo *info)
{
    return defaultReturn;
}

/* ========================================================================= */
/**
 *  This function is here for future flexibility in the Cancel action.
 */
int Interactor::IsCancelAction(int inputType, InputInfo *info)
{   if (inputType & KeyStroke)
    {   if (info->val.i == KeyEsc)
	        return 1;
	}
	return 0;
}

/* ========================================================================= */
/**
 *  This function allocates a unique InputType to the caller.
 */
EventType *Interactor::newEventType(const char *name, unsigned int flags)
{
	int i, index;
	static int runBefore = 0;
	EventType *retval = new EventType(name), **temp;
	retval->value = nextEnumeratedAction | flags;
	index = (nextEnumeratedAction & EnumeratedMask) >> EnumBits;

	//expand the directory if necessary
	if (numGlobalEventsAllocated <= index)
	{
		temp = new EventType*[numGlobalEventsAllocated + EVENT_TYPE_QUANTUM];
		if (runBefore)
		{
			for (i = 0; i < numGlobalEventsAllocated; i++)
				temp[i] = globalEvents[i];
			delete globalEvents;
		}
		else
		{
			EventType *initial;
			int localIndex;
			initial = temp[0] = new EventType("idle");
			initial->value = NoInput;
			localIndex = (BeginIdleAction & EnumeratedMask) >> EnumBits;
			initial = temp[localIndex] = new EventType("beginIdle");
			initial->value = BeginIdleAction & EnumeratedMask;
			localIndex = (EndIdleAction & EnumeratedMask) >> EnumBits;
			initial = temp[localIndex] = new EventType("endIdle");
			initial->value = EndIdleAction & EnumeratedMask;
			localIndex = (BreakAction & EnumeratedMask) >> EnumBits;
			initial = temp[localIndex] = new EventType("break");
			initial->value = BreakAction & EnumeratedMask;
			runBefore = 1;
		}
		globalEvents = temp;
		numGlobalEventsAllocated += EVENT_TYPE_QUANTUM;
	}
	globalEvents[index] = retval;

	nextEnumeratedAction += EnumIncrement;
	return retval;
}

/* ========================================================================= */
/**
 *  This function returns the string name of the passed inputType.  Child
 *	classes should redefine this function if they define their own special
 *	class of events.
 */
std::string Interactor::nameOfEventType(unsigned int inputType)
{
	unsigned int type;

	if (inputType == NoInput)
		return "idle";
	if (inputType & EnumeratedAction)
	{
		type = (inputType & EnumeratedMask) >> EnumBits;
		if (type < nextEnumeratedAction)
			return globalEvents[type]->getAddress()->getAddress();
		return "unknown";
	}
	if (inputType & KeyDown)
		return "keyDown";
	if (inputType & KeyUp)
		return "keyUp";
	if (inputType & MouseButtons)
	{
		/* Since MouseDouble only indicates that it's a down or up
		   after a click, don't name the event specially...
		if (inputType & MouseDouble)
		{
			if (inputType & MouseButton1)
				return "mouse1Double";
			if (inputType & MouseButton2)
				return "mouse2Double";
			if (inputType & MouseButton3)
				return "mouse3Double";
		}
		*/
		if (inputType & MouseButtonDown)
		{
			if (inputType & MouseButton1)
				return "mouse1Down";
			if (inputType & MouseButton2)
				return "mouse2Down";
			if (inputType & MouseButton3)
				return "mouse3Down";
		}
		if (inputType & MouseButtonUp)
		{
			if (inputType & MouseButton1)
				return "mouse1Up";
			if (inputType & MouseButton2)
				return "mouse2Up";
			if (inputType & MouseButton3)
				return "mouse3Up";
		}
	}
	if (inputType & MouseEnterMask)
		return "mouseEnter";
	if (inputType & MouseLeaveMask)
		return "mouseLeave";
	if (inputType & MouseMotion) //Do last since entering & leaving are special motions
		return "mouseMotion";
	return "unknown";
}

/* ========================================================================= */
/**
 *  This function returns the type value of the passed event name.  Child
 *	classes should redefine this function if they define their own special
 *	class of events.
 */
unsigned int Interactor::valueOfSpecificEventType(std::string eventType)
{
	char c;
	unsigned int retval;

	c = eventType[0];
	if (c == 'i')
	{
		if (eventType == "idle")
			return NoInput;
	}
	else if (c == 'k')
	{
		if (eventType == "keyUp")
			return KeyUp;
		if (eventType == "keyDown")
			return KeyDown;
	}
	else if (c == 'm')
	{
		if (eventType.substr(0,5) == "mouse")
		{
			c = eventType[5];
			if (c == 'M' && eventType == "mouseMotion")
				return MouseMotion;
			else if (c == 'E' && eventType == "mouseEnter")
				return MouseEnter;
			else if (c == 'L' && eventType == "mouseLeave")
				return MouseLeave;
			else if (c >= '1' && c <= '3')
			{
				retval = MouseButtonBase << (c - '0');
				if (eventType.substr(6) == "Double")
					return MouseDouble | retval;
				if (eventType.substr(6) == "Down")
					return MouseButtonDown | retval;
				if (eventType.substr(6) == "Up")
					return MouseButtonUp | retval;
			}
		}
	}
	return InvalidInput;
}

/* ========================================================================= */
/**
 *  Has the left mouse button been pressed?
 *
 *  @param     inputType    Keyboard event or mouse click, etc...
 *  @return                 true / false.
 */
bool Interactor::isMouseButton1Pressed( int inputType )
{
    return( (inputType & MouseButton1) && (inputType & MouseButtonDown) );
}

/* ========================================================================= */
/**
 *  Has the left mouse button been released?
 *
 *  @param     inputType    Keyboard event or mouse click, etc...
 *  @return                 true / false.
 */
bool Interactor::isMouseButton1Released( int inputType )
{
    return( (inputType & MouseButton1) && (inputType & MouseButtonUp) );
}

/* ========================================================================= */
/**
 *  Has the middle mouse button been pressed?
 *
 *  @param     inputType    Keyboard event or mouse click, etc...
 *  @return                 true / false.
 */
bool Interactor::isMouseButton2Pressed( int inputType )
{
    return( (inputType & MouseButton2) && (inputType & MouseButtonDown) );
}

/* ========================================================================= */
/**
 *  Has the middle mouse button been released?
 *
 *  @param     inputType    Keyboard event or mouse click, etc...
 *  @return                 true / false.
 */
bool Interactor::isMouseButton2Released( int inputType )
{
    return( (inputType & MouseButton2) && (inputType & MouseButtonUp) );
}

/* ========================================================================= */
/**
 *  Has the right mouse button been pressed?
 *
 *  @param     inputType    Keyboard event or mouse click, etc...
 *  @return                 true / false.
 */
bool Interactor::isMouseButton3Pressed( int inputType )
{
    return( (inputType & MouseButton3) && (inputType & MouseButtonDown) );
}

/* ========================================================================= */
/**
 *  Has the right mouse button been released?
 *
 *  @param     inputType    Keyboard event or mouse click, etc...
 *  @return                 true / false.
 */
bool Interactor::isMouseButton3Released( int inputType )
{
    return( (inputType & MouseButton3) && (inputType & MouseButtonUp) );
}


//////////////////////////////////////////////////////////////////////
// EventType Methods
//////////////////////////////////////////////////////////////////////

EventType::EventType(std::string name)
: Addressable(name)
{
  permanentEvent = true;//GL_TRUE;
	if (!name.empty())
	{
		if (!EventNeighborhood)
			EventNeighborhood = new DottedAddress(&EventNeighborhoodAnchor, "Event");
		EventNeighborhood->addChild(getAddress());
	}
}
