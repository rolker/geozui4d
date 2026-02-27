/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// SmartMemory.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is the basic infrastructure for structures that enable passing values
 *	back and forth that may be of several different types.
 *
 *	Created by mdp 3/11/2003
 */
/*---------------------------------------------------------------------------*/
#include "SmartMemory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *SmartMemory::indexName[NumTypes] =
{"Invalid", "Integer", "Float", "String", "Pointer", "Bool"};

const char *SmartMemory::format[NumTypes] =
{"%x", "%d", "%f", "%s", "@%x", "%i"};

char SmartMemory::returnValue[256] = "";

/*---------------------------------------------------------------------------*/
SmartMemory::SmartMemory()
{
	type = Integer;
	isConst = 0;
}

/*---------------------------------------------------------------------------*/
SmartMemory::~SmartMemory()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void SmartMemory::printTo(SmartStorage &ss, char *string)
{
	switch (type & TypeMask)
	{
		case String:
			sprintf(string, formatString(), ss.s);
			break;
		case Float:
			sprintf(string, formatString(), ss.f);
			break;
		case Integer:
			sprintf(string, formatString(), ss.i);
			break;
		case sBool:
			sprintf(string, formatString(), ss.b);
		case Pointer: default:
			sprintf(string, formatString(), (unsigned long)(ss.p));
			break;
	}
}

/*---------------------------------------------------------------------------*/
const char *SmartMemory::asString(SmartStorage &ss)
{
	int baseType = type & TypeMask;

	if (baseType >= NumTypes)
		return NULL;
	//format[255] = '\0'; Array index out of range!!!
	switch (baseType)
	{
		case String:
			if (isConst || !ss.s)
				return ss.s;
			else
				strncpy(returnValue, ss.s, 255);
			break;
		case Float:
			sprintf(returnValue, formatString(), ss.f);
			break;
		case Integer:
			sprintf(returnValue, formatString(), ss.i);
			break;
		case sBool:
			sprintf(returnValue, formatString(), ss.b);
		case Pointer:
			sprintf(returnValue, formatString(), (unsigned long)(ss.p));
			break;
		default:
			return NULL;
			break;
	}
	return returnValue;
}

/*---------------------------------------------------------------------------*/
int SmartMemory::asInteger(SmartStorage &ss)
{
	switch (type & TypeMask)
	{
		case String:
			return atoi(ss.s);
			break;
		case Float:
			return (int)ss.f;
			break;
		case Integer:
			return ss.i;
			break;
		case sBool:
			return (int)ss.b;
			break;
		case Pointer:
			return (unsigned long)ss.p;
			break;
		default:;
	}
	return 0;
}

bool SmartMemory::asBool(SmartStorage& ss)
{
	if (ss.i)
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
float SmartMemory::asFloat(SmartStorage &ss)
{
	switch (type & TypeMask)
	{
		case String:
			return (float) atof(ss.s);
			break;
		case Float:
			return ss.f;
			break;
		case Integer:
			return (float)ss.i;
			break;
		case sBool:
			return (float)ss.b;
			break;
		case Pointer:
			return (float)(unsigned long)ss.p;
			break;
		default:;
	}
	return 0.0;
}

/*---------------------------------------------------------------------------*/
void *SmartMemory::asPointer(SmartStorage &ss)
{
	if ((type & TypeMask) == Pointer)
		return (void *)ss.p;
	else if (type == String && ss.s[0] == '@')
		return (void *)atoi(&(ss.s[1]));
	else
		return NULL;
}

