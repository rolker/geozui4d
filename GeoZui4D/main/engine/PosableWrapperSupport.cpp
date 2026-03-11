// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// PosableWrapperSupport.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the base class for classes that support PosableStateWrapper
 *	attachments (usually these are non-scalar entities for which it would
 *	make just as much sense to have a posable for each position).
 *
 *	Created by mdp 2/11/2002
 */
/*---------------------------------------------------------------------------*/
#include "engine/PosableStateWrapper.h"
#include "engine/PosableWrapperSupport.h"
#include "PosableStateUser.h"
#include <string.h>

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
PosableWrapperSupport::PosableWrapperSupport()
{
	associativeAttribute = -1;
	numWrapableAttributes = 0;
	wrapableAttributeNames = NULL;
}

/*---------------------------------------------------------------------------*/
PosableWrapperSupport::~PosableWrapperSupport()
{
}

/*-----------------------------------Accessors-------------------------------*/
/*---------------------------------------------------------------------------*/
PosableStateBase *PosableWrapperSupport::getPosable()
{
	PosableStateUser *user;

	DYNAMIC_CAST(user, PosableStateUser *, this);
	if (user)
		return user->getPosable();
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
/**	Returns the number of attributes this object has
 */
int PosableWrapperSupport::getNumAttributes(PosableStateWrapper *newWrapper)
{
	return numWrapableAttributes;
}

/*---------------------------------------------------------------------------*/
/** Returns the index of the attribute at the passed name, or -1 if it is not
 *	a valid attribute name.
 */
int PosableWrapperSupport::getAttributeIndex(PosableStateWrapper *newWrapper, std::string attributeName)
{
    unsigned int i;

	for (i = 0; i < numWrapableAttributes; i++)
	{
		if (wrapableAttributeNames[i] == attributeName)
			return i;
	}
	return -1;
}

/*---------------------------------------------------------------------------*/
/**	Returns the attribute name for the passed attribute index, or NULL if it
 *	is not a valid attribute index.
 */
const char *PosableWrapperSupport::getAttributeName(PosableStateWrapper *newWrapper, unsigned int attributeIndex)
{
	if (attributeIndex < numWrapableAttributes && attributeIndex >= 0)
		return wrapableAttributeNames[attributeIndex];
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
/**	Returns through val the attribute value at the passed index if the index is
 *	valid (and reutrns TCL_OK).  Otherwise, returns TCL_ERROR.
 */
int PosableWrapperSupport::getAttribute(PosableStateWrapper *newWrapper, SmartValue &val, unsigned int attributeIndex)
{
	return TCL_ERROR;
}

/*-----------------------------------Mutators--------------------------------*/
/*---------------------------------------------------------------------------*/
/**	returns GL_TRUE if the wrapper was attached to a new location.
 */
GLboolean PosableWrapperSupport::attachWrapper(PosableStateWrapper *newWrapper)
{
	int idx;
	PosableWrapperList *listNode = findWrapperNode(newWrapper, GL_FALSE);

	if (!listNode)
		return GL_FALSE;

	if (listNode->wrapper == newWrapper)
		return GL_FALSE;
	listNode->addWrapper(newWrapper);
	newWrapper->attachTo(this);

	idx = newWrapper->getIndexingAttribute();
	if (idx >= 0 && idx == associativeAttribute)
	{
		if (requestAssociativeAttribute(idx) != GL_TRUE)
			newWrapper->setIndexingAttribute(-1);
	}

	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/**	returns GL_TRUE if the wrapper was found (and it has been deleted).
 */
GLboolean PosableWrapperSupport::detachWrapper(PosableStateWrapper *wrapper)
{
	int idx;

	PosableWrapperList *listNode = findWrapperNode(wrapper, GL_TRUE, GL_TRUE);

	if (!listNode)
		return GL_FALSE;

	idx = wrapper->getIndexingAttribute();
	if (idx >= 0 && idx == associativeAttribute)
		releaseAssociativeAttribute(idx);
	listNode->removeWrapper(wrapper);
	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/**	returns GL_TRUE if the wrapper is updated to a valid index.
 */
GLboolean PosableWrapperSupport::indexUpdated(PosableStateWrapper *wrapper)
{
	PosableWrapperList *oldList, *listNode = findWrapperNode(wrapper);

	oldList = wrapper->getList();
	if (!listNode)
	{
		if (oldList)
			oldList->removeWrapper(wrapper);
		return GL_FALSE;
	}
	if (listNode == oldList)
		return GL_TRUE;
	oldList->removeWrapper(wrapper);
	listNode->addWrapper(wrapper);
	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/**	returns GL_TRUE if the value could be inserted, GL_FALSE otherwise.
 */
GLboolean PosableWrapperSupport::insertValue(PosableStateWrapper *wrapper,
											 GLboolean beforeCurrent, int dim)
{
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/**	returns GL_TRUE if the value could be deleted, GL_FALSE otherwise.
 */
GLboolean PosableWrapperSupport::deleteValue(PosableStateWrapper *wrapper,
											 int dim)
{
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/**	returns TCL_OK if the attribute value could be set, TCL_ERROR otherwise.
 */
int PosableWrapperSupport::setAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex)
{
	return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PosableWrapperList::PosableWrapperList()
{
	wrapper = NULL;
	next = NULL;
}

/*---------------------------------------------------------------------------*/
/**	addWrapper always adds a new wrapper node to the second position in the
 *	list, and copies the top information down.  This way, the pointer to the
 *	list is always valid, but new items are added to the top.
 */
void PosableWrapperList::addWrapper(PosableStateWrapper *newWrapper)
{
	PosableWrapperList *newNode;

	if (wrapper) //only create a new node if we're full.
	{
		newNode = new PosableWrapperList;
		newNode->next = next;
		newNode->wrapper = wrapper;
		next = newNode;
	}
	wrapper = newWrapper;
	wrapper->setList(this);
}

/*---------------------------------------------------------------------------*/
void PosableWrapperList::notify(long changeMask)
{
	if (wrapper)
		wrapper->update(changeMask);
	if (next)
		next->notify(changeMask);
}

/*---------------------------------------------------------------------------*/
/**	removeWrapper always ensures that the top node remains the top node, but
 *	otherwise removes the list node as normal.  Returns GL_TRUE if the list
 *  is empty, and otherwise returns GL_FALSE
 */
GLboolean PosableWrapperList::removeWrapper(PosableStateWrapper *oldWrapper,
											PosableWrapperList *parent)
{
	if (wrapper == oldWrapper) //found it!
	{
		wrapper->setList(NULL);
		if (parent) //we're not a head, so it is ok to delete us.
		{
			parent->next = next;
			delete this;
		}
		else //we're a head and cannot be deleted
		{
			if (next) //copy our neighbor's stuff and get rid of them instead
			{
				wrapper = next->wrapper;
				next = next->next;
				delete next;
				//next->removeWrapper(oldWrapper, this);
			}
			else //become empty.
			{
				wrapper = NULL;
				return GL_TRUE;
			}
		}
	}
	else if (next)
		next->removeWrapper(oldWrapper, this);

	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/**	findWrapper returns the associated list entry if the wrapper is in the
 *	list, and NULL otherwise.
 */
PosableWrapperList *PosableWrapperList::findWrapper(PosableStateWrapper *targetWrapper)
{
	if (wrapper == targetWrapper)
		return this;
	else if (next)
		return next->findWrapper(targetWrapper);
	else
		return NULL;
}
