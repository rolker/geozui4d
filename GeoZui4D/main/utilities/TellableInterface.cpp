// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// TellableInterface.h
/*---------------------------------------------------------------------------*/
/* 
 *	This is the base class for entities can be given textual commands
 *
 *	Created by mdp 1/22/2002
 */
/*---------------------------------------------------------------------------*/
#include "utilities/TellableInterface.h"
#include "utilities/DottedAddress.h"
#include <tcl.h>
#include <string.h>

/// Executes a command originating from tcl.
/// @param argc   count of arguments in command.
/// @param argv   array of strings representing the command and arguments.
/// @param interp Tcl interpreter from which command originated.
/// @return  TCL_OK if command succeded, TCL_ERROR on failure.
int TellableInterface::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	char retval[1024];
	int newArgc, currentArg, iretval;
	const char **newArgv;

	if (argc < 1)
	{
		Tcl_AppendResult(interp, "Empty command sent to ",
						 getAddress()->getAddress().c_str(), NULL);
		return TCL_ERROR;
	}
	if (!strcmp(argv[0], "list"))
	{
		Tcl_ResetResult(interp);
		return list(interp);
	}
	else if (!strcmp(argv[0], "getName"))
	{
		sprintf(retval, "%s", getAddress()->getAddress().c_str());
		Tcl_SetResult(interp, retval, TCL_VOLATILE);
		return TCL_OK;
	}
	else if (!strcmp(argv[0], ":"))
	{
		currentArg = 0;
		newArgc = 0;
		newArgv = &argv[currentArg + 1];
		while (currentArg < argc)
		{
			currentArg++;
			newArgc = 0;
			newArgv = &argv[currentArg];
			while (currentArg < argc && (strcmp(argv[currentArg], "&") != 0))
			{
				currentArg++;
				newArgc++;
			}
			if (currentArg < argc)
			{
				iretval = perform(newArgc, newArgv, interp);
				if (iretval != TCL_OK)
					return iretval;
			}
		}
		return perform(newArgc, newArgv, interp);
	}
	Tcl_AppendResult(interp, "Unrecognized command sent to ",
					 getAddress()->getAddress().c_str(), ": ", argv[0],
					 "\nUse 'list' to list valid commands.", NULL);
	return TCL_ERROR;
}

/// Append the list of commands supported by this class.
/// @param interp Tcl interpreter to use.
/// @return TCL_OK
int TellableInterface::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "getName list :<cmd>&<cmd>...", NULL);
	return TCL_OK;
}
