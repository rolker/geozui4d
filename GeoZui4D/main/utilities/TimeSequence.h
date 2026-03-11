// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// TimeSequence.h

#ifndef TIMESEQUENCE_H
#define TIMESEQUENCE_H

#include <gz4d/gz4d.h>
#include <gz4d/util/Utilities.h>
#include "SDV_types.h"
#include <sstream>


#ifndef NULL
#define NULL 0
#endif

/** 
 *	Class for maintaining a linked list of time-ordered pointers to externally
 *	instantiated objects.  Pointers to objects inserted in the LinkedList are
 *	saved as type void*.  
 *  \note It is up to the user to recast the returned pointer
 *	to the proper type before referencing it.  Deleting the list does not
 *	affect (ie. delete) the objects referenced by the stored void pointers.
 *	\author Matthew Plumlee (mdp) & Roland Arsenault (rja)
 *  \date 12/9/2004
 *  \image html timeSequence.jpg
 */
///////////////////////////////////////////////////////////////////////////////
class TimeSequence
{
public:
	TimeSequence();
	~TimeSequence();

//	static double NaN;

	void insert(double t, void *data) {internalInsert(t, data);}
	void remove(void *data); //remove all instances from list, but don't delete

	void *startIteration(double start = std::numeric_limits<double>::quiet_NaN()); //sets the iterator to the node at the passed time (or the nearest time prior if the passed time is not represented exactly), or the first time if no argument is given
	void *next(); //iterates
	void *previous(); //iterates back
	double getTime(); //returns time at current iteration node
	double getNextTime(); //returns time at next iteration node

	bool isNext(double t);

	void *get(double t = std::numeric_limits<double>::quiet_NaN()); //if iterating and no argument is given, get the current one, else find the data associated with time t
	void *get(double t, double &weight, void *&next); //get information necessary to interpolate between the returned value and 'next' ((1 - weight) * 'returned' + (weight) * 'next')

    double getStartTime() {return start? start->time : gz4d::Nan<double>();}
    double getEndTime() {return end? end->time : gz4d::Nan<double>();}
        void getHtmlInfo(std::stringstream &html, short level);

	struct TimeSequenceNode
	{
		TimeSequenceNode(double t, void *d = NULL) {time = t; data = d;}

		double time;
		TimeSequenceNode *next, *prev;
		void *data;
	};

protected:
	void remove(TimeSequenceNode *c);

	TimeSequenceNode *internalInsert(double t, void *data);
	TimeSequenceNode *find(double t, TimeSequenceNode *&next);

	TimeSequenceNode *start, *end;
	TimeSequenceNode *iterator;
};

#endif
