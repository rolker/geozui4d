// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// InputMediator.cpp
/** 
 *  This class mediates control over input signals among interactors.
 *
 *  Created by mdp 3/2/2000
 */
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "engine/InputMediator.h"
//#include "engine/Joystick.h"
#include "utilities/DottedAddress.h"
#include "engine/gzCommandInterface.h"

Addressable EventMediatorNeighborhoodAnchor;
DottedAddress *EventMediatorNeighborhood = new DottedAddress(&EventMediatorNeighborhoodAnchor, "EventMediator");

static InputMediator interactorMediator("interactor"), globalMediator("internal"), highLevelMediator("user");
//static Joystick joystickMediator("joystick");
InputMediator *globalInputMediator = &globalMediator;
InputMediator *highLevelEventMediator = &highLevelMediator;
InputMediator *interactorEventMediator = &interactorMediator;
//Joystick *joystickEventMediator = &joystickMediator;

static Interactor passThroughSentinel(InputMediator::PASS_EVENT_THROUGH);

/*---------------------------------------------------------------------------*/
InputMediator::InputMediator(int retval)
{
	init(retval);
}

/*---------------------------------------------------------------------------*/
InputMediator::InputMediator(std::string name, int retval)
:Interactor(name, retval)
{
	init(retval);

	if (!name.empty())
	{
		getAddress()->reparent(EventMediatorNeighborhood);
	}
}

/*---------------------------------------------------------------------------*/
void InputMediator::init(int retval)
{
	isPermanent = GL_TRUE;

	//create the list and initialize with a default Interactor;
	if (retval == SINK_EVENT)
		interactors = new InteractorList(defaultInteractor);
	else
		interactors = new InteractorList(&passThroughSentinel);

	priorityMediator = NULL;
	if (this != &interactorMediator)
		interactorMediator.PushInteractor(this);
}

