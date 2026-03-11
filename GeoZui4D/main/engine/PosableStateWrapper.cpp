// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// PosableStateWrapper.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the interface for classes that act as proxies for accessing
 *	position, orientation, and scale properties of data that is not already
 *	modeled in a PosableStateBase.  Implementations of this interface are
 *	responsible for the following.  Default behaviors for most of these are provided.
 *		* Allowing for modification and retrieval of pose information
 *		* Applying proper transformation for display (although this is
 *		  unlikely to be used).
 *		* Respond to queries about preferred poses, focus, and context
 *		* Provide hooks that allow other objects to respond to changes
 *		  in this PosableStateBase's pose
 *		* Allow for highlighting this PosableStateBase.
 *	This class actually implements the following responsibilities:
 *		* Keep track of the index into the source data.
 *		* Provide a pose changed notification to user.
 *		* Retain a counter for being in flux (continuously changing)
 *
 *	Created by mdp 2/12/2002
 */
/*---------------------------------------------------------------------------*/
#include "engine/PosableStateWrapper.h"
#include "engine/PosableWrapperSupport.h"
#include "engine/gzCommandInterface.h"
#include "PosableStateUser.h"
#include "engine/SessionRecorder.h"
#include "utilities/DottedAddress.h"
#include "engine/InputMediator.h"
#include <stdlib.h>
#include <string.h>
#include <typeinfo>

static char indexString[16] = "index0000000000";
const EventType *PosableStateWrapper::wrapperPoseChangedInputType =
			Interactor::newEventType("wrapperPoseChanged", Interactor::DataAsAddress);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PosableStateWrapper::PosableStateWrapper(PosableStateUser *initialUser, std::string name)
