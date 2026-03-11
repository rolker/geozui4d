// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SmartMemory.h
/*---------------------------------------------------------------------------*/

class SmartMemory;

#ifndef SMARTMEMORY
#define SMARTMEMORY
/** 
 *	This is the basic infrastructure for structures that enable passing values
 *	back and forth that may be of several different types.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 3/11/2003
 */
/*---------------------------------------------------------------------------*/
class SmartMemory
{
  public:
	enum IndexType {Invalid, Integer, Float, String, Pointer, sBool, NumTypes, TypeMask = 0xFF};

	SmartMemory();
	virtual ~SmartMemory();

	union SmartStorage
	{
		int i;
		float f;
		char *s;
		const void *p;
		bool b;
	};
	IndexType type;
	int isConst;

	//accessors
	const char *typeName() {return (type > Invalid && type < NumTypes)? indexName[type] : indexName[Invalid];}
	const char *formatString() {return (type > Invalid && type < NumTypes)? format[type] : format[Invalid];}

	static const char *typeName(IndexType type) {return (type > Invalid && type < NumTypes)? indexName[type] : indexName[Invalid];}

  protected:
	void printTo(SmartStorage &ss, char *string);

	const char *asString(SmartStorage &ss);
	int asInteger(SmartStorage &ss);
	bool asBool(SmartStorage &ss);
	float asFloat(SmartStorage &ss);
	void *asPointer(SmartStorage &ss);

	static const char *indexName[NumTypes];
	static const char *format[NumTypes];
	static char returnValue[256];
};

#endif
