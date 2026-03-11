// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Animator.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the interface for classes that animate one PosableStateBase to
 *	another, with some potential input from various input streams.
 *	Implementations of this interface are responsible for the following.
 *		* Managing operand and resultant PosableStateBases
 *		* Process input events
 *		* Recognize a "cancel" event
 *
 *	Created by mdp 1/17/2002
 */
/*---------------------------------------------------------------------------*/

#include "engine/Object.h"
#include "engine/PosableStateBase.h"
#include "interactors/Animator.h"
#include "utilities/DottedAddress.h"
#include "widgets/Sv_Standards.h"
#include "engine/gzCommandInterface.h"
#include "engine/InputMediator.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <gz4d/util/Utilities.h>


using namespace std;

#define OPERANDS_LIST_QUANTUM 5
#define REGISTRY_GROWTH_STEP 50

Addressable AnimatorNeighborhoodAnchor;
DottedAddress *AnimatorNeighborhood = new DottedAddress(&AnimatorNeighborhoodAnchor, "Animator");
Animator::RegistrationEntry **Animator::animatorRegistry = NULL;
int Animator::numEntries = 0;
int Animator::maxEntries = 0;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
Animator::Animator(PosableStateBase *resultant, 
				   string name, int maxOperands)
:Interactor(name)
{
	init(resultant, maxOperands);

	if (!name.empty())
	{
		getAddress()->reparent(AnimatorNeighborhood);
	}
}

/*---------------------------------------------------------------------------*/
Animator::Animator(PosableStateBase *resultant, int maxOperands)
:Interactor()
{
	init(resultant, maxOperands);
}

/*---------------------------------------------------------------------------*/
void Animator::init(PosableStateBase *resultant, int maxOperands)
{
	resultantPose = resultant;
	if (resultantPose)
		resultantPose->setContinuousChange(GL_TRUE);

	maxOperandsAllowed = maxOperands;
	if (maxOperands > OPERANDS_LIST_QUANTUM)
		nOperandsAllocated = OPERANDS_LIST_QUANTUM;
	else
		nOperandsAllocated = maxOperands;
	if (nOperandsAllocated)
		operandPoses = new PosableStateBase*[nOperandsAllocated];
	else
		nOperandsAllocated = 0;//NULL;
	nOperands = 0;
	cancelable = GL_FALSE;
	activated = GL_TRUE;
	changeMask = ChangeCurrentMask;
}