: PosableStateBase(initialUser, name)
{
	sourceList = NULL;
	support = NULL;
	originator = GL_FALSE;
	indecies = new SmartValue[1];
	numDimensions = 1;
	indexingAttribute = -1;
	associativeIndexHigh = associativeIndexLow = -1;
	holdIndexNotify = 0;
	settingLocally = GL_FALSE;
	//infoUpToDate = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
PosableStateWrapper::PosableStateWrapper(PosableStateBase *toBeReplaced)
:PosableStateBase(toBeReplaced)
{
	sourceList = NULL;
	support = NULL;
	originator = GL_FALSE;
	indecies = new SmartValue[1];
	numDimensions = 1;
	indexingAttribute = -1;
	associativeIndexHigh = associativeIndexLow = -1;
	holdIndexNotify = 0;
	//infoUpToDate = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
PosableStateWrapper::~PosableStateWrapper()
{
	if (sourceList)
		sourceList->removeWrapper(this);
	if (indecies)
		delete [] indecies;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int PosableStateWrapper::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	int retval, dimension = 0, currentArg;
	GLboolean badSyntax = GL_FALSE, flag;
	Addressable *address;
	PosableStateBase *target;
	PosableWrapperSupport *support = NULL;
	enum {AttachTo, DetachFrom, Other, Set, Get} command = Other;
	char c, result[64];
	const char *value = NULL, *inType = NULL;

	if (argc >= 1)
	{
		if (!strcmp(argv[0], "index"))
		{
			if (argc < 2)
				badSyntax = GL_TRUE;
			else //try to extract arguments
			{
				c = *argv[1];
				if (c >= '0' && c <= '9')
				{
					dimension = atoi(argv[1]);
					currentArg = 2;
				}
				else
					currentArg = 1;
				if (dimension < 0 || dimension >= numDimensions)
				{
					Tcl_AppendResult(interp, "Invalid index.  Use \"index dimensions\" to get or set the current limit.", NULL);
					return TCL_ERROR;
				}

				if (argc > currentArg)
				{
					if (!strcmp(argv[currentArg], "set")) //collect set arguments
					{
						command = Set;
						currentArg++;
						if (argc <= currentArg)
							badSyntax = GL_TRUE;
						else
						{
							value = argv[currentArg++];
							if (argc > currentArg)
							{
								if (!strcmp(argv[currentArg++], "as"))
								{
									if (argc > currentArg)
										inType = argv[currentArg];
									else
										badSyntax = GL_TRUE;
								}
								else
									badSyntax = GL_TRUE;
							}
						}
					}
					else if (!strcmp(argv[currentArg], "get")) //collect get arguments
					{
						command = Get;
						currentArg++;
						if (argc > currentArg)
						{
							if (!strcmp(argv[currentArg], "type"))
							{
								Tcl_SetResult(interp, (char *)indecies[dimension].typeName(), TCL_VOLATILE);
								return TCL_OK;
							}
						}
					}
					else if (!strcmp(argv[1], "dimensions")) //perform dimensions...
					{
						if (argc <= 2) //return the # of dimensions
						{
							sprintf(result, "%d", numDimensions);
							Tcl_SetResult(interp, result, TCL_VOLATILE);
							return TCL_OK;
						}
						else //set the # of dimensions
						{
							dimension = atoi(argv[2]);
							if (dimension < 1)
							{
								Tcl_AppendResult(interp, "Number of dimensions must be at least 1.\n",
												 NULL);
								return TCL_ERROR;
							}
							setNumDimensions(dimension);
							return TCL_OK;
						}
					}
					else
						badSyntax = GL_TRUE;
				}
				else
					badSyntax = GL_TRUE;
			}
			if (badSyntax || command <= Other)
			{
				Tcl_AppendResult(interp, "Incorrect usage.  Should be one of:\n",
								 "index [<dimension>] set <value> [as <type>]\n",
								 "index [<dimension>] get [type]\n",
								 "index dimensions [<numDimensions>]\n",
								 NULL);
				return TCL_ERROR;
			}
			//perform get or set command.
			if (command == Set)
			{
				if (!inType)
					inType = indecies[dimension].typeName();
				if (!strcmp(inType, "String"))
					setIndex(value, dimension);
				else if (!strcmp(inType, "Integer"))
					setIndex(atoi(value), dimension);
				else if (!strcmp(inType, "Float"))
					setIndex((float)atof(value), dimension);
				else //give error
				{
					Tcl_AppendResult(interp, "Unsupported index type.  Should be one of:\n",
									 "String\n",
									 "Float\n",
									 "Integer\n",
									 NULL);
					return TCL_ERROR;
				}
			}
			else // (command == Get)
			{
				Tcl_SetResult(interp, (char *)getStringIndex(dimension), TCL_VOLATILE);
				return TCL_OK;
			}
			return TCL_OK;
		}
		if (!strcmp(argv[0], "insert"))
		{
			flag = GL_TRUE;
			currentArg = 1;
			if (argc >= 2)
			{
				if (!strcmp(argv[currentArg], "after"))
				{
					flag = GL_FALSE;
					currentArg++;
				}
			}
			if (argc > currentArg)
			{
				c = *argv[currentArg];
				if (c >= '0' && c <= '9')
					dimension = atoi(argv[currentArg]);
				else
					badSyntax = GL_TRUE;
			}
			if (badSyntax)
			{
				Tcl_AppendResult(interp, "usage: insert [after] [<dimension>]\n", NULL);
				return TCL_ERROR;
			}
			return insertValue(flag, dimension);
		}
		if (!strcmp(argv[0], "delete"))
		{
			if (argc >= 2)
			{
				c = *argv[1];
				if (c >= '0' && c <= '9')
					dimension = atoi(argv[1]);
				else
					badSyntax = GL_TRUE;
			}
			if (badSyntax)
			{
				Tcl_AppendResult(interp, "usage: delete [<dimension>]\n", NULL);
				return TCL_ERROR;
			}
			return deleteValue(dimension);
		}
		if (!strcmp(argv[0], "freeze"))
		{
			delayIndexNotification();
			return TCL_OK;
		}
		if (!strcmp(argv[0], "thaw"))
		{
			if (argv[1])
				resumeIndexNotification(atoi(argv[1]));
			else
				resumeIndexNotification();
			return TCL_OK;
		}
		if (!strcmp(argv[0], "attachTo"))
			command = AttachTo;
		if (!strcmp(argv[0], "detachFrom"))
			command = DetachFrom;
		if (command < Other)
		{
			if (argc < 2)
			{
				Tcl_AppendResult(interp, "usage: ", argv[0], " <posable>", NULL);
				return TCL_ERROR;
			}
			retval = gzCommandInterface::findPosable(argv[1], address, target);
			if (!target)
				return TCL_ERROR;
			DYNAMIC_CAST(support, PosableWrapperSupport *, target->getUser());
			if (!support)
			{
				Tcl_AppendResult(interp, argv[1], " does not support wrapper attachment/detachment.", NULL);
				return TCL_ERROR;
			}

			if (command == AttachTo)
				attachTo(support);
			else //if (command == DetachFrom)
				detach();
			return TCL_OK;
		}
		if (strcmp(argv[0], "objectType") == 0)
		{
                    std::string typeName;
			if (user)
			{
				GET_CLASS_NAME(user, typeName);
			}
			else
			{
				GET_CLASS_NAME(this, typeName);
			}
			Tcl_SetResult(interp, (char *)typeName.c_str(), TCL_VOLATILE);
			return TCL_OK;
		}
	}
	return PosableStateBase::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "attachTo detachFrom index insert delete freeze thaw objectType ", NULL);
	return PosableStateBase::list(interp);
}

/*---------------------------------------------------------------------------*/
void PosableStateWrapper::NotifyPose(long changeMask)
{
	if (!accumulatingChanges && sourceList && !(changeMask & ChangeIndex))
	{
		if (support)
			support->wrapperChangedValue(this, changeMask);
		if (changeMask)
		{
			originator = GL_TRUE;
			sourceList->notify(changeMask); //this will eventually trigger our own update
			originator = GL_FALSE;
		}
	}
	else
		PosableStateBase::NotifyPose(changeMask);
}

/*---------------------------------------------------------------------------*/
/**	Updates any currently held information, (in the future, may just set a
 *	flag indicating that such updating will be required before the next fetch).
 *	We call the PosableStateBase's NotifyPose from here to inform any attached
 *	relationships about our changes (we don't call the child version of
 *	NotifyPose).
 */
void PosableStateWrapper::update(long changeMask)
{
	//infoUpToDate = GL_FALSE;
	if (!originator && support && changeMask != ChangeNone)
	{
		AccumulateNotifications();
		if (!sourceList)
			support->attachWrapper(this);
		support->updateWrapper(this, changeMask);
		//flush changes, but only call base class's NotifyPose.
		supressRecordingRequest++;
		accumulatingChanges = GL_FALSE;
		PosableStateBase::NotifyPose(accumulatedChange);
		accumulatedChange = ChangeNone;
		ResumeSingularNotification();
		supressRecordingRequest--;
	}
	else
		PosableStateBase::NotifyPose(changeMask);
	if (!user)
		highLevelEventMediator->Trigger(wrapperPoseChangedInputType->value, (Addressable *)this, (void *)changeMask);
}

/*---------------------------------------------------------------------------*/
//void PosableStateWrapper::applyTransforms()
//{
//}

/*---------------------------------------------------------------------------*/
void PosableStateWrapper::replace(PosableStateBase *toBeReplaced)
{
	PosableStateWrapper *toBeDiscarded;
	int numDims, *dimensions, i;
	const void **indecies;
	SmartValue::IndexType *types;

	DYNAMIC_CAST(toBeDiscarded, PosableStateWrapper *, toBeReplaced);
	if (toBeDiscarded)
	{
		//set list first so that updates get done
		setList(toBeDiscarded->getList());
		sourceList->wrapper = this;

		//collect index information
		numDims = toBeDiscarded->getNumDimensions();
		indecies = new const void *[numDims];
		dimensions = new int[numDims + 1];
		types = new SmartValue::IndexType[numDims];
		for (i = 0; i < numDims; i++)
			dimensions[i] = i;
		dimensions[i] = -1;
		toBeDiscarded->getIndecies(indecies, dimensions, types);
		toBeDiscarded->setIndecies(indecies, dimensions, types);
	}
	PosableStateBase::replace(toBeReplaced);
}

/*---------------------------------------------------------------------------*/
void PosableStateWrapper::attachTo(PosableWrapperSupport *newSupport)
{
	if (support == newSupport)
		return;

	if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
	{
		char command[1024];
		if (newSupport)
			sprintf(command, "gz tellObject %s attachTo %s", getAddress()->getAddress().c_str(), newSupport->getPosable()->getAddress()->getAddress().c_str());
		else
			sprintf(command, "gz tellObject %s detachFrom %s", getAddress()->getAddress().c_str(), support->getPosable()->getAddress()->getAddress().c_str());
		sessionRecorder->recordCommand(command);
	}

	detach();

	support = newSupport;
	if (newSupport)
		newSupport->attachWrapper(this);
	NotifyIndexChange();
	//infoUpToDate = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void PosableStateWrapper::detach()
{
	if (support)
		support->detachWrapper(this);
	support = NULL;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
SmartValue::IndexType PosableStateWrapper::getIndexType(int dim)
{
	if (dim >= 0 && dim < numDimensions)
		return indecies[dim].type;
	else
		return SmartValue::Invalid;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::getIntegerIndex(int dim)
{
	if (dim >= 0 && dim < numDimensions)
		return indecies[dim].asInteger();

	return 0;
}

/*---------------------------------------------------------------------------*/
float PosableStateWrapper::getFloatIndex(int dim)
{
	if (dim >= 0 && dim < numDimensions)
		return indecies[dim].asFloat();

	return 0.0;
}

/*---------------------------------------------------------------------------*/
const char *PosableStateWrapper::getStringIndex(int dim)
{
	if (dim >= 0 && dim < numDimensions)
		return indecies[dim].asString();

	return NULL;
}

/*---------------------------------------------------------------------------*/
const void *PosableStateWrapper::getPointerIndex(int dim)
{
	if (dim >= 0 && dim < numDimensions)
		return indecies[dim].asPointer();

	return NULL;
}

/*---------------------------------------------------------------------------*/
void PosableStateWrapper::getIndecies(const void **values, int *dimensions, SmartValue::IndexType *types)
{
	int i;
	SmartValue *currentIndex;

	if (dimensions)
	{
		while (dimensions && *dimensions >= 0)
		{
			if (*dimensions < numDimensions)
			{
				currentIndex = &(indecies[*dimensions++]);
				*values++ = currentIndex->value.p;
				*types++ = currentIndex->type;
			}
			else
			{
				dimensions++;
				*values++ = NULL;
				*types++ = SmartValue::Invalid;
			}
		}
		return;
	}

	for (i = 0; i < numDimensions; i++)
	{
		values[i] = indecies[i].value.p;
		if (types)
			types[i] = indecies[i].type;
	}
}

/*---------------------------------------------------------------------------*/
/**	For wrapper attributes, wrapper indices come first, then support attributes,
 *	then support-posable attributes, and finally any user attributes.
 */
int PosableStateWrapper::getNumAttributes()
{
	if (support)
		return numDimensions
			   + support->getNumAttributes(this)
			   + support->getPosable()->getNumAttributes()
			   + PosableStateBase::getNumAttributes();
	else
		return numDimensions + PosableStateBase::getNumAttributes();
}

/*---------------------------------------------------------------------------*/
/**	For wrapper attributes, wrapper indices come first, then support attributes,
 *	then support-posable attributes, and finally any user attributes.
 *	If there are ever any duplicates between any of these, and the default order
 *	is not desired, a prepended '*' character will designate support attributes,
 *	a prepended '#' character will designate support-posable attributes,
 *	and a prepended '^' character will designate user attributes.
 */
int PosableStateWrapper::getAttributeIndex(std::string attributeName)
{
	int i, retval;
	char designator = attributeName[0];
	int numSupportAttributes = 0, numSupportAndPosableAttributes = 0;

	if (attributeName.substr(0,5) == "index")
	{
		i = atoi(attributeName.substr(5).c_str());
		if (i < numDimensions)
			return i;
	}
	if (support && designator != '^')
	{
		numSupportAttributes = support->getNumAttributes(this);
		numSupportAndPosableAttributes = support->getPosable()->getNumAttributes() + numSupportAttributes;
		if (designator != '#')
		{
			if (designator == '*')
				retval = support->getAttributeIndex(this, &(attributeName[1]));
			else
				retval = support->getAttributeIndex(this, attributeName);
			if (retval >= 0)
				return retval + numDimensions;
		}
		if (designator == '#')
			retval = support->getPosable()->getAttributeIndex(&(attributeName[1]));
		else
			retval = support->getPosable()->getAttributeIndex(attributeName);
		if (retval >= 0)
			return retval + numDimensions + numSupportAttributes;
	}
	if (designator == '^')
		retval = PosableStateBase::getAttributeIndex(&(attributeName[1]));
	else
		retval = PosableStateBase::getAttributeIndex(attributeName);
	if (retval >= 0)
		return retval + numDimensions + numSupportAndPosableAttributes;
	else
		return -1;
}

/*---------------------------------------------------------------------------*/
/**	For wrapper attributes, wrapper indices come first, then support attributes,
 *	then support-posable attributes, and finally any user attributes.  Special
 *	prepended characters do not appear in names returned.  The return value of
 *	getAttributeName is not stable, so it is not thread-safe.  The caller should
 *	copy the result if they are going to make another call to getAttributeName
 *	before using the result.  This is known to be true for "index##" names;
 *	support and user attributes may or may not be stable.
 */
std::string PosableStateWrapper::getAttributeName(int attributeIndex)
{
	int numSupportAttributes = 0, numSupportAndPosableAttributes = 0;

	if (attributeIndex < numDimensions)
	{
		sprintf(&(indexString[5]), "%d", attributeIndex);
		return indexString;
	}
	if (support)
	{
		numSupportAttributes = support->getNumAttributes(this);
		numSupportAndPosableAttributes = support->getPosable()->getNumAttributes() + numSupportAttributes;
		if (attributeIndex < numDimensions + numSupportAttributes)
			return support->getAttributeName(this, attributeIndex - numDimensions);
		if (attributeIndex < numDimensions + numSupportAndPosableAttributes)
			return support->getPosable()->getAttributeName(attributeIndex - numDimensions - numSupportAttributes);
	}
	return PosableStateBase::getAttributeName(attributeIndex - numDimensions - numSupportAndPosableAttributes);
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::getAttribute(SmartValue &val, int attributeIndex)
{
	int numSupportAttributes = 0, numSupportAndPosableAttributes = 0;

	if (attributeIndex < numDimensions)
	{
		val = indecies[attributeIndex];
		val.isConst = GL_TRUE;
		return TCL_OK;
	}
	if (support)
	{
		numSupportAttributes = support->getNumAttributes(this);
		numSupportAndPosableAttributes = support->getPosable()->getNumAttributes() + numSupportAttributes;
		if (attributeIndex < numDimensions + numSupportAttributes)
			return support->getAttribute(this, val, attributeIndex - numDimensions);
		if (attributeIndex < numDimensions + numSupportAndPosableAttributes)
			return support->getPosable()->getAttribute(val, attributeIndex - numDimensions - numSupportAttributes);
	}
	return PosableStateBase::getAttribute(val, attributeIndex - numDimensions - numSupportAndPosableAttributes);
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::setAttribute(SmartValue &val, int attributeIndex)
{
	int numSupportAttributes = 0, numSupportAndPosableAttributes = 0;

	if (attributeIndex < numDimensions)
	{
		indecies[attributeIndex] = val;
		if (val.isConst && val.type == val.String)
			indecies[attributeIndex].set(strdup(val.value.s));

		NotifyIndexChange();
		return TCL_OK;
	}
	if (support)
	{
		numSupportAttributes = support->getNumAttributes(this);
		numSupportAndPosableAttributes = support->getPosable()->getNumAttributes() + numSupportAttributes;
		if (attributeIndex < numDimensions + numSupportAttributes)
			return support->setAttribute(this, val, attributeIndex - numDimensions);
		if (attributeIndex < numDimensions + numSupportAndPosableAttributes)
			return support->getPosable()->setAttribute(val, attributeIndex - numDimensions - numSupportAttributes);
	}
	return PosableStateBase::setAttribute(val, attributeIndex - numDimensions - numSupportAndPosableAttributes);
}

/*---------------------------------------------------------------------------*/
/**	This function sets the indexingAttribute for later calls to
 *	indexByAttributeValue, returning GL_TRUE when successful.  The
 *	only ways to fail is if this wrapper is attached to a support that
 *	doesn't allow the passed attributeIndex to be used (for whatever reason),
 *	or if the attributeIndex is not within the range of indices valid for
 *	support attributes.
 */
GLboolean PosableStateWrapper::setIndexingAttribute(int attributeIndex)
{
	if (attributeIndex >= 0)
	{
		attributeIndex -= numDimensions;
		if (attributeIndex < 0)
			return GL_FALSE;
	}

	if (indexingAttribute >= 0 && support)
	{
		if (attributeIndex == -1)
		{
			support->releaseAssociativeAttribute(indexingAttribute);
			indexingAttribute = -1;
			return GL_TRUE;
		}
		if (attributeIndex >= support->getNumAttributes(this))
			return GL_FALSE;
		if (attributeIndex == indexingAttribute) //already set
			return GL_TRUE;
		support->releaseAssociativeAttribute(indexingAttribute);
		indexingAttribute = -1;
	}
	if (support)
	{
		if (support->requestAssociativeAttribute(attributeIndex))
		{
			indexingAttribute = attributeIndex;
			return GL_TRUE;
		}
		else
			return GL_FALSE;
	}
	indexingAttribute = attributeIndex;
	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/**	This function causes the wrapped posable to update its time bounds based
    on the current indexingAttribute
 */
void PosableStateWrapper::resetTimeBoundsOnAttribute(double offset, double scale)
{
	if (support && indexingAttribute >= 0)
		support->resetTimeBoundsOnAttribute(this, indexingAttribute, offset, scale);
}

/*---------------------------------------------------------------------------*/
/**	This function allows the index for the wrapper to be set according to the
 *	passed attribute value, where the attribute was either set previously
 *	by a call to setIndexingAttibute, or the support already has an indexing
 *	attribute.  If the attribute set previously does not match the support's
 *	attribute, or if this wrapper is not attached to any support, this function
 *	returns GL_FALSE.
 */
GLboolean PosableStateWrapper::indexByAttributeValue(float value)
{
	if (!support)
		return GL_FALSE;

	if (indexingAttribute >= 0 && indexingAttribute != support->getAssociativeAttribute())
		return GL_FALSE;

	return support->setWrapperIndexFromAttribute(this, value);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void PosableStateWrapper::setNumDimensions(int newCount)
{
	int smaller, i;
	SmartValue *newindecies;

	if (newCount == numDimensions)
		return;

	if (newCount < numDimensions)
		smaller = newCount;
	else
		smaller = numDimensions;

	newindecies = new SmartValue[newCount];
	for (i = 0; i < smaller; i++)
	{
		newindecies[i] = indecies[i];
		indecies[i].type = SmartValue::Invalid; //in case it was a string, we effectively moved it.
	}

	delete [] indecies;
	indecies = newindecies;
	numDimensions = newCount;
	if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
	{
		char command[1024];
		sprintf(command, "gz tellObject %s index dimensions %d", getAddress()->getAddress().c_str(), newCount);
		sessionRecorder->recordCommand(command);
	}

}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::setIndecies(const void **values, int *dimensions, SmartValue::IndexType *types) //multi-dimensional set
{
	int dim;
	SmartValue::IndexType type;

	while (dimensions && *dimensions >= 0)
	{
		dim = *dimensions++;
		if (dim < numDimensions)
		{
			type = indecies[dim].type;
			if (type == SmartValue::String)
				delete indecies[dim].value.s;
			if (types)
				type = *types++;
			indecies[dim].type = type;
			if (type == SmartValue::String)
				indecies[dim].value.s = strdup((const char *)(*values++));
			else
				indecies[dim].value.p = *values++;
		}	
	}
	NotifyIndexChange();
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::setIndex(int val, int dim)
{
	if (dim >= 0 && dim < numDimensions)
	{
		indecies[dim].set(val);
		if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
		{
			char command[1024];
			sprintf(command, "gz tellObject %s index %d set %d as %s", getAddress()->getAddress().c_str(), dim, val, SmartValue::typeName(SmartValue::Integer));
			sessionRecorder->recordCommand(command);
		}
		NotifyIndexChange();
		return TCL_OK;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::setIndex(float val, int dim)
{
	if (dim >= 0 && dim < numDimensions)
	{
		indecies[dim].set(val);
		if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
		{
			char command[1024];
			sprintf(command, "gz tellObject %s index %d set %f as %s", getAddress()->getAddress().c_str(), dim, val, SmartValue::typeName(SmartValue::Float));
			sessionRecorder->recordCommand(command);
		}
		NotifyIndexChange();
		return TCL_OK;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::incrementIndex(float amount, int dim)
{
	SmartValue::IndexType type;

	if (dim >= 0 && dim < numDimensions)
	{
		type = indecies[dim].type;
		if (type == SmartValue::Float)
			indecies[dim].value.f += amount;
		else if (type == SmartValue::Integer)
			indecies[dim].value.i += (int)amount;
		else
		return TCL_ERROR;

		if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
		{
			char command[1024];
			sprintf(command, "gz tellObject %s index %d set %f as %s", getAddress()->getAddress().c_str(), dim, amount, SmartValue::typeName(SmartValue::Float));
			sessionRecorder->recordCommand(command);
		}
		NotifyIndexChange();
		return TCL_OK;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::setIndex(const char *val, int dim)
{
	if (dim >= 0 && dim < numDimensions)
	{
		indecies[dim].set(strdup(val));
		if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
		{
			char command[1024];
			sprintf(command, "gz tellObject %s index %d set %s as %s", getAddress()->getAddress().c_str(), dim, val, SmartValue::typeName(SmartValue::String));
			sessionRecorder->recordCommand(command);
		}
		NotifyIndexChange();
		return TCL_OK;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::setIndex(const void *val, int dim)
{
	if (dim >= 0 && dim < numDimensions)
	{
		indecies[dim].set(val);
		if (sessionRecorder->getRecordingState() == SessionRecorder::Recording)
		{
			char command[1024];
            // Matt, I typecasted the following to make it compile, I hope it still does what it was meant to do! RJA
			sprintf(command, "gz tellObject %s index %d set @%lx as %s", getAddress()->getAddress().c_str(), dim, (unsigned long)val, SmartValue::typeName(SmartValue::Pointer));
			sessionRecorder->recordCommand(command);
		}
		NotifyIndexChange();
		return TCL_OK;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::insertValue(GLboolean beforeCurrent, int dim)
{
	if (support)
	{
		if (support->insertValue(this, beforeCurrent, dim))
			return TCL_OK;
	}
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateWrapper::deleteValue(int dim)
{
	if (support)
	{
		if (support->deleteValue(this, dim))
			return TCL_OK;
	}
	return TCL_ERROR;
}
