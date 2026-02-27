/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"DXF_BLOCK.h"
#include<iostream>
#include"DXF_entity.h"
#include"DXF_INSERT.h"
#include"objects/dxf/DXF.h"
#include <typeinfo>
#include <cstring>

using namespace std;
REGISTER_OBJECT(DXF_BLOCK, "");

DXF_BLOCK::DXF_BLOCK()
{
	base[0] = base[1] = base[2] = 0.0;
	flags = 0;
	dxfManager = NULL;
}

// documented in Object
int DXF_BLOCK::readFrom(FILE * f)
{
	DXF_entity * entity = NULL;
	char s[160];
	string typeName;

	while(groupCode!=0)
	{
		if(!processEntry())
		{
			switch(groupCode)
			{
			case 10:
				base[0] = atof(valueString);
				break;
			case 20:
				base[1] = atof(valueString);
				break;
			case 30:
				base[2] = atof(valueString);
				break;
			case 70:
				flags = atoi(valueString);
				break;
			default:
			break;
			}
		}
		readNext();
	}
	while(strcmp(valueString,"ENDBLK"))
	{
		strcpy(s,valueString);
		entity = processEntity();
		if(entity)
		{
			GET_CLASS_NAME(entity,typeName);
			if(dxfManager&& typeName == "DXF_INSERT")
				((DXF_INSERT*)entity)->setBlock(dxfManager->getBlock(entity->getName()));
			addObject(entity);
		}
	}
	readNext();
	while(groupCode!=0)
		readNext();
	
	
	return 0;
}
