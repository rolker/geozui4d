// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"DXF_thing.h"
#include<string.h>
#include"engine/Persistence.h"

DXF_thing::DXF_thing()
{
	handle = 0;
}

DXF_thing::~DXF_thing()
{
}

int DXF_thing::processEntry()
{
	
	switch(groupCode)
	{
	case 2:
		name = valueString;
		return 1;
	case 5:
		setHandle(valueString);
		return 1;
	case 8:
		layer=valueString;
		return 1;
	default:
		return 0;
	}
}

void DXF_thing::setHandle(char * h)
{
	sscanf(h,"%x",&handle);
}

void DXF_thing::setHandle(int h)
{
	handle = h;
}

