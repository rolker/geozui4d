/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"objects/dxf/DXF.h"
#include<iostream>
#include"DXF_layer.h"
#include"DXF_BLOCK.h"
#include"DXF_INSERT.h"
#include "engine/ObjectPoseState.h"
#include<boost/filesystem.hpp>
#include"utilities/DottedAddress.h"
#include <typeinfo>
#include <cstring>
#include"DXF_entity.h"

using namespace std;

REGISTER_OBJECT(DXF, "dxf");

DXF::DXF()
:ObjectCollection(GL_FALSE)
{
	SetTypeAndVersion("");
	blocks = NULL;
	r=g=b=1.0;
	useColor = 1;
	
	ObjectPoseState * op = new ObjectPoseState(this);
	pose = op;
	
	op->ignoreHeightExaggeration();
	
	nullLayer = new DXF_layer;
	nullLayer->Object::SetLabel("null layer");
	AddObject(nullLayer);
	binaryFile = true;
}


// documented in Object
int DXF::readFrom(FILE *infile)
{
	SetPosition(0.0,0.0,0.0);
	dxf = infile;

	int done = readNext();
	
	while(!done)
	{
		if(groupCode==0)
		{
			if(strcmp(valueString,"SECTION")==0)
			{
				done = readNext();
				if(groupCode==2)
				{
					if(strcmp(valueString,"HEADER")==0)
					{
						processHeader();
					}
					else if(strcmp(valueString,"CLASSES")==0)
					{
						processClasses();
					}
					else if(strcmp(valueString,"TABLES")==0)
					{
						processTables();
					}
					else if(strcmp(valueString,"BLOCKS")==0)
					{
						processBlocks();
					}
					else if(strcmp(valueString,"ENTITIES")==0)
					{
						processEntities();
					}
					else
					{
						done = readNext();
					}
				}
			}
			else
				done = readNext();
		}
		else
			done = readNext();
	}

		
	return 0;
}

void DXF::processHeader()
{
	int done = 0;
	while(!done)
	{
		switch(groupCode)
		{
		case 0:
			if(strcmp(valueString,"ENDSEC")==0)
				done = 1;
			readNext();
			break;
		case 9:
			if(strcmp(valueString,"$EXTMIN")==0)
			{
				readNext();
				while(groupCode!=9)
				{
					if(groupCode==10)
					{
						if(!localBBox)
							localBBox = new BBox;
						localBBox->w = atof(valueString);
					}
					if(groupCode==20)
					{
						if(!localBBox)
							localBBox = new BBox;
						localBBox->s = atof(valueString);
					}
					if(groupCode==30)
					{
						if(!localBBox)
							localBBox = new BBox;
						localBBox->b = atof(valueString);
					}
					readNext();
				}
			}
			else if(strcmp(valueString,"$EXTMAX")==0)
			{
				readNext();
				while(groupCode!=9)
				{
					if(groupCode==10)
					{
						if(!localBBox)
							localBBox = new BBox;
						localBBox->e = atof(valueString);
					}
					if(groupCode==20)
					{
						if(!localBBox)
							localBBox = new BBox;
						localBBox->n = atof(valueString);
					}
					if(groupCode==30)
					{
						if(!localBBox)
							localBBox = new BBox;
						localBBox->t = atof(valueString);
					}
					readNext();
				}
			}
			else if(strcmp(valueString,"$ANGBASE")==0)
			{
				readNext();
				angbase = atof(valueString);
				readNext();
			}
			else if(strcmp(valueString,"$ANGDIR")==0)
			{
				readNext();
				clockwise = atoi(valueString);
				readNext();
			}
			else
				readNext();
			break;
		default:
			readNext();
		}
	}
}

void DXF::processClasses()
{
	int done = 0;
	while(!done)
	{
		switch(groupCode)
		{
		case 0:
			if(strcmp(valueString,"ENDSEC")==0)
			{
				done = 1;
			}
			readNext();
			break;
		default:
			readNext();
		}
	}
}

