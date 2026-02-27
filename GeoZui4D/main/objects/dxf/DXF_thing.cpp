/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

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

