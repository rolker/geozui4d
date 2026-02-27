/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// SmartArray.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a structure for storing arrays of values that can be passed back
 *	and forth, where the array stores all the same type, but what it holds may
 *	be one of several different types.  The smart array does not change its
 *	type upon set, as a SmartMemory normally would.
 *
 *	Created by mdp 3/11/2003
 */
/*---------------------------------------------------------------------------*/
#include "utilities/SmartArray.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
SmartArray::SmartArray()
{
	array.i = NULL;
}

/*---------------------------------------------------------------------------*/
SmartArray::SmartArray(int size, IndexType arrayType, int constDesignation)
{
	init(size, arrayType, constDesignation);
}

/*---------------------------------------------------------------------------*/
void SmartArray::init(int sz, IndexType arrayType, int constDesignation)
{
	int i;

	allocatedSize = size = sz;
	switch (arrayType & TypeMask)
	{
		case Integer:
			array.i = new int[size];
			break;
		case Float:
			array.f = new float[size];
			break;
		case String:
			array.s = new char*[size];
			break;
		case Pointer:
			array.p = new const void*[size];
			break;
    default:;
	}
	isConst = constDesignation;
	type = arrayType;
	if ((type & TypeMask) == String && !isConst)
	{
		for (i = 0; i < size; i++)
			array.s[i] = NULL;
	}
}

/*---------------------------------------------------------------------------*/
SmartArray::~SmartArray()
{
	int i;

	if ((type & TypeMask) == String && !isConst)
	{
		for (i = 0; i < size; i++)
			delete array.s[i];
	}
	switch (type & TypeMask)
	{
		case Integer:
			delete [] array.i;
			break;
		case Float:
			delete [] array.f;
			break;
		case String:
			delete [] array.s;
			break;
		case Pointer:
			delete [] array.p;
			break;
    default:;
	}
}

