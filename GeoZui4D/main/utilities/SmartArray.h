// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SmartArray.h
/*---------------------------------------------------------------------------*/

class SmartArray;

#ifndef SMARTARRAY
#define SMARTARRAY

#include "SmartMemory.h"
#include "utilities/SmartValue.h"
/** 
 *	This is a structure for storing arrays of values that can be passed back
 *	and forth, where the array stores all the same type, but what it holds may
 *	be one of several different types.  The smart array does not change its
 *	type upon set, as a SmartMemory normally would.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 3/11/2003
 */
/*---------------------------------------------------------------------------*/
class SmartArray : public SmartMemory
{
  public:
	SmartArray(int size, IndexType arrayType, int constDesignation = 0);
	SmartArray();
	~SmartArray();

	void init(int size, IndexType arrayType, int constDesignation = 0);
	void clear();

	union SmartArrayStorage
	{
		int *i;
		float *f;
		char **s;
		const void **p;
	} array;
	SmartValue value;
	int size, allocatedSize;

	//accessors
	void printTo(char *string) {if (currentIndex >= 0) SmartMemory::printTo(value.value, string);}

	const char *asString() {return (currentIndex >= 0)? SmartMemory::asString(value.value): (const char *)0;}
	int asInteger() {return (currentIndex >= 0)? SmartMemory::asInteger(value.value): 0;}
	float asFloat() {return (currentIndex >= 0)? SmartMemory::asFloat(value.value): 0.0f;}
	void *asPointer() {return (currentIndex >= 0)? SmartMemory::asPointer(value.value): (void *)0;}

	//mutators
	SmartArray &operator [](int i);
	void set(int val);
	void set(float val);
	void set(char* val, int constFlag = 0);
	void set(const char* val);
	void set(const void* val);

	void insertPosition(int index);
	void deletePosition(int index);

  protected:
	int currentIndex;
	void set();
};

#endif
