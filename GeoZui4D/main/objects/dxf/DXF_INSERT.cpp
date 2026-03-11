// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#include"DXF_INSERT.h"
#include"DXF_BLOCK.h"
#include<iostream>

using namespace std;

DXF_INSERT::DXF_INSERT()
{
	reference[0] = reference[1] = reference[2] = 0.0;
	scale[0] = scale[1] = scale[2] = 1.0;
	block = NULL;
	rotation = 0.0;
}

// documented in Object
int DXF_INSERT::readFrom(FILE * file)
{
	readNext();
	while(groupCode!=0)
	{
		if(!processEntry())
		{
			switch(groupCode)
			{
			case 10:
				reference[0]=atof(valueString);
				break;
			case 20:
				reference[1]=atof(valueString);
				break;
			case 30:
				reference[2]=atof(valueString);
				break;
			case 41:
				scale[0]=atof(valueString);
				break;
			case 42:
				scale[1]=atof(valueString);
				break;
			case 43:
				scale[2]=atof(valueString);
				break;
			case 50:
				rotation = atof(valueString);
				break;
			default:
				break;
			}
		}
		readNext();
	}
	return 0;
}


void DXF_INSERT::Draw(float degradation, Window3D *win)
{
	if(block)
	{
        glPushMatrix();
		gotoOCS();
		glTranslatef(reference[0],reference[1],reference[2]);
		if(clockwise)
			glRotated(-rotation,0.0,0.0,1.0);
		else
			glRotated(rotation,0.0,0.0,1.0);
		glScalef(scale[0],scale[1],scale[2]);
		block->DrawLowRes(degradation,NULL);
        glPopMatrix();
	}
	
}

void DXF_INSERT::setBlock(DXF_BLOCK * b)
{
	block = b;
}


