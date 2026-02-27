/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// WindowMediator.cpp
// 
//  This class mediates control over idle events among ToglWindows, sending
//	and every idle event to each registered ToglWindow.  All other events
//	are distributed to the window indicated in the event structure.
//
// Created by mdp 7/25/2000
///////////////////////////////////////////////////////////////////////////////

#include <gz4d/glo.h>
#include "engine/WindowMediator.h"
#include "utilities/DottedAddress.h"

WindowMediator *windowMediator = NULL;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
WindowMediator::WindowMediator(std::string name)
:InputMediator(name)
{
	if (!name.empty())
		getAddress()->reparent(EventMediatorNeighborhood);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int WindowMediator::PushWindow(ToglWindow *client, int instanceInfoOrID)
{
	client->inputMediator.getAddress()->reparent(getAddress());
	return InputMediator::PushInteractor(client, instanceInfoOrID);
}

/*---------------------------------------------------------------------------*/
DottedAddress *WindowMediator::getClientAddress(Interactor *i)
{
	ToglWindow *tw;

	DYNAMIC_CAST(tw, ToglWindow *, i);
	if (tw)
		return tw->inputMediator.getAddress();
	else
		return i->getAddress();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This function distributes events to the appropriate windows.  For idle
 *	events, and events with NULL window pointers, it starts at the top of the
 *	window stack and continues sending the idle event to ToglWindows until it
 *	reaches the end of the list.  Return values are disregarded unless they are
 *	-1 for such events.  All other events are passed to the window indicated
 *	in the info structure, and the return value is either 0 or 1, where any
 *	-1 return values are disregarded and changed to 1's.
 *	
 *
 *	@param	inputType	Type field of the info structure
 *	@param	info		Full event info structure
 *	@return				0=not handled / 1=event was handled
 */
int WindowMediator::HandleInput(unsigned int inputType, InputInfo *info)
{   InteractorList *current = interactors;
    int retval;

	if (info->eventStream == interactorEventMediator)
		return InputMediator::HandleInput(inputType, info);

	//not an idle event, so pass it to proper window.
	if (inputType != NoInput && info->window != NULL)
	{
		retval = info->window->inputMediator.HandleInput(inputType, info);
		return (retval == PASS_EVENT_THROUGH)? PASS_EVENT_THROUGH : SINK_EVENT;
	}

    //We have an idle event or a NULL window, pass to every window
	while (current->next != NULL)
	{
		info->window = (ToglWindow *) current->client; //they are all ToglWindows.
		retval = info->window->inputMediator.HandleInput(inputType, info);
	    if (retval == REMOVE_FROM_STREAM)
		   removeListMember(current);

		current = current->next;
	}

	return SINK_EVENT; //always handled if its an idle event
}
