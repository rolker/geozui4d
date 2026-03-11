// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// TellableInterface.h

class TellableInterface;

#ifndef TELLABLEINTERFACE_H
#define TELLABLEINTERFACE_H

struct Tcl_Interp;
class DottedAddress;

/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for entities can be given textual commands
 *
 *	\author Matt Plumlee (mdp)
 *  \date 1/22/2002
 *  \image html scripts.jpg 
 */
/*---------------------------------------------------------------------------*/
class TellableInterface
{
public:
	virtual ~TellableInterface(){}
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	/// Perform a command without arguments.
	/// An argument-less command may be executed from a string, without needing argc or argv.
	/// @param[in] command The command to be executed.
	/// @param[in] interp The TCL interpreter to use for interacting with tcl
	/// @return TCL_OK if command succeded, TCL_ERROR on failure.
	int performCommand(const char *&command, Tcl_Interp *interp) {	return perform(1, &command, interp); } 
	virtual int list(Tcl_Interp *interp);

	virtual DottedAddress *getAddress() = 0;
};

#endif
