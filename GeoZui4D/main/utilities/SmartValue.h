// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SmartValue.h
/*---------------------------------------------------------------------------*/

class SmartValue;

#ifndef SMARTVALUE
#define SMARTVALUE

#include "SmartMemory.h"
/** 
 *	This is a structure for passing values back and forth that may be of
 *	several different types.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 3/5/2003
 */
/*---------------------------------------------------------------------------*/
class SmartValue : public SmartMemory
{
  public:
	SmartValue();
	~SmartValue();

	SmartStorage value;

	//accessors
	void printTo(char *string) {SmartMemory::printTo(value, string);}

	const char *asString() {return SmartMemory::asString(value);}
	int asInteger() {return SmartMemory::asInteger(value);}
	float asFloat() {return SmartMemory::asFloat(value);}
	void *asPointer() {return SmartMemory::asPointer(value);}
	bool asBool() {return SmartMemory::asBool(value);}

	//mutators
	void set(int val);
	void set(float val);
	void set(char* val, int constFlag = 0);
	void set(const char* val);
	void set(const void* val);
	void set(bool val);
};

#endif
