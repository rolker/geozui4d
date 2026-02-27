/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// RelationshipQueryResult.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class RelationshipQueryResult;

#ifndef RELATIONSHIPQUERYRESULT_H
#define RELATIONSHIPQUERYRESULT_H

class PosableStateRelationship;
/** 
 *	This class provides ways to access results from a query on a
 *	RelationshipManager.  This class is responsible for implementing the
 *	following.
 *	\li Providing methods for adding result nodes.
 *	\li Providing various methods for accessing result nodes, such as
 *		by iterator or index.
 *
 *	\note Created as LockQueryResult (separated out from struct in LockManager)
 *	Transformed to RelationshipQueryREsult by mdp 6/20/2001
 *  \author Matt Plumlee (mdp)
 *  \date 2/8/2001 

 */
class RelationshipQueryResult
{
  public:
	RelationshipQueryResult();
	~RelationshipQueryResult();

	//mutator methods
	void addResult(PosableStateRelationship *relationship);
	void removeResult(PosableStateRelationship *relationship);

	//accessor methods
	int numResults();
	PosableStateRelationship *first();
	PosableStateRelationship *next();

  private:
	struct IndividualResult
	{
		PosableStateRelationship *relationship;
		IndividualResult *next;

		~IndividualResult() {if (next) delete next;}
	};

	IndividualResult *head, *current;
	int resultCount;
};

#endif
