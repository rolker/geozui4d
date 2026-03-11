// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// RelationshipManager.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class RelationshipManager;

#ifndef RELATIONSHIPMANAGER_H
#define RELATIONSHIPMANAGER_H

#include "widgets/Sv_Standards.h"
#include <typeinfo>
class PosableStateRelationship;
class PosableStateBase;
class RelationshipQueryResult;
/** 
 *	This class provides a way to register, store, access, and unregister
 *	PosableStateRelationships, as referenced by certain information about the
 *	relationship.  This class is responsible for implementing the following.
 *	\li Managing a database of all PosableStateBases.
 *  \li Providing methods for registering and unregistering relationships.
 *  \li Providing various methods for accessing relationships, such as by
 *	    creator, or by managed PosableStateBases.
 *	\note Transformed to RelationshipManager by mdp 6/20/2001
 *	\author Matt Plumlee (mdp)
 *  \date 1/15/2001 as LockManager
 */
class RelationshipManager
{
  public:
	RelationshipManager();
	virtual ~RelationshipManager();

	/*---PosableStateBase Management---*/
	virtual long manage(PosableStateBase *pose);
	virtual void relinquish(PosableStateBase *pose = 0);
	virtual void replace(PosableStateBase *oldPose, PosableStateBase *newPose);
	virtual long numManaging();
	virtual void poseChanged(PosableStateBase *whichPose, long whatChanged);

	virtual void flush();

	/*---Relationship/Posable Associating---*/
	virtual void attach(PosableStateRelationship *relationship, PosableStateBase *pose);
	virtual void detach(PosableStateRelationship *relationship, PosableStateBase *pose);

	void registerRelationship(PosableStateRelationship *newRelationship);
	void unregisterRelationship(PosableStateRelationship *dyingRelationship);

	enum QueryType {WithoutRestriction		= 0x00,
					WithOwnerPose			= 0x01,
					WithTargetPose			= 0x02,
					BetweenPoses			= 0x04,
					CreatedBy				= 0x08,
					WithinMask				= 0x10,
					WithoutMask				= 0x20,
					MatchingMask			= 0x30,
					MatchingAtLeastMask		= 0x70,
					MaskFromOwner			= 0x80,
					WithinMaskFromOwner		= 0x90,
					WithoutMaskFromOwner	= 0xA0,
					MatchingMaskFromOwner	= 0xB0,
					MatchingAtLeastMaskFromOwner = 0xF0,
					OfClassType				= 0x100,
					NotOfClassType			= 0x200};
	RelationshipQueryResult *getConcerning(const PosableStateBase *pose);
	RelationshipQueryResult *getWithOwner(const PosableStateBase *ownerPose, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getWithTarget(const PosableStateBase *targetPose, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getBetween(const PosableStateBase *aPose, const PosableStateBase *bPose, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getLocksBetween(const PosableStateBase *aPose, const PosableStateBase *bPose, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getCreatedBy(const void *creator, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getMatchingAtLeastMask(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getMatchingMask(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getWithinMask(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getWithoutMask(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getMatchingAtLeastMaskFromOwner(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getMatchingMaskFromOwner(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getWithinMaskFromOwner(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getWithoutMaskFromOwner(long changeMask, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getOfType(const char *classType, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getNotOfType(const char *classType, RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *getThoseThatUpdate(const PosableStateBase *updatedPose, long changeMask, const char *classType = NULL,
										   RelationshipQueryResult *within = NULL);
	RelationshipQueryResult *get(long queryType = WithoutRestriction,
							  const PosableStateBase *ownerPose = NULL,
							  const PosableStateBase *targetPose = NULL,
							  const void *creator = NULL,
							  long changeMask = 0,
							  const char *classType = NULL,
							  RelationshipQueryResult *within = NULL);

  protected:
	struct ManagedPosableRecord
	{
		PosableStateBase *pose;
		RelationshipQueryResult *relationshipsConcerningPose;
		long changesToUpdateWith;
	};
	ManagedPosableRecord *posableTable;
	void prepareTableForNewRecord();
	long nextRecord, numRecords, fullRecords;

	GLboolean needsUpdate;

	void prepareTableForNewRelationship();

	PosableStateRelationship **relationshipTable;
	long firstUnusedRelationship, lastRelationship;
};

extern RelationshipManager *relationshipManager;
#endif
