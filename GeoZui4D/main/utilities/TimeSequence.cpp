/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// TimeSequence.cpp
/** 
 *	Class for maintaining a linked list of time-ordered pointers to externally
 *	instantiated objects.  Pointers to objects inserted in the LinkedList are
 *	saved as type void*.  It is up to the user to recast the returned pointer
 *	to the proper type before referencing it.  Deleting the list does not
 *	affect (ie. delete) the objects referenced by the stored void pointers.
 *  
 *	Created by Matthew Plumlee & Roland Arsenault 12/9/2004
 */
///////////////////////////////////////////////////////////////////////////////
#include "utilities/TimeSequence.h"
#include <math.h>
#include "utilities/TimePieces.h"
#include <gz4d/util/Utilities.h>

//double TimeSequence::NaN = log(0.0);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
TimeSequence::TimeSequence()
{
	start = end = iterator = NULL;
}

/*---------------------------------------------------------------------------*/
TimeSequence::~TimeSequence()
{
	iterator = start;
	while (iterator)
	{
		end = iterator;
		iterator = iterator->next;
		delete end;
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
TimeSequence::TimeSequenceNode *TimeSequence::internalInsert(double t, void *data)
{
	TimeSequenceNode *c, *newNode;

	newNode = new TimeSequenceNode(t, data);

	if (!start) //no nodes, just put it in
	{
		start = end = newNode;
		newNode->next = NULL;
		newNode->prev = NULL;
	}
	else if (t >= end->time) //insert at the end
	{
		end->next = newNode;
		newNode->prev = end;
		newNode->next = NULL;
		end = newNode;
	}
	else //insert before the end
	{
		c = start;
		while (c->time < t)
			c = c->next;
		//on exiting the loop, c has advanced past insertion point
		if (c->prev)
			c->prev->next = newNode;
		else
			start = newNode;
		newNode->prev = c->prev;
		newNode->next = c;
		c->prev = newNode;
	}

	return newNode;
}

/*---------------------------------------------------------------------------*/
/**	remove all instances from list, but don't delete
 */
void TimeSequence::remove(void *data)
{
	TimeSequenceNode *c, *doomedNode;

	c = start;
	while (c)
	{
		if (c->data == data)
		{
			doomedNode = c;
			c = c->next;
			remove(doomedNode);
		}
		else
			c = c->next;
	}
}

/*---------------------------------------------------------------------------*/
/**	remove all instances from list, but don't delete
 */
void TimeSequence::remove(TimeSequenceNode *c)
{
	if (!c)
		return;

	if (c->prev)
		c->prev->next = c->next;
	else
		start = c->next;

	if (c->next)
		c->next->prev = c->prev;
	else
		end = c->prev;

	if (iterator == c)
		iterator = c->next;

	delete c;
}

/*---------------------------------------------------------------------------*/
/**	sets the iterator to the node at the passed time (or the nearest time
	prior if the passed time is not represented exactly), or the first time if
	no argument is given
 */
void *TimeSequence::startIteration(double iter_start)
{
	iterator = start;
	if (!iterator)
		return NULL;

    if (!gz4d::IsNan(iter_start))
	{
		while (iterator->time <= iter_start)
		{
			if (iterator->next)
				iterator = iterator->next;
			else
				return iterator->data;
		}
		if (iterator->prev)
			iterator = iterator->prev;
	}
	return iterator->data;
}

/*---------------------------------------------------------------------------*/
/**	iterates
 */
void *TimeSequence::next()
{
	if (iterator)
		iterator = iterator->next;
	else
		iterator = start;//this makes it so if prev forced us off the front, we're back on again

	if (iterator)
		return iterator->data;
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
/**	iterates back
 */
void *TimeSequence::previous()
{
	if (iterator)
		iterator = iterator->prev;
	else
		iterator = end;//this makes it so if next forced us off the end, we're back on again

	if (iterator)
		return iterator->data;
	else
		return NULL;
}

/*---------------------------------------------------------------------------*/
/**	returns time at current iteration node
 */
double TimeSequence::getTime()
{
	if (iterator)
		return iterator->time;

    return gz4d::Nan<double>();
}

/*---------------------------------------------------------------------------*/
/**	returns time at next iteration node
 */
double TimeSequence::getNextTime()
{
	if (iterator && iterator->next)
		return iterator->next->time;

    return gz4d::Nan<double>();
}


/*---------------------------------------------------------------------------*/
bool TimeSequence::isNext(double t)
{
	TimeSequenceNode *c;

	if (!iterator)
		return false;

	c = iterator;
	
	if(t<c->time)
		return true;
	
	c = c->next;
	if (!c)
		return false;

	if (t >= c->time)
		return true;

	return false;
}

/*---------------------------------------------------------------------------*/
/**	if iterating and no argument is given, get the current one, else find the
	data associated with time t (or the nearest time prior if the passed time
	is not represented exactly).  If t is prior to the starting time, the data
	at the first node is returned.
 */
void *TimeSequence::get(double t)
{
	TimeSequenceNode *n = NULL;

    if (gz4d::IsNan(t))
	{
		if (iterator)
			return iterator->data;
		return NULL;
	}

	if (find(t, n))
		return iterator->data;
	else if (n)
		return n->data;
	return NULL;
}

/*---------------------------------------------------------------------------*/
/**	get information necessary to interpolate between the returned value and
	'next' ((1 - weight) * 'returned' + (weight) * 'next')
	NULL is returned if the passed time is not on or between two passed nodes
 */
void *TimeSequence::get(double t, double &weight, void *&next)
{
	TimeSequenceNode *n = NULL;
	double diff;

	if (!find(t, n))
		return NULL;

	if (iterator && iterator->time == t)
	{
		weight = 1.0;
		next = iterator->data;
		return iterator->data;
	}

	if(n)
	{
		diff = n->time - iterator->time;
		weight = (float) ((t - iterator->time) / diff);
//		cerr << "t: " << (t - 1122985600.0) << ",\tweight: " << weight << endl;
		next = n->data;
		return iterator->data;
	}
	return NULL;
}

/*---------------------------------------------------------------------------*/
/**	Find the node(s) surrounding the passed time t. NULL is returned if the
 *	passed time is before the first node
 */
TimeSequence::TimeSequenceNode *TimeSequence::find(double t, TimeSequenceNode *&next)
{
	
	if (!iterator)
		iterator = start;
	if (!iterator)
		return NULL;

	//make it so iterator->time is at or prior to t and n->time is subsequent to t
	if (iterator->time > t)
	{
		while (iterator && iterator->time > t)
		{
			next = iterator;
			iterator = iterator->prev;
		}
		if (!iterator) //if t < starting time, there is no valid node
			return NULL;
	}
	else
	{
		next = iterator->next;
		while (next && (next->time <= t))
		{
			iterator = next;
			next = next->next;
		}
	}

	//no next? only report non-NULL if iterator->time == t. // rja sez: why would we want to do this? A value is valid from its timestamp until a later value appears. If we return null past last value, the last value will only be valid at the exact time it becomes valid.
	//if (!next)
	//{
	//	if (iterator && iterator->time == t)
	//		return iterator;
	//	return NULL;
	//}
	return iterator;
}

void TimeSequence::getHtmlInfo(std::stringstream &html, short level)
{
	html << "<h" << level << ">TimeSequence</h" << level << ">";
	if (start)
	{
		html << "start: " << start->time << "<br>";
		html << "end: " << end->time << "<br>";
	}
	else
		html << "(empty)<br>";
}
