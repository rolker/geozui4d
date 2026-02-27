/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// DataTriangles.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing triangle strips, with attributes
 *	associated with the strips or component points (temperature, for
 *	instance) mapped to attributes of the same (color, for instance).
 *	This class implements the following responsibilities:
 *		* Read source data from a file to generate triangle strips
 *		* Support viewing of the strips.
 *
 *	Created by mdp 6/8/2001
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
#include "DataTriangles.h"
#include "widgets/Sv_Standards.h"
#include "objects/GeoData/Glyph.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/PosableStateBase.h"
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/Color.h>

using namespace std;

extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(DataTriangles, "gdt");

#define STRIPS_AT_A_TIME 5
#define ZERO_TOLERANCE 0.0001

const char *DataTriangles::DataTrianglesNames[] =
			{GEODATA_NAMES, "FILL", "NORMALS", "SCALE_Z",
			 "RIGHT_HANDED", "LEFT_HANDED", "UP", "DOWN", "FAN",
			 ""};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
DataTriangles::DataTriangles()
: GeoData(NumTriangleAttributes, "DATA_TRIANGLES", DataTrianglesNames)
{
	triangleRecords = NULL;
	displayFill = ON;
	degradationFlag = OFF;
	normalsRule = RIGHT_HANDED;
    triangleFan = OFF;
	normal_z_factor = 1.0;
	numDimensions = 2;
}

/*--------------------------------------------------------------------------*/
DataTriangles::~DataTriangles()
{
    unsigned int i;

	if (triangleRecords)
	{
		for (i = 0; i < numRecords; i++)
			delete triangleRecords[i];
		delete triangleRecords;
	}
}

/*--------------------------------------------------------------------------*/
DataTriangles::TriangleRecord::TriangleRecord(int points, int colorFormat)
{
	int i, numColors;

	numPoints = points;
	positions.resize(numPoints);
	texturePosition = NULL;
	norms = new vNorms[numPoints];
	rgbColor = NULL;
	//bytewiseColor = NULL;
	colorVal = NULL;
	allocatedColors = 0;
	displayFill = ON;
	normalsRule = RIGHT_HANDED;

	//color memory allocation
	if (colorFormat & FORMAT_BY_COLUMN)
		numColors = numPoints;
	else
		numColors = 1;

	if (colorFormat & COLOR_AS_BYTES)
		//bytewiseColor = new const unsigned char*[numColors];
            bytewiseColor.resize(numColors);
	else if (colorFormat & COLOR_TEXTURE)
	{
		if (colorFormat & FORMAT_BY_COLUMN)
			texturePosition = new TextureCoordinate[numPoints];
	}
	else
	{
		rgbColor = new float*[numColors];
		if (colorFormat & COLOR_RGB)
		{
			for (i = 0; i < numColors; i++)
				rgbColor[i] = new Point3f;
			allocatedColors = numColors;
		}
	}
	if (colorFormat & COLOR_ATTRIBUTE)
		colorVal = new float[numColors];
}

