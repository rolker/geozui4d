/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"DXF_reader.h"
#include<stdlib.h>
#include<string.h>
#include<iostream>

#include"DXF_LINE.h"
#include"DXF_ARC.h"
#include"DXF_POLYLINE.h"
#include"DXF_3DFACE.h"
#include"DXF_INSERT.h"
#include"DXF_LWPOLYLINE.h"

using namespace std;

char DXF_reader::valueString[260];
int DXF_reader::groupCode;
FILE * DXF_reader::dxf=NULL;
map<string,bool> DXF_reader::warnings;

float DXF_reader::red[256]={1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f,
            0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f,
            0.15f, 0.75f, 0.875f, 0.4875f, 0.56875f, 0.375f, 0.4375f, 0.225f, 0.2625f, 0.1125f, 0.13125f, 0.5f, 0.75f, 0.325f, 0.4875f, 0.25f, 0.375f, 0.15f, 0.225f, 0.075f, 0.1125f, 0.25f, 0.625f, 0.1625f, 0.40625f, 0.125f, 0.3125f, 0.075f, 0.1875f, 0.0375f, 0.09375f, 0,
            0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0,
            0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0,
            0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0.25f, 0.625f, 0.1625f, 0.40625f, 0.125f, 0.3125f, 0.075f,
            0.1875f, 0.0375f, 0.09375f, 0.5f, 0.75f, 0.325f, 0.4875f, 0.25f, 0.375f, 0.15f, 0.225f, 0.075f, 0.1125f, 0.75f, 0.875f, 0.4875f, 0.56875f, 0.375f, 0.4375f, 0.225f, 0.2625f, 0.1125f, 0.13125f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f,
            0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 0.33f,
            0.464f, 0.598f, 0.732f, 0.866f, 1, 0};

float DXF_reader::green[256]={0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0.25f, 0.625f, 0.1625f, 0.40625f, 0.125f, 0.3125f, 0.075f,
                0.1875f, 0.0375f, 0.09375f, 0.5f, 0.75f, 0.325f, 0.4875f, 0.25f, 0.375f, 0.15f, 0.225f, 0.075f, 0.1125f, 0.75f, 0.875f, 0.4875f, 0.56875f, 0.375f, 0.4375f, 0.225f, 0.2625f, 0.1125f, 0.13125f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f,
                0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1,
                1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f,
                0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 0.75f, 0.875f, 0.4875f, 0.56875f, 0.375f, 0.4375f, 0.225f, 0.2625f, 0.1125f, 0.13125f, 0.5f, 0.75f, 0.325f, 0.4875f, 0.25f,
                0.375f, 0.15f, 0.225f, 0.075f, 0.1125f, 0.25f, 0.625f, 0.1625f, 0.40625f, 0.125f, 0.3125f, 0.075f, 0.1875f, 0.0375f, 0.09375f, 0, 0.5, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0,
                0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0,
                0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0.33f,
                0.464f, 0.598f, 0.732f, 0.866f, 1, 0};

float DXF_reader::blue[256]={0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0,
            0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0,
            0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0, 0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0,
            0.5f, 0, 0.325f, 0, 0.25f, 0, 0.15f, 0, 0.075f, 0.25f, 0.625f, 0.1625f, 0.40625f, 0.125f, 0.3125f, 0.075f, 0.1875f, 0.0375f, 0.09375f, 0.5f, 0.75f, 0.325f, 0.4875f, 0.25f, 0.375f, 0.15f, 0.225f, 0.075f, 0.1125f, 0.75f, 0.875f, 0.4875f,
            0.56875f, 0.375f, 0.4375f, 0.225f, 0.2625f, 0.1125f, 0.13125f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f,
            0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f,
            0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f, 0.15f, 1, 1, 0.65f, 0.65f, 0.5f, 0.5f, 0.3f, 0.3f, 0.15f,
            0.15f, 0.75f, 0.875f, 0.4875f, 0.56875f, 0.375f, 0.4375f, 0.225f, 0.2625f, 0.1125f, 0.13125f, 0.5f, 0.75f, 0.325f, 0.4875f, 0.25f, 0.375f, 0.15f, 0.225f, 0.075f, 0.1125f, 0.25f, 0.625f, 0.1625f, 0.40625f, 0.125f, 0.3125f, 0.075f, 0.1875f, 0.0375f, 0.09375f, 0.33f,
            0.464f, 0.598f, 0.732f, 0.866f, 1, 0};

float DXF_reader::angbase=0.0;
int DXF_reader::clockwise = 0;

DXF_reader::DXF_reader()
{
}

int DXF_reader::readNext()
{
	char tmp[260];

	if(!fgets(tmp,260,dxf))
		return 1;
	groupCode = atoi(tmp);

	if(!fgets(tmp,260,dxf))
		return 1;
	sscanf(tmp,"%s",valueString);

	return 0;
}

void DXF_reader::savePosition()
{
	fileposition = ftell(dxf);
}

void DXF_reader::restorePosition()
{
	fseek(dxf,fileposition,SEEK_SET);
}

DXF_entity* DXF_reader::processEntity()
{
	DXF_entity * entity = NULL;
	
	if(strcmp(valueString,"LINE")==0)
	{
		entity = new DXF_LINE;
	}
	else if(strcmp(valueString,"ARC")==0)
	{
		entity = new DXF_ARC;
	}
	else if(strcmp(valueString,"POLYLINE")==0)
	{
		entity = new DXF_POLYLINE;
	}
	else if(strcmp(valueString,"3DFACE")==0)
	{
		entity = new DXF_3DFACE;
	}
	else if(strcmp(valueString,"INSERT")==0)
	{
		entity = new DXF_INSERT;
	}
	else if(strcmp(valueString,"LWPOLYLINE")==0)
	{
		entity = new DXF_LWPOLYLINE;
	}
	else
	{
		map<string,bool>::iterator i = warnings.find(valueString);
		if(i == warnings.end())
		{
			std::cerr << valueString << " not supported." << std::endl;
			warnings[valueString] = true;
		}
	}

	if(entity)
	{
		entity->readFrom();
	}
	else
	{
		while(!readNext() && groupCode !=0){}
	}
	
	return entity;
}