void DXF::processTables()
{
	DXF_layer * layer;
	DXF_BLOCK * block;
	BlockNode * bn;
	
	int done = 0;
	while(!done)
	{
		switch(groupCode)
		{
		case 0:
			if(strcmp(valueString,"ENDSEC")==0)
			{
				done = 1;
			}
			else if(strcmp(valueString,"TABLE")==0)
			{
				readNext();
				if(groupCode==2)
				{
					if(strcmp(valueString,"LAYER")==0)
					{
						readNext();
						while(groupCode!=0)
							readNext();
						while(strcmp(valueString,"LAYER")==0)
						{
							readNext();
							layer = new DXF_layer;
							layer->readFrom();
							AddObject(layer);
						}
					}
					else if(strcmp(valueString,"BLOCK_RECORD")==0)
					{
						while(strcmp(valueString,"BLOCK_RECORD")==0)
						{
							readNext();
							block = new DXF_BLOCK;
							while(groupCode!=0)
							{
								block->processEntry();
								readNext();
							}
							bn = new BlockNode;
							bn->block = block;
							bn->next = blocks;
							blocks = bn;
						}
					}
					else
					{
						readNext();
					}
				}
			}
			else
				readNext();
			break;
		default:
			readNext();
		}
	}
}

void DXF::processBlocks()
{
	DXF_BLOCK * block;
	BlockNode * bn;

	int done = 0;
	while(!done)
	{
		switch(groupCode)
		{
		case 0:
			if(strcmp(valueString,"ENDSEC")==0)
			{
				done = 1;
			}
			else if(strcmp(valueString,"BLOCK")==0)
			{
				while(strcmp(valueString,"BLOCK")==0)
				{
					// first, find the block's name
					savePosition();
					readNext();
					while(groupCode!=2)
						readNext();
					block = getBlock(valueString);
					if(!block)
					{
						block = new DXF_BLOCK;
						bn = new BlockNode;
						bn->block = block;
						bn->next = blocks;
						blocks = bn;
					}
					block->setDXF(this);
					restorePosition();
					readNext();
					block->readFrom();
				}
			}
			else
			{
				readNext();
			}
			break;
		default:
			readNext();
		}
	}
}

void DXF::processEntities()
{
	DXF_entity * entity = NULL;
	string typeName;
	
	
	int done = 0;
	while(!done)
	{
		switch(groupCode)
		{
		case 0:
			if(strcmp(valueString,"ENDSEC")==0)
			{
				done = 1;
			}
			else
			{
				entity = processEntity();
				if(entity)
				{
					GET_CLASS_NAME(entity,typeName);
					if(typeName == "DXF_INSERT")
						((DXF_INSERT*)entity)->setBlock(getBlock(entity->getName()));
					
					if(!entity->getLayer().empty()&&GetObjectPtr(entity->getLayer()))
						((DXF_layer*)(GetObjectPtr(entity->getLayer())))->addObject(entity);
					else
						nullLayer->addObject(entity);
				}
				
			}
			break;
		default:
			readNext();
		}
	}
}

DXF_BLOCK * DXF::getBlock(std::string const &n)
{
	BlockNode * bn;
	bn=blocks;
	while(bn)
	{
		if(bn->block->getName() == n)
			return bn->block;
		bn = bn->next;
	}
	return NULL;
}

void DXF::DrawLowRes(float degradation, Window3D *win)
{
	if(useColor)
		glColor3f(r,g,b);
	ObjectCollection::DrawLowRes(degradation,win);
}

int DXF::DrawHighRes(int eye, Window3D *win)
{
    if(useColor)
	glColor3f(r,g,b);
	return ObjectCollection::DrawHighRes(eye,win);
}

void DXF::setColor(float red, float green, float blue)
{
	r = red;
	g = green;
	b = blue;
}

void DXF::dontUseColor()
{
	useColor = 0;
}

