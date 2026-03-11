// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// RelationshipQueryResult.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides ways to access results from a query on a
 *	RelationshipManager.  This class is responsible for implementing the
 *	following.
 *		* Providing methods for adding result nodes.
 *		* Providing various methods for accessing result nodes, such as
 *		  by iterator or index.
 *
 *	Created as LockQueryResult by mdp 2/8/2001 (separated out from struct in
 *												LockManager)
 *	Transformed to RelationshipQueryREsult by mdp 6/20/2001
 */
/*---------------------------------------------------------------------------*/

#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "widgets/Sv_Standards.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
RelationshipQueryResult::RelationshipQueryResult()
{
	current = head = NULL;
	resultCount = 0;
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult::~RelationshipQueryResult()
{
	if (head)
		delete head;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void RelationshipQueryResult::addResult(PosableStateRelationship *relationship)
{
	IndividualResult *newResult;

	newResult = new IndividualResult;
	newResult->relationship = relationship;
	newResult->next = head;
	head = newResult;

	resultCount++;
}

/*---------------------------------------------------------------------------*/
void RelationshipQueryResult::removeResult(PosableStateRelationship *relationship)
{
	IndividualResult *currentRecord, *previousRecord = NULL;

	//find the record we want to get rid of
	for (currentRecord = head;
		 currentRecord != NULL;)
	{
		if (currentRecord->relationship == relationship) //if we find it, delete it and keep searching.
		{
			//move our iterator out of the way
			if (current == currentRecord)
				current = current->next;

			//hook the previous link to the next link
			if (previousRecord)
				previousRecord->next = currentRecord->next;
			else
				head = currentRecord->next;

			//delete the current link
			currentRecord->next = NULL; //do this so it doesn't clobber the rest of the list upon the deletion.
			delete currentRecord;
			resultCount--;
			if (!previousRecord)
			{
				currentRecord = head;
				continue;
			}
			else
				currentRecord = previousRecord;
		}
		else
			previousRecord = currentRecord;

		if (currentRecord)
			currentRecord = currentRecord->next;
	}
}

/*---------------------------------------------------------------------------*/
int RelationshipQueryResult::numResults()
{
	return resultCount;
}

/*---------------------------------------------------------------------------*/
PosableStateRelationship *RelationshipQueryResult::first()
{
	current = head;

	if (current)
		return current->relationship;
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
PosableStateRelationship *RelationshipQueryResult::next()
{
	if (current)
		current = current->next;

	if (current)
		return current->relationship;
	else
		return NULL;
}

