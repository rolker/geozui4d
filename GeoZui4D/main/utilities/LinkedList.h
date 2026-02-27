/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// LinkedList.h
/** 
 *	Class for maintaining a linked list of pointers to externally instantiated
 *  objects. 
 *  \par Purpose:
 *  This class was created to remove reliance upon using the C++ standard template 
 *  library (STL) list template class, mainly
 *  because there is a need to link with 3rd party libraries which use the old
 *  style C headers (especially <iostream.h>) which conflict with the newer STL
 *  libraries. 
 *  \par Use:
 *	Call ResetIterator() before iterating through the list. Use GetNext() to
 *	perform list iteration; you've reached the end (it's assumed) when GetNext()
 *	returns NULL. Do NOT attempt to Remove() or Append() while iterating; the
 *	iterator will be reset to the beginning of the list.
 *  \note Pointers to objects inserted in the LinkedList are saved as type
 *  void*. It is up to the user to recast the returned pointer to the proper
 *  type before referencing it. Deleting the list does not affect (ie. delete)
 *	the objects referenced by the stored void pointers.
 *
 *	\author Colin Ware and Rick Komerska 
 *  \date 5/25/2001
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#error

#include <iostream>

class LinkedList
{
public:
	LinkedList();
	~LinkedList();
	void Append(void * i);		// add a void* data element to the end of the list
	int InsertAfterIndex(int i, void * d);	// insert void* d after index i in list, return updated number of
											//  elements or -1 if insertion out of bounds
											//  if i=0, insert at start
	int InsertAfterFirstElement(const void * e, void * d); // insert void* d after first occurance of void* e
											// in list, return index of e or -1 if e not found
											//  if e=NULL, insert at start (returned index will = 0)
	int InsertBeforeFirstElement(const void * e, void * d);
	int RemoveFirst(void * i);	// remove the first occurance of the matching void* from the list,
								//  return the index of the deleted item, or -1 if not found
	int  RemoveAll(void * i);	// remove all occurances of the matching void* from the list
	void ResetIterator();		// reset the iterator, do this before iterating
	void * GetNext(int increment = 1);	// get the next void* data element in the list
	void * GetElementAtIndex(int i);	// ones-based index to retrieve void* element from list
	int GetIndexOfFirstElement(const void * i);	// retrieve ones-based index to void* element int list (-1 if not found)
	int Clear();				// delete all the data elements and start the list fresh
	int Size() const;			// return the number of stored void* elements

	int PrintList(std::ostream &stream);	// print the list of void* as hex, useful for debugging

private:

	struct Node
	{
		Node * next;
		void * data;
	};

	Node * start;
	Node * end;
	Node * iterator;

	int num_elements;
};

#endif