/*--------------------------------------------------------------------------*/
DataTriangles::TriangleRecord::~TriangleRecord()
{
    unsigned int i;

	if (texturePosition)
		delete [] texturePosition;
	if (norms)
		delete [] norms;
	if (rgbColor)
	{
		if (allocatedColors)
		{
			for (i = 0; i < allocatedColors; i++)
				delete rgbColor[i];
		}
		delete [] rgbColor;
	}
// 	if (bytewiseColor)
// 		delete [] bytewiseColor;
	if (colorVal)
		delete colorVal;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DataTriangles::DrawLowRes(float degradation, Window3D *)
{
    unsigned int i, gap;

	//determine degradation type
//	if (renderingForPicking) //ObjectCollection takes care of degradation == OFF
//		degradation = 0.0;
//	if (degradation == NTH || degradation == ON)
//		increment = (int)(2.1 / (2.1 - (2 * degradation)));
//	else
	gap = 1;
	
	//draw
	//size
	if (sizeFlags & FORMAT_CONSTANT)
		glLineWidth(sizeFactor);

	//fill
	if (displayFill == OFF || (displayFill == FRAGILE && !renderingForPicking))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (i = 0; i < numRecords; i++)
	{
		if (renderingForPicking)
			glLoadName(i);
		DrawStrip(i, !renderingForPicking, gap);
	}
	glLineWidth(1);
	glColor3f(1.0,1.0,1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	ResetHighRes();
}

/*--------------------------------------------------------------------------*/
int DataTriangles::DrawHighRes(int eye, Window3D *win)
{
    unsigned int j, newCurrentRecord;

	if (!doneFlag)
	{
		if (currentRecord < numRecords)
		{
			//size
			if (sizeFlags & FORMAT_CONSTANT)
				glLineWidth(sizeFactor);

			//fill
			if (displayFill == OFF || displayFill == TRANSIENT)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			//draw STRIPS_AT_A_TIME segments at once
			newCurrentRecord = currentRecord;
			for (j = 0; j < STRIPS_AT_A_TIME; j++) 
			{
				if (newCurrentRecord < numRecords)
					DrawStrip(newCurrentRecord);
				else
					break;
				newCurrentRecord++;
			}
			if (eye == dominantEye || eye == MONO_EYE)
				currentRecord = newCurrentRecord;

			glLineWidth(1);
			glColor3f(1.0,1.0,1.0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
			doneFlag = 1;
	}
	else
		doneFlag = 1;
	
	return doneFlag;
}

/*--------------------------------------------------------------------------*/
void DataTriangles::DrawStrip(int record, GLboolean inLowRes, int gap)
{
	// draw a single shaded triangle strip
	int i;
	TriangleRecord *rec = triangleRecords[record];
	int nPoints = rec->numPoints;
	vNorms *norms = rec->norms;
// 	union
// 	{
// 		float               *f;
// 		//const unsigned char **u;
//                 gz4d::Pixel<gz4d::RGBA8> u;
// 	} color;
        float *color;
        gz4d::Pixel<gz4d::RGBA8> color_p;
        
	std::shared_ptr<TextureMap> currentTexture;

	//color
	if (colorFlags & COLOR_TEXTURE)
	{
		if (!customtextureptr)
			currentTexture = *defaulttextureptr;
		else
			currentTexture = customtextureptr;
		if(colorFlags & FORMAT_BY_COLUMN)
			EnableTexture(TEXTURE_COORD_NONE);
		else
			EnableTexture();
		color = palette[11]; //white
	}
	if (colorFlags & FORMAT_CONSTANT)
	{
		if (colorFlags & COLOR_RGB)
			color = constantColor.rgb;
		else
			color = palette[constantColor.index];
	}
	else if (colorFlags & FORMAT_BY_RECORD)
	{
		if (colorFlags & COLOR_AS_BYTES)
			color_p = rec->bytewiseColor[0];
		else
			color = rec->rgbColor[0];
	}

	//fill
	if (displayFill == PER_RECORD &&
		(rec->displayFill == OFF || (inLowRes && rec->displayFill == FRAGILE)
		|| (!inLowRes && rec->displayFill == TRANSIENT)))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//do triangle strip.
	glShadeModel(GL_SMOOTH);
    if(triangleFan==ON)
    {
        glBegin(GL_TRIANGLE_FAN);
    }
    else
    {
		glBegin(GL_TRIANGLE_STRIP);
    }
	for (i = 0; i < nPoints; i+= gap)
	{
		if (colorFlags & FORMAT_BY_COLUMN)
		{
			if (colorFlags & COLOR_AS_BYTES)
				//glColor4ubv(rec->bytewiseColor[i]);
                        gz4d::glo::Color(rec->bytewiseColor[i]);
			else if (colorFlags & COLOR_TEXTURE)
			{
				if (colorFlags & COLOR_TEXTURE_T)
					glTexCoord2f(rec->texturePosition[i].s, rec->texturePosition[i].t);
				else if (colorFlags & FORMAT_BY_COLUMN)
					currentTexture->SetHeight(rec->texturePosition[i].s);
			}
			else
				glColor3fv(rec->rgbColor[i]);
		}
		else //if (!(colorFlags & TEXTURE))
		{
			if (colorFlags & COLOR_ATTRIBUTE)
				//glColor4ubv(*color.u);
                            gz4d::glo::Color(color_p);
			else
				glColor3fv(color);
		}
		if (!renderingForPicking)
		{
			if (normalsRule == XYZ_COLUMNS)
				//glNormal3f(norms[i].nx, norms[i].ny, norms[i].nz / lastHeightExaggeration);
				glNormal3f(norms[i][X], norms[i][Y], norms[i][Z] / lastHeightExaggeration);
			else
				//glNormal3s(norms[i].nx, norms[i].ny, norms[i].nz);
				glNormal3sv(norms[i]);
		}
                gz4d::glo::Vertex(rec->positions[i]);
	}
	glEnd();
	if (colorFlags & COLOR_TEXTURE)
		DisableTexture();
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void DataTriangles::ResetHighRes()
{
	doneFlag = currentRecord = currentPoint = 0;
}

/*--------------------------------------------------------------------------*/
int DataTriangles::processField(int option, char **fields)
{
	int i = 1, j, nextField;

	switch(option)
	{
		case FILL:
			displayFill = Lookup(fields[i++]);
			break;
		case NORMALS:
			normalsRule = Lookup(fields[i++]);
			if (normalsRule == XYZ_COLUMNS)
			{
				for (j = ATTRIB_X_NORMALS; j <= ATTRIB_Z_NORMALS; j++)
				{
					if (assignColumn(i, j, NORMALS) != 0)
						return 1;
				}
			}
			if (getNumFields() >= i + 2)
			{
				nextField = Lookup(fields[i++]);
				if (nextField == SCALE_Z)
					normal_z_factor = this->ConvertFloat(i);
			}
			break;
		case FAN:
			triangleFan = Lookup(fields[i++]);
			break;
		default:
			return GeoData::processField(option, fields);
			break;
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int DataTriangles::readFrom(FILE *infile)
{
	char **fields;
        unsigned int i, point, numPoints;
	TriangleRecord *rec;
	float xnorm, ynorm, znorm, normnorm;

	if (GeoData::readFrom(infile))
		return 1;

	if ((colorFlags & COLOR_RGB) && !(colorFlags & COLOR_AS_BYTES) && !(colorFlags & COLOR_TEXTURE))
		colorFlags |= COLOR_ALLOCATED;

	std::cout << numRecords << " triangle records" << std::endl;
	//allocate necessary memory.
	triangleRecords = new TriangleRecord *[numRecords];
    if(!triangleRecords)
    {
        cerr << "Error allocating memory for triangles\n";
        return 1;
    }

	// read lines one record at a time
	for (i = 0; i < numRecords; i++)
	{
		fields = NextLine();//GL_FALSE);
		if (fields == NULL)
		{
			numRecords = i;
			cerr << "Short file.  Truncated number of records to "
				 << numRecords << endl;
			break;
		}
		int j = 0;
		if (strcmp(fields[j++], "NEW") != 0)
		{
			numRecords = i;
			cerr << "Expected NEW, but got " << fields[0]
				 << ".  Truncated number of records to "
				 << numRecords << endl;
			break;
		}

		//---pluck out any per-record parameters---
		//number of points
		numPoints = ConvertInt(j);
		if (numPoints <= 0)
		{
			numRecords = i;
			cerr << "Expected a number of points for the current record, "
				 << "but got a nonsense value of " << fields[1]
				 << ".  Truncated number of records to "
				 << numRecords << endl;
			break;
		}
		rec = triangleRecords[i] = new TriangleRecord(numPoints, colorFlags);
		//color
		if (colorFlags & FORMAT_BY_RECORD)
		{
			if (colorFlags & COLOR_ATTRIBUTE)
				rec->colorVal[0] = ConvertFloat(j);
			else if (colorFlags & COLOR_RGB)
				ConvertFloatPoint(rec->rgbColor[0], j);
			else if (colorFlags & COLOR_CODE)
				rec->rgbColor[0] = palette[convertColorCode(fields[j++][0])];
		}
		//fill mode
		if (displayFill == PER_RECORD)
			rec->displayFill = Lookup(fields[j++]);
		if (normalsRule == PER_RECORD)
			rec->normalsRule = Lookup(fields[j++]);
		//size
		if (sizeFlags & FORMAT_BY_RECORD)
			size[i] = ConvertFloat(j);
		//text
		if (textFlags & FORMAT_BY_RECORD)
			text[i] = ReleaseField(j++);

		//---Gather per-point data---
		for (point = 0; point < numPoints; point++)
		{
			fields = NextLine();
			//position
			rec->positions[point][0] = ConvertFloatAt(attribToColumn[ATTRIB_X]);
			rec->positions[point][1] = ConvertFloatAt(attribToColumn[ATTRIB_Y]);
			rec->positions[point][2] = ConvertFloatAt(attribToColumn[ATTRIB_Z]);
			//color
			if (attribToColumn[ATTRIB_R] != -1)
				rec->rgbColor[point][0] = ConvertFloatAt(attribToColumn[ATTRIB_R]);
			if (attribToColumn[ATTRIB_G] != -1)
				rec->rgbColor[point][1] = ConvertFloatAt(attribToColumn[ATTRIB_G]);
			if (attribToColumn[ATTRIB_B] != -1)
				rec->rgbColor[point][2] = ConvertFloatAt(attribToColumn[ATTRIB_B]);
			if (attribToColumn[ATTRIB_COLOR] != -1)
			{
				if (colorFlags & COLOR_CODE)
					rec->rgbColor[point] = palette[convertColorCode(fields[attribToColumn[ATTRIB_COLOR]][0])];
				else
				{
					rec->colorVal[point] = ConvertFloatAt(attribToColumn[ATTRIB_COLOR]);
					if (colorFlags & COLOR_AS_HEIGHT)
						rec->colorVal[point] += (float)GetPosition(2);
					rec->bytewiseColor[point] = GetColorAtValue(rec->colorVal[point]);
				}
			}
			//normals
			if (normalsRule == XYZ_COLUMNS)
			{
				xnorm = ConvertFloatAt(attribToColumn[ATTRIB_X_NORMALS]);
				ynorm = ConvertFloatAt(attribToColumn[ATTRIB_Y_NORMALS]);
				znorm = ConvertFloatAt(attribToColumn[ATTRIB_Z_NORMALS]) * normal_z_factor;
				normnorm = (float) sqrt(xnorm * xnorm + ynorm * ynorm + znorm * znorm);
/*				rec->norms[point].nx = (GLshort)((xnorm / normnorm) * 32767);
				rec->norms[point].ny = (GLshort)((ynorm / normnorm) * 32767);
				rec->norms[point].nz = (GLshort)((znorm / normnorm) * 32767);*/
				rec->norms[point][X] = (GLshort)((xnorm / normnorm) * 32767);
				rec->norms[point][Y] = (GLshort)((ynorm / normnorm) * 32767);
				rec->norms[point][Z] = (GLshort)((znorm / normnorm) * 32767);
			}
			//texture
			if (attribToColumn[ATTRIB_TEXTURE_S] != -1)
				rec->texturePosition[point].s = ConvertFloatAt(attribToColumn[ATTRIB_TEXTURE_S]);
			if (attribToColumn[ATTRIB_TEXTURE_T] != -1)
				rec->texturePosition[point].t = ConvertFloatAt(attribToColumn[ATTRIB_TEXTURE_T]);
		}
		projectXY(rec->positions, GL_FALSE);
	}
	projectionRequired = GL_FALSE; //just to be consistent.
	if (normalsRule != XYZ_COLUMNS)
		CalcAllNormals();
	return(0);
}

/*---------------------------------------------------------------------------*/
void DataTriangles::CalcAllNormals()
{
    unsigned int i;

	for (i = 0; i < numRecords; i++)
		updatePoints(i, 0, triangleRecords[i]->numPoints - 1);
}

/*---------------------------------------------------------------------------*/
void DataTriangles::useNewHeightExaggeration(GLfloat heightEx)
{
	Object::useNewHeightExaggeration(heightEx);
	if (triangleRecords && (normalsRule != XYZ_COLUMNS))
		CalcAllNormals();
}

/*---------------------------------------------------------------------------*/
int DataTriangles::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    unsigned int i;

	const char *message = argv[0];
	
	if (argc >= 1)
	{
		//----- init -----
		if (!strcmp(message, "init"))
		{
			if(numRecords>0)
			{
				return TCL_ERROR;
			}
			if(argc >= 2)
			{
				numRecords=atoi(argv[1]);
			}
			else
			{
				return TCL_ERROR;
			}
			triangleRecords = new TriangleRecord *[numRecords];
			size = new float[numRecords];
			colorFlags = FORMAT_BY_COLUMN|COLOR_RGB;
			normalsRule = RIGHT_HANDED;
			for (i = 0; i < numRecords; i++)
			{
				triangleRecords[i] = new TriangleRecord(4, COLOR_RGB);
				triangleRecords[i]->positions[0][0] = 0.0;
				triangleRecords[i]->positions[0][1] = 0.0;
				triangleRecords[i]->positions[0][2] = 0.0;
				triangleRecords[i]->positions[1][0] = 0.0;
				triangleRecords[i]->positions[1][1] = 0.0;
				triangleRecords[i]->positions[1][2] = 0.0;
				triangleRecords[i]->positions[2][0] = 0.0;
				triangleRecords[i]->positions[2][1] = 0.0;
				triangleRecords[i]->positions[2][2] = 0.0;
				triangleRecords[i]->positions[3][0] = 0.0;
				triangleRecords[i]->positions[3][1] = 0.0;
				triangleRecords[i]->positions[3][2] = 0.0;

				triangleRecords[i]->rgbColor = new float*[4];
				triangleRecords[i]->rgbColor[0] = new Point3f;
				triangleRecords[i]->rgbColor[0][0]=1.0;
				triangleRecords[i]->rgbColor[0][1]=1.0;
				triangleRecords[i]->rgbColor[0][2]=1.0;
				triangleRecords[i]->rgbColor[1] = new Point3f;
				triangleRecords[i]->rgbColor[1][0]=1.0;
				triangleRecords[i]->rgbColor[1][1]=1.0;
				triangleRecords[i]->rgbColor[1][2]=1.0;
				triangleRecords[i]->rgbColor[2] = new Point3f;
				triangleRecords[i]->rgbColor[2][0]=1.0;
				triangleRecords[i]->rgbColor[2][1]=1.0;
				triangleRecords[i]->rgbColor[2][2]=1.0;
				triangleRecords[i]->rgbColor[3] = new Point3f;
				triangleRecords[i]->rgbColor[3][0]=1.0;
				triangleRecords[i]->rgbColor[3][1]=1.0;
				triangleRecords[i]->rgbColor[3][2]=1.0;
				size[i]=1;
			}
			CalcAllNormals();
			return TCL_OK;
		}
		//----- position -----
		if (!strcmp(message, "position"))
		{
			if(argc >= 14)
			{
				if(atoi(argv[1]) >= (int)numRecords)
				{
					return TCL_ERROR;
				}
				triangleRecords[atoi(argv[1])]->positions[0][X] = atof(argv[2]);
				triangleRecords[atoi(argv[1])]->positions[0][Y] = atof(argv[3]);
				triangleRecords[atoi(argv[1])]->positions[0][Z] = atof(argv[4]);
				triangleRecords[atoi(argv[1])]->positions[1][X] = atof(argv[5]);
				triangleRecords[atoi(argv[1])]->positions[1][Y] = atof(argv[6]);
				triangleRecords[atoi(argv[1])]->positions[1][Z] = atof(argv[7]);
				triangleRecords[atoi(argv[1])]->positions[2][X] = atof(argv[8]);
				triangleRecords[atoi(argv[1])]->positions[2][Y] = atof(argv[9]);
				triangleRecords[atoi(argv[1])]->positions[2][Z] = atof(argv[10]);
				triangleRecords[atoi(argv[1])]->positions[3][X] = atof(argv[11]);
				triangleRecords[atoi(argv[1])]->positions[3][Y] = atof(argv[12]);
				triangleRecords[atoi(argv[1])]->positions[3][Z] = atof(argv[13]);
				CalcAllNormals();
			}
			else
			{
				return TCL_ERROR;
			}
			return TCL_OK;
		}
		//----- color -----
		if (!strcmp(message, "color"))
		{
			if(argc >= 14)
			{
				if(atoi(argv[1]) >= (int)numRecords)
				{
					return TCL_ERROR;
				}
				triangleRecords[atoi(argv[1])]->rgbColor[0][X] = atof(argv[2]);
				triangleRecords[atoi(argv[1])]->rgbColor[0][Y] = atof(argv[3]);
				triangleRecords[atoi(argv[1])]->rgbColor[0][Z] = atof(argv[4]);
				triangleRecords[atoi(argv[1])]->rgbColor[1][X] = atof(argv[5]);
				triangleRecords[atoi(argv[1])]->rgbColor[1][Y] = atof(argv[6]);
				triangleRecords[atoi(argv[1])]->rgbColor[1][Z] = atof(argv[7]);
				triangleRecords[atoi(argv[1])]->rgbColor[2][X] = atof(argv[8]);
				triangleRecords[atoi(argv[1])]->rgbColor[2][Y] = atof(argv[9]);
				triangleRecords[atoi(argv[1])]->rgbColor[2][Z] = atof(argv[10]);
				triangleRecords[atoi(argv[1])]->rgbColor[3][X] = atof(argv[11]);
				triangleRecords[atoi(argv[1])]->rgbColor[3][Y] = atof(argv[12]);
				triangleRecords[atoi(argv[1])]->rgbColor[3][Z] = atof(argv[13]);
			}
			else
			{
				return TCL_ERROR;
			}
			return TCL_OK;
		}
		//----- size -----
		if (!strcmp(message, "size"))
		{
			if(argc >= 3)
			{
				if(atoi(argv[1]) >= (int)numRecords)
				{
					return TCL_ERROR;
				}
				size[atoi(argv[1])] = atof(argv[2]);
			}
			else
			{
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}
	return GeoData::perform(argc, argv, interp);
	
}

/*---------------------------------------------------------------------------*/
/** This function calculates the normals for all triangles between the passed
 *	corner indices.
 */
void DataTriangles::updatePoints(int record, int start, int end)
{
	Point3d normal = {0.0,0.0,0.0}, oldNormal = {0.0,0.0,0.0}, oldNormalSum = {0.0,0.0,0.0};
	TriangleRecord *rec = triangleRecords[record];
	int i, nPoints = rec->numPoints;
	vNorms *vnorms = rec->norms;
	int newstart, newend;
	int normalsDirective = normalsRule;
	GLboolean front = GL_FALSE;

	newstart = start - 2; //have to recalculate normals back to two points, possibly.
	if (newstart < 0)
		newstart = 0;
	newend = end;
	if (newend >= nPoints - 2) //can't go further than nPoints - 2.
		newend = nPoints - 2;

	if (normalsDirective == PER_RECORD)
		normalsDirective = rec->normalsRule;
	if (normalsDirective == LEFT_HANDED)
		front = GL_TRUE;

	for (i = newstart; i < newend; i++)
	{
		if (front)
                    Glyph::CalcNormal(rec->positions[i], rec->positions[i+1], rec->positions[i+2], normal, lastHeightExaggeration);
		else
                    Glyph::CalcNormal(rec->positions[i], rec->positions[i+2], rec->positions[i+1], normal, lastHeightExaggeration);
		front ^= GL_TRUE;

		if ((normalsDirective == UP && normal[2] < 0.0)
			|| (normalsDirective == DOWN && normal[2] > 0.0))
		{
			normal[0] *= -1.0;
			normal[1] *= -1.0;
			normal[2] *= -1.0;
		}
		
		if (i == newstart && i >= start)
		{
/*			vnorms[i].nx = (GLshort)(normal[0]*32767);
			vnorms[i].ny = (GLshort)(normal[1]*32767);
			vnorms[i].nz = (GLshort)(normal[2]*32767);*/
			vnorms[i][X] = (GLshort)(normal[0]*32767);
			vnorms[i][Y] = (GLshort)(normal[1]*32767);
			vnorms[i][Z] = (GLshort)(normal[2]*32767);
			oldNormal[0] = normal[0];  //for special case, need this!
			oldNormal[1] = normal[1];
			oldNormal[2] = normal[2];
		}
		else if (i == newstart + 1 && i >= start)
		{
/*			vnorms[i].nx = (GLshort)((normal[0] + oldNormal[0])*16383.5);
			vnorms[i].ny = (GLshort)((normal[1] + oldNormal[1])*16383.5);
			vnorms[i].nz = (GLshort)((normal[2] + oldNormal[2])*16383.5);*/
			vnorms[i][X] = (GLshort)((normal[0] + oldNormal[0])*16383.5);
			vnorms[i][Y] = (GLshort)((normal[1] + oldNormal[1])*16383.5);
			vnorms[i][Z] = (GLshort)((normal[2] + oldNormal[2])*16383.5);
		}
		else
		{
/*			vnorms[i].nx = (GLshort)((normal[0] + oldNormalSum[0])*10922.3);
			vnorms[i].ny = (GLshort)((normal[1] + oldNormalSum[1])*10922.3);
			vnorms[i].nz = (GLshort)((normal[2] + oldNormalSum[2])*10922.3);*/
			vnorms[i][X] = (GLshort)((normal[0] + oldNormalSum[0])*10922.3);
			vnorms[i][Y] = (GLshort)((normal[1] + oldNormalSum[1])*10922.3);
			vnorms[i][Z] = (GLshort)((normal[2] + oldNormalSum[2])*10922.3);
		}
		oldNormalSum[0] = oldNormal[0] + normal[0];
		oldNormalSum[1] = oldNormal[1] + normal[1];
		oldNormalSum[2] = oldNormal[2] + normal[2];
		oldNormal[0] = normal[0];
		oldNormal[1] = normal[1];
		oldNormal[2] = normal[2];
	}
	//get the last two
	if (i <= end)
	{
/*		vnorms[i].nx = (GLshort)(oldNormalSum[0]*16383.5);
		vnorms[i].ny = (GLshort)(oldNormalSum[1]*16383.5);
		vnorms[i].nz = (GLshort)(oldNormalSum[2]*16383.5);*/
		vnorms[i][X] = (GLshort)(oldNormalSum[0]*16383.5);
		vnorms[i][Y] = (GLshort)(oldNormalSum[1]*16383.5);
		vnorms[i][Z] = (GLshort)(oldNormalSum[2]*16383.5);
	}
	if (++i <= end)
	{
/*		vnorms[i].nx = (GLshort)(normal[0]*32767);
		vnorms[i].ny = (GLshort)(normal[1]*32767);
		vnorms[i].nz = (GLshort)(normal[2]*32767);*/
		vnorms[i][X] = (GLshort)(normal[0]*32767);
		vnorms[i][Y] = (GLshort)(normal[1]*32767);
		vnorms[i][Z] = (GLshort)(normal[2]*32767);
	}
	//setEdited();
}

/*--------------------------------------------------------------------------*/
void DataTriangles::UpdateTexture(TextureMap *ptr)
{
    unsigned int i, j, numPoints;
	TriangleRecord *rec;
	float z = 0.0;

	if (colorFlags & COLOR_ATTRIBUTE)
	{
		if (colorFlags & COLOR_AS_HEIGHT)
			z = pose->getPosition()[2];

		for (i = 0; i < numRecords; i++)
		{
			rec = triangleRecords[i];
			if (colorFlags & FORMAT_BY_RECORD)
				numPoints = 1;
			else
				numPoints = rec->numPoints;
			for (j = 0; j < numPoints; j++)
				rec->bytewiseColor[j] = GetColorAtValue(rec->colorVal[j] + z);
		}
	}
}
