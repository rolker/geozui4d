/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// RelationshipManager.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides a way to register, store, access, and unregister
 *	PosableStateRelationships, as referenced by certain information about the
 *	relationship.  This class is responsible for implementing the following.
 *		* Managing a database of all PosableStateBases.
 *		* Providing methods for registering and unregistering relationships.
 *		* Providing various methods for accessing relationships, such as by
 *		  creator, or by managed PosableStateBases.
 *
 *	Created by mdp 1/15/2001 as LockManager
 *	Transformed to RelationshipManager by mdp 6/20/2001
 */
/*---------------------------------------------------------------------------*/

#include "MainWindow.h"
#include <string.h>
#include "engine/PosableStateBase.h"
#include "PosableStateRelationship.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include <typeinfo>

extern MainWindow *mainWindow;

static RelationshipManager instanceOfRelationshipManager;
RelationshipManager *relationshipManager = &instanceOfRelationshipManager;

#define RELATIONSHIP_TABLE_QUANTUM 20
#define POSABLE_RECORD_TABLE_QUANTUM 35

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
RelationshipManager::RelationshipManager()
{
	numRecords = 0;
	nextRecord = 0;
	fullRecords = 0;

	firstUnusedRelationship = 0;
	lastRelationship = -1;
}

/*---------------------------------------------------------------------------*/
RelationshipManager::~RelationshipManager()
{
	delete [] posableTable;
	delete [] relationshipTable;
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::prepareTableForNewRecord()
{
	ManagedPosableRecord *newTable;
	long newNumRecords, currentRecord;

	if (numRecords <= nextRecord)
	{
		//create new space
		newNumRecords = numRecords + POSABLE_RECORD_TABLE_QUANTUM;
		newTable = new ManagedPosableRecord[newNumRecords];

		//copy over the old records
		for (currentRecord = 0; currentRecord < nextRecord; currentRecord++)
		{
			newTable[currentRecord].pose = posableTable[currentRecord].pose;
			newTable[currentRecord].relationshipsConcerningPose =
					posableTable[currentRecord].relationshipsConcerningPose;
			newTable[currentRecord].changesToUpdateWith =
					posableTable[currentRecord].changesToUpdateWith;
		}

		//create new relationships to fill the space.
		for (; currentRecord < newNumRecords; currentRecord++)
		{
			newTable[currentRecord].pose = NULL;
			newTable[currentRecord].relationshipsConcerningPose = NULL;
		}

		//set the new table as the current one.
		delete [] posableTable;
		posableTable = newTable;
		numRecords = newNumRecords;
	}
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::prepareTableForNewRelationship()
{
	PosableStateRelationship **newTable;
	long newLastRelationship, currentRelationship;

	if (lastRelationship < firstUnusedRelationship)
	{
		//create new space
		newLastRelationship = lastRelationship + RELATIONSHIP_TABLE_QUANTUM;
		newTable = new PosableStateRelationship*[newLastRelationship + 1];

		//copy over the old relationships
		for (currentRelationship = 0; currentRelationship <= lastRelationship;
			 currentRelationship++)
			newTable[currentRelationship] = relationshipTable[currentRelationship];

		//create new relationships to fill the space.
		for (; currentRelationship <= newLastRelationship; currentRelationship++)
			newTable[currentRelationship] = NULL;

		//set the new table as the current one.
		delete [] relationshipTable;
		relationshipTable = newTable;
		lastRelationship = newLastRelationship;
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
long RelationshipManager::manage(PosableStateBase *pose)
{
	prepareTableForNewRecord();
	posableTable[nextRecord].pose = pose;
	posableTable[nextRecord].relationshipsConcerningPose = new RelationshipQueryResult;
	posableTable[nextRecord].changesToUpdateWith = 0;
	fullRecords++;
	return nextRecord++;
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::relinquish(PosableStateBase *pose)
{
	long index = pose->getSerialNumber();
	PosableStateRelationship *currentRelationship;
	RelationshipQueryResult *relationshipsConcerningPose;

	//remove all traces of this pose in all relationships and records
	if (posableTable[index].pose != pose) //added in case there was a replacement.
		return;
	posableTable[index].pose = NULL;
	relationshipsConcerningPose = posableTable[index].relationshipsConcerningPose;
	posableTable[index].changesToUpdateWith = 0;

	//since we're removing items, we always want to grab the first item
	//(rather than next)
	for (currentRelationship = relationshipsConcerningPose->first();
		 currentRelationship;
		 currentRelationship = relationshipsConcerningPose->first())
	{
		currentRelationship->relinquish(pose);
		if (currentRelationship->numManaging() < currentRelationship->getDeleteThreshold())
			delete currentRelationship;
	}
	posableTable[index].relationshipsConcerningPose = NULL;
	fullRecords--;
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::replace(PosableStateBase *oldPose,
								  PosableStateBase *newPose)
{
	PosableStateRelationship *currentRelationshipRecord;
	RelationshipQueryResult *relationshipsConcerningPose;
	long index = oldPose->getSerialNumber();

	posableTable[index].pose = newPose;

	relationshipsConcerningPose = posableTable[index].relationshipsConcerningPose;
	for (currentRelationshipRecord = relationshipsConcerningPose->first();
		 currentRelationshipRecord != NULL;
		 currentRelationshipRecord = relationshipsConcerningPose->next())
	{
		currentRelationshipRecord->replace(oldPose, newPose);
	}

}

/*---------------------------------------------------------------------------*/
/**	The numManaging function returns how many PosableStateBases this instance
 *	is currently managing (minus the number it has relinquished).
 */
long RelationshipManager::numManaging()
{
	return fullRecords;
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::poseChanged(PosableStateBase *whichPose, long whatChanged)
{
	PosableStateRelationship *currentRelationshipRecord;
	RelationshipQueryResult *relationshipsConcerningPose;
	long currentRecord;

	if (whichPose)
	{
		currentRecord = whichPose->getSerialNumber();

   		if (!mainWindow->establishValidContext())
		{
			posableTable[currentRecord].changesToUpdateWith |= whatChanged;
			needsUpdate = GL_TRUE;
			return;
		}

		//If we got here, we can go forward with the relationships.
		relationshipsConcerningPose = posableTable[currentRecord].relationshipsConcerningPose;
		whatChanged |= posableTable[currentRecord].changesToUpdateWith;
		for (currentRelationshipRecord = relationshipsConcerningPose->first();
			 currentRelationshipRecord != NULL;
			 currentRelationshipRecord = relationshipsConcerningPose->next())
		{
			currentRelationshipRecord->PoseChanged(whichPose, whatChanged);
		}
		posableTable[currentRecord].changesToUpdateWith = 0;
	}
}

/*---------------------------------------------------------------------------*/
/**	This procedure attempts to flush any unpropogated pose changes.  It can
 *	fail if it cannot establish a GL context.
 */
void RelationshipManager::flush()
{
	PosableStateRelationship *currentRelationshipRecord;
	RelationshipQueryResult *relationshipsConcerningPose;
	PosableStateBase *currentPose;
	long currentRecord, whatChanged;

	if (!needsUpdate || !mainWindow->establishValidContext())
		return;

	for (currentRecord = 0; currentRecord < nextRecord; currentRecord++)
	{
		whatChanged = posableTable[currentRecord].changesToUpdateWith;
		if (!whatChanged)
			continue;

		currentPose = posableTable[currentRecord].pose;
		relationshipsConcerningPose = posableTable[currentRecord].relationshipsConcerningPose;
		for (currentRelationshipRecord = relationshipsConcerningPose->first();
			 currentRelationshipRecord != NULL;
			 currentRelationshipRecord = relationshipsConcerningPose->next())
		{
			currentRelationshipRecord->PoseChanged(currentPose, whatChanged);
		}
		posableTable[currentRecord].changesToUpdateWith = 0;
	}
	
	needsUpdate = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void RelationshipManager::attach(PosableStateRelationship *relationship,
								 PosableStateBase *pose)
{
	long index;

	if (!pose || !relationship)
		return;

	//create a new record
	index = pose->getSerialNumber();

	//insert into the relationships concerning this pose.
	posableTable[index].relationshipsConcerningPose->addResult(relationship);
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::detach(PosableStateRelationship *relationship,
								 PosableStateBase *pose)
{
	long index;

	if (!pose || !relationship)
		return;

	index = pose->getSerialNumber();

	posableTable[index].relationshipsConcerningPose->removeResult(relationship);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void RelationshipManager::registerRelationship(PosableStateRelationship *newRelationship)
{
	prepareTableForNewRelationship();
	relationshipTable[firstUnusedRelationship++] = newRelationship;
}

/*---------------------------------------------------------------------------*/
void RelationshipManager::unregisterRelationship(PosableStateRelationship *dyingRelationship)
{
	long currentRelationship, emptyingRelationship;

	for (currentRelationship = 0; currentRelationship < lastRelationship;
		 currentRelationship++)
	{
		if (relationshipTable[currentRelationship] == dyingRelationship)
		{
			emptyingRelationship = firstUnusedRelationship - 1;

			//copy the last relationship into the new hole
			if (currentRelationship < emptyingRelationship)
			{
				relationshipTable[currentRelationship] = relationshipTable[emptyingRelationship];
			}

			//empty the last relationship.
			relationshipTable[emptyingRelationship] = NULL;
			firstUnusedRelationship--;
		}
	}
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getConcerning(const PosableStateBase *pose)
{
	return posableTable[pose->getSerialNumber()].relationshipsConcerningPose;
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::get(long query,
									   const PosableStateBase *ownerPose,
									   const PosableStateBase *targetPose,
									   const void *creator,
									   long changeMask,
									   const char *classType,
									   RelationshipQueryResult *within)
{
	RelationshipQueryResult *buildingResult;
	long currentRelationship, fromWhom;
	GLboolean checkingWithin, currentQueryResult;
	PosableStateRelationship *rel;
	int matchType;
        std::string typeName;

	currentRelationship = 0;

	buildingResult = new RelationshipQueryResult;

	checkingWithin = (within)? GL_TRUE : GL_FALSE;

	//use a different loop structure based on whether or not we are
	//searching within a previous query result (but use the same
	//body at performChecks).
	//------------------------------------------------------
	if (checkingWithin)
	{
		for (rel = within->first(); rel; rel = within->next())
		{
			goto performChecks;
continueWithin:
			;
		}
	}
	//------------------------------------------------------
	else
	{
		for (currentRelationship = 0;
			 currentRelationship < firstUnusedRelationship;
			 currentRelationship++)
		{
			rel = relationshipTable[currentRelationship];
			goto performChecks;
continueBaseSearch:
			;
		}
	}

	//------------------------------------------------------
	return buildingResult;


	//------------------------------------------------------
performChecks:
	//where the actual query checks get done.
	currentQueryResult = true;

	//check owner
	if ((query & WithOwnerPose) &&
		!(rel->getPoseOfOwner() == ownerPose))
		currentQueryResult = GL_FALSE;
	//check target
	if ((query & WithTargetPose) &&
		!(rel->getPoseOfTarget() == targetPose))
		currentQueryResult = GL_FALSE;
	//check if both poses are in either slot without regard to owner/target
	if ((query & BetweenPoses) &&
		!((rel->getPoseOfOwner() == ownerPose &&
		   rel->getPoseOfTarget() == targetPose) ||
		  (rel->getPoseOfOwner() == targetPose &&
		   rel->getPoseOfTarget() == ownerPose)))
		currentQueryResult = GL_FALSE;
	//check creator
	if ((query & CreatedBy) &&
		!(rel->getCreator() == creator))
		currentQueryResult = GL_FALSE;
	//check changeMask (split up the info into Matching's and MaskFromOwner)
	matchType = query & MatchingAtLeastMask;
	if (query & MaskFromOwner)
		fromWhom = PosableStateRelationship::FromOwner;
	else
		fromWhom = PosableStateRelationship::FromTarget;
	//does mask contain any bits in common with changeMask?
	if ((matchType == WithinMask) &&
		!(rel->getType(fromWhom) & changeMask))
		currentQueryResult = GL_FALSE;
	//does mask exclude all bits in changeMask?
	if ((matchType == WithoutMask) &&
		(rel->getType(fromWhom) & changeMask))
		currentQueryResult = GL_FALSE;
	//does mask equal changeMask?
	if ((matchType == MatchingMask) &&
		!(rel->getType(fromWhom) == changeMask))
		currentQueryResult = GL_FALSE;
	//does mask at least match changeMask where there are 1's?
	if ((matchType == MatchingAtLeastMask) &&
		!((rel->getType(fromWhom) & changeMask) == changeMask))
		currentQueryResult = GL_FALSE;
	//check class type
	if (query & (OfClassType | NotOfClassType))
	{
		GET_CLASS_NAME(rel, typeName);
		bool comparisonResult = (typeName == classType);
		if ((!comparisonResult && (query & OfClassType)) || (comparisonResult && (query & NotOfClassType)))
			currentQueryResult = false;
	}

	//If we passed all our tests, create a new result record and
	//tack it on the fron of our return result.
	if (currentQueryResult == GL_TRUE)
		buildingResult->addResult(rel);

	//return to our current loop.
	if (checkingWithin)
		goto continueWithin;
	else
		goto continueBaseSearch;
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getWithOwner(const PosableStateBase *ownerPose, RelationshipQueryResult *within)
{
	return get(WithOwnerPose, ownerPose, NULL, NULL, 0, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getWithTarget(const PosableStateBase *targetPose, RelationshipQueryResult *within)
{
	return get(WithTargetPose, NULL, targetPose, NULL, 0, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getBetween(const PosableStateBase *aPose, const PosableStateBase *bPose, RelationshipQueryResult *within)
{
	return get(BetweenPoses, aPose, bPose, NULL, 0, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getLocksBetween(const PosableStateBase *aPose, const PosableStateBase *bPose, RelationshipQueryResult *within)
{
	return get(BetweenPoses | OfClassType, aPose, bPose, NULL, 0, "PosableStateLock", within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getCreatedBy(const void *creator, RelationshipQueryResult *within)
{
	return get(CreatedBy, NULL, NULL, creator, 0, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getMatchingAtLeastMask(long changeMask, RelationshipQueryResult *within)
{
	return get(MatchingAtLeastMask, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getMatchingMask(long changeMask, RelationshipQueryResult *within)
{
	return get(MatchingMask, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getWithinMask(long changeMask, RelationshipQueryResult *within)
{
	return get(WithinMask, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getWithoutMask(long changeMask, RelationshipQueryResult *within)
{
	return get(WithoutMask, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getMatchingAtLeastMaskFromOwner(long changeMask, RelationshipQueryResult *within)
{
	return get(MatchingAtLeastMaskFromOwner, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getMatchingMaskFromOwner(long changeMask, RelationshipQueryResult *within)
{
	return get(MatchingMaskFromOwner, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getWithinMaskFromOwner(long changeMask, RelationshipQueryResult *within)
{
	return get(WithinMaskFromOwner, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getWithoutMaskFromOwner(long changeMask, RelationshipQueryResult *within)
{
	return get(WithoutMaskFromOwner, NULL, NULL, NULL, changeMask, NULL, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getOfType(const char *classType, RelationshipQueryResult *within)
{
	return get(OfClassType, NULL, NULL, NULL, 0, classType, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getNotOfType(const char *classType, RelationshipQueryResult *within)
{
	return get(NotOfClassType, NULL, NULL, NULL, 0, classType, within);
}

/*---------------------------------------------------------------------------*/
RelationshipQueryResult *RelationshipManager::getThoseThatUpdate(const PosableStateBase *updatedPose, long changeMask, const char *classType,
															RelationshipQueryResult *within)
{
	RelationshipQueryResult *result1 /*, *result2*/;
	long query = WithinMask;
	
	if (classType)
		query |= OfClassType;

	result1 = get(WithTargetPose | query | MaskFromOwner,
		NULL, updatedPose, NULL, changeMask | PosableStateRelationship::FromOwner, classType, within);
// Can take the shortcut and not do the following for now.
/*	result2 = get(WithOwnerPose | query,
				  updatedPose, NULL, NULL, changeMask | PosableStateRelationship::FromTarget, classType, within);

	*result1 += result2;
	delete result2;
*/
	return result1;
}