/*---------------------------------------------------------------------------*/
Animator::~Animator()
{
	if (resultantPose)
		resultantPose->setContinuousChange(GL_FALSE);

	if (operandPoses)
		delete operandPoses;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Animator::replaceResultant(PosableStateBase *newPose)
{
	if (resultantPose)
		resultantPose->setContinuousChange(GL_FALSE);

	resultantPose = newPose;

	if (resultantPose)
		resultantPose->setContinuousChange(GL_TRUE);
}

/*---------------------------------------------------------------------------*/
/**	Returns the operand at the passed index.  If the index is less than 0 (or
 *	not given) then the index is assumed to be zero.  If the index is greater
 *	than the number of operand slots in the list, NULL is returned.
 */
PosableStateBase *Animator::getOprerand(int index)
{
	if (index < 0)
		index = 0;

	if (index < nOperands)
		return operandPoses[index];
	return NULL;
}

/*---------------------------------------------------------------------------*/
/**	Adds an operand to the end of the list, creating more space if necessary.
 *	If adding a new operand would exceed maxOperandsAllowed, the operand is
 *	added to the first available NULL slot, or if there is not slot available,
 *	the operand is not added and -1 is returned.
 */
long Animator::addOperand(PosableStateBase *pose)
{
	int i;
	PosableStateBase **newList;

	if (nOperands >= 0 && nOperands >= maxOperandsAllowed)
	{
		for (i = 0; i < nOperands; i++)
		{
			if (operandPoses[i] == NULL)
			{
				operandPoses[i] = pose;
				return i;
			}
		}
		return -1;
	}

	if (nOperands + 1 >= nOperandsAllocated)
	{
		newList = new PosableStateBase*[nOperandsAllocated + OPERANDS_LIST_QUANTUM];
		for (i = 0; i < nOperandsAllocated; i++)
			newList[i] = operandPoses[i];
		delete operandPoses;
		operandPoses = newList;
		nOperandsAllocated += OPERANDS_LIST_QUANTUM;
	}
	operandPoses[nOperands] = pose;

	return nOperands++;
}

/*---------------------------------------------------------------------------*/
/**	Removes the passed operand from the operand list as many times as it
 *	appears.  Operands at the end of the list are moved to fill in the holes.
 */
void Animator::removeOperand(PosableStateBase *pose)
{
	int i;

	for (i = 0; i < nOperands; i++)
	{
		if (operandPoses[i] == pose)
			operandPoses[i] = operandPoses[--nOperands]; //replace with last one
	}
}

/*---------------------------------------------------------------------------*/
/**	Replaces the passed oldPose operand with the passed newPose operand, as
 *	often as oldPose appears.
 */
void Animator::replaceOperand(PosableStateBase *oldPose, PosableStateBase *newPose)
{
	int i;

	for (i = 0; i < nOperands; i++)
	{
		if (operandPoses[i] == oldPose)
			operandPoses[i] = newPose;
	}
}

/*---------------------------------------------------------------------------*/
/**	Replaces the operand at the passed indesx with the passed newPose operand.
 *	If an index less than 0 is given (or no index is given) index 0 is
 *	assumed.
 */
void Animator::replaceOperand(PosableStateBase *newPose, int index)
{
	if (index < 0)
		index = 0;

	if (index >= nOperands)
		return;

	operandPoses[index] = newPose;
}

/*---------------------------------------------------------------------------*/
int Animator::IsCancelAction(int inputType, InputInfo *info)
{
	if (cancelable)
		return Interactor::IsCancelAction(inputType, info);
	else
		return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
int Animator::HandleInput(unsigned int inputType, InputInfo *info)
{
	PosableStateBase *pose;

	/*---Handle idle event---*/
	if (inputType == NoInput)
	{
		if (!resultantPose || !activated)
			return PASS_EVENT_THROUGH;

		return animate();
	}
	else if (inputType & EnumeratedAction)
	{
		if (inputType == PosableStateBase::PosableDeletedEvent())
		{
			pose = (PosableStateBase*)info->val.p;
			if (resultantPose == pose)
				replaceResultant(NULL);
			removeOperand(pose);
		}
	}

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
int Animator::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *command, *atArg;
	Addressable *address;
	PosableStateBase *newPose, *firstPose;
	int retval, index, currentArg;
	GLboolean badSyntax = GL_FALSE;

    index = 0;
    
	if (argc >= 1 && !strcmp(argv[0], "activate"))
	{
		if (argc >= 2)
			activate(atoi(argv[1]));
		else
			activate();
		return TCL_OK;
	}
	if (argc >= 1 && !strcmp(argv[0], "animate"))
	{
		animate();
		return TCL_OK;
	}
	if (argc >= 2)
	{
		if (!strcmp(argv[0], "resultant"))
		{
			retval = gzCommandInterface::findPosable(argv[1], address, newPose);
			if (retval != TCL_OK)
				return retval;
			replaceResultant(newPose);
			return TCL_OK;
		}
		if (!strcmp(argv[0], "operand"))
		{
			if (argc >= 3)
			{
				currentArg = 1;
				command = argv[currentArg++];
				atArg = argv[currentArg++];
				if (strcmp(atArg, "at") == 0)
				{
					retval = gzCommandInterface::findPosable(argv[currentArg++],
								address, firstPose);
					index = atoi(argv[currentArg++]);
				}
				else
				{
					retval = gzCommandInterface::findPosable(atArg, address,
								firstPose);
					atArg = NULL;
				}
				if (retval != TCL_OK)
					return retval;
				if (strcmp(command, "add") == 0)
				{
					if (addOperand(firstPose) < 0)
					{
						Tcl_AppendResult(interp, "Maximum number of operands reached.  ",
										 "Use set to replace an existing operand.\n");
						return TCL_ERROR;
					}
				}
				else if (strcmp(command, "remove") == 0)
				{
					if (atArg)
						replaceOperand(NULL, index);
					else
						removeOperand(firstPose);
				}
				else if (strcmp(command, "replace") == 0)
				{
					retval = gzCommandInterface::findPosable(argv[currentArg++],
								address, newPose);
					if (retval != TCL_OK)
						return retval;
					replaceOperand(firstPose, newPose);
				}
			}
			else
			{
				Tcl_AppendResult(interp, "Invalid subcommand.  Valid fomrats:\n",
								 "operand add <pose>\n",
								 "operand remove {at <index> | <old-pose>}\n",
								 "operand replace {at <index> | <old-pose>} <new-pose>\n",
								 NULL);
				return TCL_ERROR;
			}
			return TCL_OK;
		}
		if (!strcmp(argv[0], "effect"))
		{
			//Process the description string
			if (argc < 2)
				badSyntax = GL_TRUE;
			else
			{
				if (!setChangeMask(argv[1]))
					badSyntax = GL_TRUE;
			}
			if (badSyntax)
			{
				Tcl_AppendResult(interp,"Bad or missing effect descriptor string.  Must consist of the following:\n",
										"    x - X position\n",
										"    y - Y position\n",
										"    z - Z position\n",
										"    h - Heading\n",
										"    p - Pitch\n",
										"    r - Roll\n",
										"    s - Scale",
								NULL);
				return TCL_ERROR;
			}
			setChangeMask(changeMask);
			return TCL_OK;
		}
	}
	return Interactor::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Animator::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "resultant operand effect activate animate ", NULL);
	return Interactor::list(interp);
}

/*---------------------------------Class Registration------------------------*/
/*---------------------------------------------------------------------------*/
const Animator::RegistrationEntry *Animator::registerAnimator(
					const char *classname,
					Animator *(*instantiating_fn)())
{
  int i;
  RegistrationEntry *newEntry, **newRegistry;

  //create the new entry item.
  newEntry = new RegistrationEntry;
  newEntry->className = classname;
  newEntry->create = instantiating_fn;

  //expand the registry if needed
  if (numEntries >= maxEntries)
  {
      maxEntries += REGISTRY_GROWTH_STEP;
      newRegistry = new RegistrationEntry*[maxEntries];
      for (i = 0; i < numEntries; i++)
		newRegistry[i] = animatorRegistry[i];
      delete [] animatorRegistry;
      animatorRegistry = newRegistry;
    }

  //add the item.
  animatorRegistry[numEntries++] = newEntry;

  return newEntry;
}

/*---------------------------------------------------------------------------*/
Animator::RegistrationEntry *Animator::getRegistryEntry(const char *classname)
{
	int i;
	RegistrationEntry *currentEntry;

	for (i = 0; i < numEntries; i++)
	{
		currentEntry = animatorRegistry[i];
		if (strcmp(classname, currentEntry->className) == 0)
			return currentEntry;
	}
	return NULL;
}

/*---------------------------------------------------------------------------*/
Animator *Animator::createFromClassname(string classname)
{
	Animator *a;

	RegistrationEntry *entry = getRegistryEntry(classname.c_str());
	if (entry)
	{
		a = entry->create();
		return a;
	}
	return NULL;
}

/*---------------------------------------------------------------------------*/
Animator *Animator::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Addressable *address;
    PosableStateBase *pose;

    boost::optional<std::string> attribute;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.label"))
        publishName(*attribute);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.target"))
    {
        if (gzCommandInterface::findPosable(*attribute, address, pose, GL_FALSE, ObjectNeighborhood) != TCL_OK)
            cerr << "Error finding target named " << *attribute << " for XML animator.\n";
        replaceResultant(pose);
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.operand"))
    {
        if (gzCommandInterface::findPosable(*attribute, address, pose, GL_FALSE, ObjectNeighborhood) != TCL_OK)
            cerr << "Error finding operand named " << *attribute << " for XML animator.\n";
        addOperand(pose);
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.effect"))
    {
        if(!setChangeMask(*attribute))
            cerr << "Error in effect string " << *attribute << " for XML animator (choose any combo from xyzhprs).\n";
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.activate"))
        activate(gz4d::IsTrue(*attribute));
    else
        activate();

    AnimatorNeighborhood->addChild(getAddress());
    globalInputMediator->PushInteractor(this);
    return this;
}

/*---------------------------------------------------------------------------*/
GLboolean Animator::setChangeMask(string effect)
{
	changeMask = ChangeCurrentMask;
	for (string::size_type i = 0; i < effect.size(); i++)
	{
		switch (effect[i])
		{
			case 'x':
				changeMask |= ChangeTranslateXMask;
				break;
			case 'y':
				changeMask |= ChangeTranslateYMask;
				break;
			case 'z':
				changeMask |= ChangeTranslateZMask;
				break;
			case 'h':
				changeMask |= ChangeRotateYawMask;
				break;
			case 'p':
				changeMask |= ChangeRotatePitchMask;
				break;
			case 'r':
				changeMask |= ChangeRotateRollMask;
				break;
			case 's':
				changeMask |= ChangeScaleMask;
				break;
			case ' ': case '\t': case '\n':
				break;
			default:
				return GL_FALSE;
				break;
		}
	}
	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void Animator::activate(GLboolean val)
{
    activated = val;
    if (resultantPose)
        resultantPose->setContinuousChange(val);
}