/*---------------------------------------------------------------------------*/
void SmartArray::clear()
{
	int i;

	if ((type & TypeMask) == String && !isConst)
	{
		for (i = 0; i < size; i++)
			delete array.s[i];
	}
	switch (type & TypeMask)
	{
		case Integer:
			delete [] array.i;
			break;
		case Float:
			delete [] array.f;
			break;
		case String:
			delete [] array.s;
			break;
		case Pointer:
			delete [] array.p;
			break;
		default:;
	}
	size = allocatedSize = 0;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
SmartArray &SmartArray::operator [](int i)
{
	if (i < size)
		currentIndex = i;
	else
		currentIndex = -1;

	if (currentIndex >= 0)
	{
		switch (type & TypeMask)
		{
			case Integer:
				value.set(array.i[currentIndex]);
				break;
			case Float:
				value.set(array.f[currentIndex]);
				break;
			case String:
				value.set(array.s[currentIndex], 1);
				break;
			case Pointer:
				value.set(array.p[currentIndex]);
				break;
			default:
				currentIndex = -1;
				break;
		}
	}
	return *this;
}

/*---------------------------------------------------------------------------*/
void SmartArray::set(int val)
{
	value.set(val);
	set();
}

/*---------------------------------------------------------------------------*/
void SmartArray::set(float val)
{
	value.set(val);
	set();
}

/*---------------------------------------------------------------------------*/
void SmartArray::set(char* val, int constFlag)
{
	value.set(val, constFlag);
	set();
}

/*---------------------------------------------------------------------------*/
void SmartArray::set(const char* val)
{
	value.set(val);
	set();
}

/*---------------------------------------------------------------------------*/
void SmartArray::set(const void *val)
{
	value.set(val);
	set();
}

/*---------------------------------------------------------------------------*/
void SmartArray::set()
{
	if (currentIndex < 0 || currentIndex >= size)
		return;

	switch (type & TypeMask)
	{
		case Integer:
			array.i[currentIndex] = value.asInteger();
			break;
		case Float:
			array.f[currentIndex] = value.asFloat();
			break;
		case String:
			if (isConst) //caution: potential memory leak if strings are coming in not const!
				array.s[currentIndex] = (char *)value.asString();
			else
			{
				delete array.s[currentIndex];
				if (value.isConst)
					array.s[currentIndex] = strdup(value.asString());
				else
				{
					array.s[currentIndex] = value.value.s;
					value.isConst = 1; //don't let it delete it later!
				}
			}
			break;
		case Pointer:
			array.p[currentIndex] = value.asPointer();
			break;
		default:;
	}
}

/*---------------------------------------------------------------------------*/
#define EMPTY()
#define COPY_RECORD(source, sourceIndex, target, targetIndex) \
{\
	switch (type & TypeMask)\
	{\
		case Integer:\
			target##array.i[targetIndex] = source##array.i[sourceIndex];\
			break;\
		case Float:\
			target##array.f[targetIndex] = source##array.f[sourceIndex];\
			break;\
		case String:\
			target##array.s[targetIndex] = source##array.s[sourceIndex];\
			break;\
		case Pointer:\
			target##array.p[targetIndex] = source##array.p[sourceIndex];\
			break;\
		default:;\
	}\
}

/*---------------------------------------------------------------------------*/
void SmartArray::insertPosition(int index)
{
	int i, i1, i2;
	float c1, c2;
	int newLength, insertPoint;
	int baseType;
	SmartArrayStorage new_array;

	if (index < size)
	{
		newLength = size + 1;
		insertPoint = index;
	}
	else
	{
		newLength = index + 1;
		insertPoint = size;
	}
	baseType = type & TypeMask;

	//create new array
	if (newLength > allocatedSize)
	{
		switch (baseType)
		{
			case Integer:
				new_array.i = new int[newLength];
				break;
			case Float:
				new_array.f = new float[newLength];
				break;
			case String:
				new_array.s = new char*[newLength];
				break;
			case Pointer:
				new_array.p = new const void*[newLength];
				break;
			default:;
		}

		for (i = 0; i < insertPoint; i++)
//#ifdef WIN32
#ifdef MSVC
			COPY_RECORD(EMPTY(), i, new_, i);
#else
			COPY_RECORD(, i, new_, i);
#endif
	}
	else //following works unless pointer sizes differ!
		new_array.p = array.p;

	//copy everything past insertion point
	for (i = size; i > insertPoint; i--)
//#ifdef WIN32
#ifdef MSVC
		COPY_RECORD(EMPTY(), i - 1, new_, i);
#else
		COPY_RECORD(, i - 1, new_, i);
#endif

	//insertion in middle or at end
	if (baseType == Integer || baseType == Float)
	{
		if (insertPoint >= size) //project last two points out
		{
			i1 = insertPoint - 2;
			i2 = insertPoint - 1;
			c1 = -1;
			c2 = 2;
		}
		else //find midpoint
		{
			if (insertPoint >= 1)
			{
				i1 = insertPoint - 1;
				i2 = insertPoint + 1;
				c1 = .5f;
				c2 = .5f;
			}
			else
			{
				i1 = insertPoint + 2;
				i2 = insertPoint + 1;
				c1 = -1;
				c2 = 2;
			}
		}
		if (i1 < 0) //oops, not enough points!
			new_array.f[insertPoint] = 0.0f;
		else //calculate the new point
		{
			if (baseType == Integer)
				new_array.i[insertPoint] = (int)(c1 * new_array.i[i1] + c2 * new_array.i[i2]);
			else
				new_array.f[insertPoint] = c1 * new_array.f[i1] + c2 * new_array.f[i2];
		}
	}
	else
		new_array.p[insertPoint] = NULL;

	//insertion past end
	for (i = size + 1; i < newLength; i++)
	{
		if (baseType == Integer || baseType == Float)
		{
			i1 = i - 2;
			i2 = i - 1;
			if (i1 < 0) //oops, not enough points!
				new_array.f[insertPoint] = 0.0f;
			else //calculate the new point
			{
				if (baseType == Integer)
					new_array.i[insertPoint] = -new_array.i[i1] + 2 * new_array.i[i2];
				else
					new_array.f[insertPoint] = -new_array.f[i1] + 2 * new_array.f[i2];
			}
		}
		else
			new_array.p[i] = new_array.p[insertPoint];
	}
	//reassign & clean up
	if (newLength > allocatedSize)
	{
		switch (baseType)
		{
			case Integer:
				delete [] array.i;
				break;
			case Float:
				delete [] array.f;
				break;
			case String:
				delete [] array.s;
				break;
			case Pointer:
				delete [] array.p;
				break;
			default:;
		}
		array.p = new_array.p; //again, can do this because they're pointers
	}

	size = newLength;
	if (allocatedSize < size)
		allocatedSize = size;
}

/*---------------------------------------------------------------------------*/
void SmartArray::deletePosition(int index)
{
	int i;

	if ((type & TypeMask) == String && !isConst)
		 delete array.s[index];

	for (i = index + 1; i < size; i++)
//#ifdef WIN32
#ifdef MSVC
		COPY_RECORD(EMPTY(), i, EMPTY(), i - 1);
#else
		COPY_RECORD(, i,, i - 1);
#endif

	size--;
}
