/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// InputTentacle.h
/**
 *	This class acts as a projection of another interactor for intercepting
 *	particular types of events (from a different place in the input stream
 *	than the owner of the tentacle).
 *
 * Created by mdp 2/5/2003
 */
///////////////////////////////////////////////////////////////////////////////

#include "interactors/InputTentacle.h"
#include "utilities/DottedAddress.h"
#include "engine/gzCommandInterface.h"
#include "SDV_types.h"
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
InputTentacle::InputTentacle(std::string name)
:Interactor(name)
{
	owner = NULL;
	inputFilter = NoInput;
	ignoreBeforeSN = 0;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void InputTentacle::setOwner(Interactor *newOwner)
{
	DottedAddress *parentAddress;
	DottedAddress *myAddress;

	owner = newOwner;

	parentAddress = owner->getAddress();
	myAddress = getAddress();
	if (myAddress && parentAddress)
		myAddress->reparent(parentAddress);
}

/*---------------------------------------------------------------------------*/
void InputTentacle::setFilter(unsigned int inputType, GLboolean sendNoInput)
{
	inputFilter = inputType;
	passAlongNoInput = sendNoInput;
}

/*---------------------------------------------------------------------------*/
void InputTentacle::ignoreBefore(long serialNumber)
{
	ignoreBeforeSN = serialNumber;
}

/*---------------------------------------------------------------------------*/
int InputTentacle::HandleInput(unsigned int inputType, InputInfo *info)
{
//	int retval = PASS_EVENT_THROUGH;

	/*---Handle idle event---*/
	if (info->serialNumber <= ignoreBeforeSN)
		return PASS_EVENT_THROUGH;

	if (inputType == NoInput && passAlongNoInput)
		return owner->HandleInput(inputType, info);

	if (inputType & EnumeratedAction)
	{
		if ((inputFilter & EnumeratedAction) &&
		    (inputType & EnumeratedMask) == (inputFilter & EnumeratedMask))
			return owner->HandleInput(inputType, info);
	}
	else if ((inputType & inputFilter) > DataTypeMask)
		return owner->HandleInput(inputType, info);

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
int InputTentacle::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];
	Addressable *address = NULL;
	GLboolean badSyntax = GL_FALSE;
	unsigned int eventVal;

	if (argc >= 1)
	{
		if (!strcmp(message, "owner"))
		{
			if (argc >= 2)
			{
				if (gzCommandInterface::findAddressIn(argv[1], InteractorNeighborhood, address) != TCL_OK)
					return TCL_ERROR;
				DYNAMIC_CAST(owner, Interactor *, address);
				//don't use setOwner, because the caller can move the address around if desired.
				return TCL_OK;
			}
			else
				badSyntax = GL_TRUE;
		}
		if (!strcmp(message, "filter"))
		{
			if (argc >= 3)
			{
				eventVal = gzCommandInterface::convertEvent(argv[2]);
				if (eventVal == Interactor::InvalidInput)
					return TCL_ERROR;
				if (!strcmp(argv[1], "add"))
				{
					if (eventVal == NoInput)
						passAlongNoInput = GL_TRUE;
					else
						inputFilter |= eventVal;
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "remove"))
				{
					if (eventVal == NoInput)
						passAlongNoInput = GL_FALSE;
					else
						inputFilter &= ~eventVal;
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "set"))
				{
					inputFilter = eventVal;
					if (eventVal == NoInput)
						passAlongNoInput = GL_TRUE;
					else
						passAlongNoInput = GL_FALSE;
					return TCL_OK;
				}
				else
					badSyntax = GL_TRUE;
			}
			else
				badSyntax = GL_TRUE;
		}
		if (!strcmp(message, "ignore"))
		{
			if (argc >= 2)
				ignoreBefore(atol(argv[1]));
			else
				badSyntax = GL_TRUE;
			return TCL_OK;
		}
		if (badSyntax)
		{
			Tcl_AppendResult(interp, "usage:\n",
							 "owner <interactor>\n",
							 "filter {set | add | remove} <basic-event>\n",
							 "ignore <serial-number>\n", NULL);
			return TCL_ERROR;
		}
	}
	return Interactor::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int InputTentacle::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "owner filter ignore ", NULL);
	return Interactor::list(interp);
}
