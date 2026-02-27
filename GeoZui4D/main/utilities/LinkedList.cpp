/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// LinkedList.cpp
/** 
 *	Class for maintaining a linked list of pointers to externally instantiated
 *  objects. Pointers to objects inserted in the LinkedList are saved as type
 *  void*. It is up to the user to recast the returned pointer to the proper
 *  type before referencing it. Deleting the list does not affect (ie. delete)
 *	the objects referenced by the stored void pointers.
 *  
 *	Call ResetIterator() before iterating through the list. Use GetNext() to
 *	perform list iteration; you've reached the end (it's assumed) when GetNext()
 *	returns NULL. Do NOT attempt to Remove() or Append() while iterating; the
 *	iterator will be reset to the beginning of the list.
 *
 *  This class was created to remove reliance upon
 *  using the C++ standard template library (STL) list template class, mainly
 *  because there is a need to link with 3rd party libraries which use the old
 *  style C headers (especially <iostream.h>) which conflict with the newer STL
 *  libraries.
 *
 *	Created by Colin Ware and Rick Komerska 5/25/2001
 */
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <iostream>
#include <iomanip>

#include "utilities/LinkedList.h"

using namespace std;

LinkedList::LinkedList()
{
	start = new Node;		// dummy node
	start->next = NULL;
	start->data = NULL;

	end = start;

	ResetIterator();

	num_elements = 0;
}

LinkedList::~LinkedList()
{
	Node *tmp1, *tmp2;

	tmp1 = start;
	while (tmp1 != NULL)
	{
		tmp2 = tmp1;
		tmp1 = tmp1->next;
		delete tmp2;
	}
}

void LinkedList::Append(void * i)
{
	Node * tmp;

	tmp = new Node;
	tmp->next = NULL;
	tmp->data = i;

	end->next = tmp;
	end = tmp;

	ResetIterator();

	num_elements++;
}


int LinkedList::InsertAfterIndex(int i, void * d)
{
	if( i < 0 || i > num_elements )
		return -1;

	// move iterator to correct insertion point
	Node *itr = start;
	for( int counter = 0; counter < i; counter++ )
	{
		itr = itr->next;
	}

	// create new Node and attach it
	Node *tmp;
	tmp = new Node;
	tmp->next = itr->next;
	tmp->data = d;
	itr->next = tmp;

	if( i == num_elements )  // special case for last node
		end = tmp;

	num_elements++;

	ResetIterator();

	return num_elements;
}

int LinkedList::InsertBeforeFirstElement(const void * e, void * d)
{
	int i;
	i = GetIndexOfFirstElement(e);
	if(i>1)
	{
		return InsertAfterIndex(i-1,d);
	}
	else
	{
		return InsertAfterFirstElement(NULL,d);
	}
}


int LinkedList::InsertAfterFirstElement(const void * e, void * d)
{
//	int retval = -1;
	int index = 1;

	// special case, insert at beginning
	if( e == NULL )
	{
		InsertAfterIndex(0, d);
		index = 0;
	}
	else
	{
		// move iterator to correct insertion point
		Node *itr = start;
		while(itr->next != NULL)
		{
			if(itr->next->data == e) // we found it!
			{
				// create new Node and attach it
				Node *tmp;
				tmp = new Node;
				tmp->next = itr->next->next;
				tmp->data = d;
				itr->next->next = tmp;

				if( itr->next == end )  // special case for last node
					end = tmp;

				num_elements++;

				ResetIterator();
				break;
			}
			itr = itr->next;
			index++;
		}
	}

	return index;
}

void LinkedList::ResetIterator()
{
	iterator = start->next;
}

int LinkedList::Size() const
{
	return num_elements;
}

void * LinkedList::GetNext(int ahead)
{
	void * tmp;
	int i;

	tmp = NULL;
	if (iterator != NULL)
	{
		tmp = iterator->data;
		for (i = 0; i < ahead; i++) //ahead stuff added by mdp 1/23/2002
			iterator = iterator->next;
	}
	return tmp;
}

void * LinkedList::GetElementAtIndex(int i)		// Ones-based index
{
	ResetIterator();

	if( i < 1 || i > num_elements )
		return NULL;

	for( int counter = 1; counter < i; counter++ )
	{
		iterator = iterator->next;
	}
	return iterator->data;
}

int LinkedList::GetIndexOfFirstElement(const void * i)	// Ones-based index
{
	Node *tmp;
	int retval = -1;
	int counter = 1;

	tmp = start;

	while(tmp->next != NULL)
	{
		if(tmp->next->data == i) // we found it!
		{
			retval = counter;
			break;
		}
		tmp = tmp->next;
		counter++;
	}

	return retval;
}

int LinkedList::RemoveFirst(void * i)
{
	Node *tmp,*tmp2;
	int retval = -1;
	int counter = 1;

	tmp = start;

	while(tmp->next != NULL)
	{
		if(tmp->next->data == i) // we found it!
		{
			tmp2 = tmp->next;
			tmp->next = tmp->next->next;

			if (tmp2 == end)
				end = tmp;  // special case for last node
			
			delete tmp2;

			num_elements--;

			retval = counter;
			break;
		}
		tmp = tmp->next;
		counter++;
	}

	ResetIterator();

	return retval;
}

int LinkedList::RemoveAll(void * i)
{
	Node *tmp,*tmp2;
	int count = 0;

	tmp = start;

	while( (tmp != NULL) && (tmp->next != NULL) )
	{
		if(tmp->next->data == i) // we found it!
		{
			tmp2 = tmp->next;
			tmp->next = tmp->next->next;

			if (tmp2 == end)
				end = tmp;  // special case for last node
			
			delete tmp2;

			num_elements--;
			count++;
		}
		else
			tmp = tmp->next;
	}

	ResetIterator();

	return count;
}

int LinkedList::Clear()
{
	Node *tmp1, *tmp2;
	int count = 0;

	// delete entire list, except start node
	tmp1 = start->next;
	while (tmp1 != NULL)
	{
		tmp2 = tmp1;
		tmp1 = tmp1->next;
		delete tmp2;
		count++;
	}

	// initialize start node
	start->next = NULL;
	start->data = NULL;

	end = start;

	ResetIterator();

	num_elements = 0;

	return count;			// number of deleted items
}

int LinkedList::PrintList(ostream &stream)
{
	Node *tmp1, *tmp2;
	int count = 0;
//#ifdef WIN32
#ifdef MSVC
	long old_base;
#else
	std::_Ios_Fmtflags old_base;
#endif

	// save old format
	stream.setf(ios::showbase);
	old_base = stream.setf(ios::hex, ios::basefield);

	// traverse entire list, except start node
	tmp1 = start->next;
	while (tmp1 != NULL)
	{
		tmp2 = tmp1;
		tmp1 = tmp1->next;
		stream << tmp2->data << endl;
		count++;
	}

	// reset output format
	stream.setf(old_base, ios::showbase);

	return count;			// number of deleted items
}

