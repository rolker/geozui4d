/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PosableStateUser.cpp
/*---------------------------------------------------------------------------*/
/* 
 *	This is the interface for classes that use objects derived form
 *	PosableStateBase, and require notification of changes from these objects.
 *	scale properties.  Implementations of this interface are responsible for
 *	the following.
 *		* Handling pose-change notifications.
 *
 *	Created by mdp 1/11/2001
 */
/*---------------------------------------------------------------------------*/

#include "engine/PosableStateBase.h"
#include "PosableStateUser.h"
#include "widgets/Sv_Standards.h"
#include <string.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PosableStateUser::PosableStateUser(PosableStateBase *initialPosable)
{
	pose = initialPosable;
	numAttributes = 0;
	attributeNames = NULL;
}

/*---------------------------------------------------------------------------*/
PosableStateUser::~PosableStateUser()
{
	if (pose)
		delete pose;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PosableStateBase *PosableStateUser::getPosable()
{
	return pose;
}

/*---------------------------------------------------------------------------*/
void PosableStateUser::setPosable(PosableStateBase *newPosable)
{
	pose = newPosable;
}

/*---------------------------------------------------------------------------*/
DottedAddress *PosableStateUser::getAddress()
{
	return pose? pose->getAddress() : NULL;
}

/*----------------------------Attribute Support------------------------------*/
/*---------------------------------------------------------------------------*/
/**
Returns number of attributes this object has.
\return Number of attributes
 */
int PosableStateUser::getNumAttributes()
{
	return numAttributes;
}

/*---------------------------------------------------------------------------*/
/**
Gets the index of a named attribute.
\param attributeName Name of attribute for which we want the index.
\return Index of the attribute at the passed name.
\retval >=0 of the attribute at the passed name.
\retval -1 Not a valid attribute name.
 
 */
int PosableStateUser::getAttributeIndex(std::string attributeName)
{
	int i;

	for (i = 0; i < numAttributes; i++)
	{
		if (attributeNames[i] == attributeName)
			return i;
	}
	return -1;
}

/*---------------------------------------------------------------------------*/
/**	Returns the attribute name for the passed attribute index, or NULL if it
 *	is not a valid attribute index.
 */
std::string PosableStateUser::getAttributeName(int attributeIndex)
{
	if (attributeIndex < numAttributes && attributeIndex >= 0)
		return attributeNames[attributeIndex];
	else
		return "";
}

/*---------------------------------------------------------------------------*/
/**	Returns through val the attribute value at the passed index if the index is
 *	valid (and reutrns TCL_OK).  Otherwise, returns TCL_ERROR.
 */
int PosableStateUser::getAttribute(SmartValue &val, int attributeIndex)
{
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/**	returns TCL_OK if the attribute value could be set, TCL_ERROR otherwise.
 */
int PosableStateUser::setAttribute(SmartValue &val, int attributeIndex)
{
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateUser::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	int i, num;

	const char *message = argv[0];
	char retval[8192];
	
	if (argc >= 1)
	{
		if (!strcmp(message, "listAttributes"))
		{
			retval[0] = '\0';
			num = getNumAttributes();
			for (i = 0; i < num; i++)
			{
				if (i > 0)
					strcat(retval, "\n");
				strcat(retval, getAttributeName(i).c_str());
			}
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
	}
	return TellableInterface::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int PosableStateUser::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "listAttributes ", NULL);
	return TellableInterface::list(interp);
}


void PosableStateUser::getHtmlInfo(std::stringstream &html, short level)
{
	html << "<h" << level << ">PosableStateUser:</h" << level << ">";
}