/*---------------------------------------------------------------------------*/
InputMediator::~InputMediator()
{   InteractorList *current, *next;

	if (priorityMediator)
		delete priorityMediator;

    //destroy the list(stack) members on the way down.
	next = interactors;
	while (next)
	{   current = next;
	    next = current->next;
	    delete current;
	}

	interactors = NULL;
	if (this == interactorEventMediator)
		interactorEventMediator = NULL;

	if (interactorEventMediator)
		interactorEventMediator->RemoveInteractor(this);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
InputMediator *InputMediator::getPriorityMediator()
{
	if (!priorityMediator)
	{
		priorityMediator = new InputMediator("priority", PASS_EVENT_THROUGH);
		priorityMediator->getAddress()->reparent(getAddress());
	}

	return priorityMediator;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int InputMediator::PushInteractor(Interactor *client, int instanceInfoOrID)
{   InteractorList *newItem;

    //create the new list item.
    newItem = new InteractorList(client, instanceInfoOrID);
	if (!newItem)
	    return 0; //failed to allocate!

    //add to the top of the list.
    newItem->next = interactors;
	interactors->prev = newItem;
	interactors = newItem;
	return 1;
}

/*---------------------------------------------------------------------------*/
int InputMediator::PopInteractor(Interactor *client, int instanceInfoOrID)
{   InteractorList *dyingItem;

    //Make the checks, if valid data was passed in.
    dyingItem = interactors;
	if (dyingItem->next == NULL ||  //can't delete sentinal!
	    (client && (dyingItem->client != client || //clients don't match
		            (instanceInfoOrID != INPUT_NON_ID &&
					 dyingItem->instanceInfo != instanceInfoOrID)))) //instance info doesn't match
	    return 0;

    //If we got here, delete the dyingItem and return success
	interactors = dyingItem->next;
	interactors->prev = NULL;
	delete dyingItem;

	return 1;
}

/*---------------------------------------------------------------------------*/
int InputMediator::BringInteractorToTop(Interactor *client, int instanceInfoOrID)
{   InteractorList *currentItem, *prevItem, *nextItem;

    //First, find the item.
    currentItem = findInteractor(client, instanceInfoOrID);
	if (!currentItem)
	    return 0;

	//We've got it, now check to see if we really need to do anything.
	prevItem = currentItem->prev;
	nextItem = currentItem->next;

    if (nextItem == NULL) //can't remove sentinal
	    return 0;

	if (prevItem == NULL) //already in the right place
	    return 1;

	//Ok, all set to make the move.
	prevItem->next = nextItem; //link upper to lower
	nextItem->prev = prevItem; //link lower to upper
	interactors->prev = currentItem; //link old top to new top
	currentItem->prev = NULL;
	currentItem->next = interactors; //link new top to old top
	interactors = currentItem; //make new top the active top

    return 1;
}

/*---------------------------------------------------------------------------*/
int InputMediator::RemoveInteractor(Interactor *client, int instanceInfoOrID)
{   InteractorList *currentItem;

    //First, find the item.
    currentItem = findInteractor(client, instanceInfoOrID);
	if (!currentItem)
	    return 0;

	return removeListMember(currentItem);
}

/*---------------------------------------------------------------------------*/
int InputMediator::removeListMember(InteractorList *currentItem,
									GLboolean notifyMember)
{   InteractorList *prevItem, *nextItem;

	//Check to see if we really need to do anything.
	prevItem = currentItem->prev;
	nextItem = currentItem->next;

    if (nextItem == NULL) //can't remove sentinal
	    return 0;

	//Ok, all set to make the changes
	nextItem->prev = prevItem; //link lower to upper
	if (prevItem)
	    prevItem->next = nextItem; //link upper to lower
	else
	    interactors = nextItem; //make new top the active top

	//Inform the item that it is being removed
	if (notifyMember)
		currentItem->client->RemovalNotify(this);
	//now do the deletion
	delete currentItem;

	return 1;
}

/*---------------------------------------------------------------------------*/
InputMediator::InteractorList *InputMediator::findInteractor(Interactor *i, int instanceInfoOrID)
{	InteractorList *currentItem;

    if (!i)
        return NULL;

    //iterate from front of list, looking for the interactor
	for (currentItem = interactors; currentItem; currentItem = currentItem->next)
	{   if (currentItem->client == i) //potential match
		{   if (instanceInfoOrID == INPUT_NON_ID || instanceInfoOrID == currentItem->instanceInfo)
		        return currentItem; //we have a match!
		}
	}

	//We searched the whole list, and found nothing.
	return NULL;
}

/*---------------------------------------------------------------------------*/
/**	This funciton starts at the top of the list(stack) and continues
 *	sending events to interactors until one of them handles it.  There is
 *	no need for finding the end of the list, since the defaultInteractor is
 *	at the base, and will always return one (sentinels that do not return
 *	1 are not currently supported).
 *
 *	@param	inputType	Type field of the info structure
 *	@param	info		Full event info structure
 *	@return				1=event handled
 */
int InputMediator::HandleInput(unsigned int inputType, InputInfo *info)
{   InteractorList *current = interactors;
    int retval = PASS_EVENT_THROUGH;

	if (info->eventStream == interactorEventMediator)
	{
		if (inputType == interactorDestroyedInputType->value)
		{
			current = findInteractor((Interactor *)info->val.p);
			if (current)
				removeListMember(current, GL_FALSE);
			if (this != interactorEventMediator)
				return PASS_EVENT_THROUGH;
		}
	}

	if (priorityMediator)
	{
		retval = priorityMediator->HandleInput(inputType, info);
		inputType = info->type;
	}

	current = interactors;
	while (retval == PASS_EVENT_THROUGH)
	{   
        retval = current->client->HandleInput(inputType, info);

		if (retval == SINK_EVENT)
			retval = retval;

	    if (retval == REMOVE_FROM_STREAM) // remove the interactor if desired
		{   removeListMember(current);
		    retval = SINK_EVENT;
		}
		else if (!(current = current->next))
			break;
	}

	return retval;
}

/*---------------------------------------------------------------------------*/
/**	This funciton creates a temporary InputInfo and propogates the virtual
 *	resulting event through HandleInput.
 *
 *	@param	eventType	Type of event to generate.
 *	@param	data		Optional pointer to data for the val field.
 *	@param	modifier	Not sure...
 */
void InputMediator::Trigger(unsigned int eventType, void *data, void *modifier)
{
	InputInfo *info = new InputInfo(eventType);

	if (eventType & DataTypeMask)
	{
		info->val.p = data;
		info->modifier.p = modifier;
	}
	info->eventStream = this;

	HandleInput(eventType, info);
	delete info;
}

/*--------------------------Scripting Support--------------------------------*/
/*---------------------------------------------------------------------------*/
int InputMediator::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	Addressable *address;
	Interactor *interactor;
	enum {Invalid, Push, Pop, MoveToTop, Remove};
	int retval, command = Invalid, firstItem = GL_TRUE;
	char list[2048];
	DottedAddress *clientAddress;
	InteractorList *current;
	void *data = NULL, *modifier = NULL;
	unsigned int eventType;

	if (argc >= 1 && !strcmp(argv[0], "listOrder"))
	{
		list[0] = '\0';
		for (current = interactors; current; current = current->next)
		{
			if (firstItem)
				firstItem = GL_FALSE;
			else
				strcat(list, " ");

			clientAddress = getClientAddress(current->client);

			if (clientAddress)
				strcat(list, clientAddress->getAddress().c_str());
			else
				strcat(list, "{}");
		}
		Tcl_SetResult(interp, list, TCL_VOLATILE);
		return TCL_OK;
	}
	if (argc >= 2)
	{
		if (!strcmp(argv[0], "trigger"))
		{
			eventType = gzCommandInterface::convertEvent(argv[1], this);
			if (argc >= 3)
			{
				if (argc >= 4)
					retval = packArgument(data, argv[2], argv[3], interp);
				else
					retval = packArgument(data, argv[2], NULL, interp); //get the error reported
				if (retval != TCL_OK)
					return retval;
				if ((eventType & DataTypeMask) == 0)
				{
					switch (argv[2][1])
					{
						case 'i': case 'c':
							eventType |= DataAsInt;
							break;
						/*
						case 'd':
							eventType |= DataAsDouble;
							break;
						*/
						case 'l':
							eventType |= DataAsLong;
							break;
						case 'a':
							eventType |= DataAsAddress;
							break;
						case 's':
							eventType |= DataAsString;
							break;
						case 'p':
							eventType |= DataAsPointer;
							break;
					}
				}
			}
			if (argc >= 5)
			{
				if (argc >= 6)
					retval = packArgument(modifier, argv[4], argv[5], interp);
				else
					retval = packArgument(modifier, argv[4], NULL, interp); //get the error reported
				if (retval != TCL_OK)
					return retval;
			}
			Trigger(eventType, data, modifier);
			return TCL_OK;
		}

		if (!strcmp(argv[0], "push"))
			command = Push;
		else if (!strcmp(argv[0], "pop"))
			command = Pop;
		else if (!strcmp(argv[0], "remove"))
			command = Remove;
		else if (!strcmp(argv[0], "moveToTop"))
			command = MoveToTop;

		if (command != Invalid)
		{
			retval = gzCommandInterface::findAddressIn(argv[1], UserHandlerNeighborhood, address, InteractorNeighborhood);
			if (retval != TCL_OK)
				return retval;

			DYNAMIC_CAST(interactor, Interactor *, address);
			if (interactor)
			{
				switch (command)
				{
					case Push:
						retval = PushInteractor(interactor);
						break;
					case Pop:
						retval = PopInteractor(interactor);
						break;
					case MoveToTop:
						retval = BringInteractorToTop(interactor);
						break;
					case Remove:
						retval = RemoveInteractor(interactor);
						break;
				}
				if (!retval)
				{
					Tcl_AppendResult(interp, "Unable to ", argv[0], " ", argv[1], "!", NULL);
					return TCL_ERROR;
				}
			}
			else
			{
				Tcl_AppendResult(interp, "Invalid interactor: ", argv[1], NULL);
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}
	return Interactor::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int InputMediator::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "push pop moveToTop remove trigger listOrder ", NULL);
	return Interactor::list(interp);
}

/*---------------------------------------------------------------------------*/
DottedAddress *InputMediator::getClientAddress(Interactor *i)
{
	return i->getAddress();
}

/*---------------------------------------------------------------------------*/
/**	This function converts the value to a void* according to the passed flag.
 *	If there is an error, it is reported through Tcl, and TCL_ERROR is
 *	returned.  Otherwise, TCL_OK is returned and the passed void reference
 *	is filled with the converted value.
 */
int InputMediator::packArgument(void *&result, const char *flag, const char *value, Tcl_Interp *interp)
{
	Addressable *address;
	int retval, badSyntax = GL_FALSE;

	if (value == NULL)
	{
		Tcl_AppendResult(interp, "Missing flag or value", NULL);
		return TCL_ERROR;
	}
	else if (*flag == '-')
	{
		switch (flag[1])
		{
			case 'i':
				result = (void*)atoi(value);
				break;
			case 'c':
				result = (void*)((unsigned int)(value[0]));
				break;
			case 'l':
				result = (void*)atol(value);
				break;
			/*
			case 'd':
				result = (void*)atof(value);
				break;
			*/
			case 'a':
				retval = gzCommandInterface::findAddressIn(value, rootAddress, address);
				if (retval != TCL_OK)
					return retval;
				result = (void*)address;
				break;
			case 's':
				result = (void*)value;
				break;
			case 'p':
				if (*value == '@')
					sscanf(&value[1], "%x", (unsigned int *)result); //skip the '@'
				else
					sscanf(value, "%x", (unsigned int *)result);
				break;
			default:
				badSyntax = GL_TRUE;
				break;
		}
	}
	if (badSyntax)
	{
		Tcl_AppendResult(interp, "Bad flag.  Must be -i, -l, -a, -s, or -p", NULL);
		return TCL_ERROR;
	}

	return TCL_OK;
}
