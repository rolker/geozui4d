// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SmartValue.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This is a structure for passing values back and forth that may be of
 *	several different types.
 *
 *	Created by mdp 3/5/2003
 */
/*---------------------------------------------------------------------------*/
#include "utilities/SmartValue.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
SmartValue::SmartValue()
{
	value.i = 0;
}

/*---------------------------------------------------------------------------*/
SmartValue::~SmartValue()
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void SmartValue::set(int val)
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;

	type = Integer;
	value.i = val;
}

/*---------------------------------------------------------------------------*/
void SmartValue::set(float val)
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;

	type = Float;
	value.f = val;
}

/*---------------------------------------------------------------------------*/
void SmartValue::set(char* val, int constFlag)
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;

	type = String;
	isConst = constFlag;
	value.s = val;
}

/*---------------------------------------------------------------------------*/
void SmartValue::set(const char* val)
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;

	type = String;
	isConst = 1;
	value.s = (char *)val;
}

/*---------------------------------------------------------------------------*/
void SmartValue::set(const void *val)
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;

	type = Pointer;
	value.p = val;
}

/*---------------------------------------------------------------------------*/
void SmartValue::set(bool val)
{
	if ((type & TypeMask) == String && !isConst)
		delete value.s;

	type = sBool;
	value.b = val;
}
