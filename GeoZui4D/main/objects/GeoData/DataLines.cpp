// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// DataLines.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing lines and tubes, with attributes
 *	associated with the lines or component points (temperature, for
 *	instance) mapped to attributes of the same (color, for instance).
 *	This class implements the following responsibilities:
 *		* Read source data from a file to generate lines
 *		* Support viewing of the lines.
 *
 *	Created by mdp 6/4/2001
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
#include "objects/GeoData/DataLines.h"
#include "widgets/Sv_Standards.h"
#include <filesystem>
#include "interactors/PointHandle.h"
#include "engine/PosableStateWrapper.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/TimeControl.h"
#include <gz4d/glo/Vertex.h>

using namespace std;

extern PointHandle *pointHandle;
extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(DataLines, "gdl");

#define POINTS_AT_A_TIME 10
#define NFACETS 60
#define ZERO_TOLERANCE 0.0001
#define DEFAULT_FACETS 20
#define PI 3.1415926

const char *DataLines::DataLinesNames[] =
			{GEODATA_NAMES, "LINE", "TUBE", "FACETS",
			 "FACETS_NTH", "ROLL", "SHADING", "FLAT", "SMOOTH", "STRETCH_X", "STRETCH_Y",
			 ""};

static int DataLineAttributeOrdering[] = {DataLines::SIZE, DataLines::TEXT, DataLines::ROLL, DataLines::DATA, -1};

float *DataLines::circX = NULL;
float *DataLines::circY = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
DataLines::DataLines()
: GeoData(NumLineAttributes, "DATA_LINES", DataLinesNames)
{
	lineRecords = NULL;
	displayLine = ON;
	displayTube = OFF;
	hasRoll = GL_FALSE;
	if (!circX || !circY)
		CalculateCircles(circX, circY);
	nFacets = DEFAULT_FACETS;
	degradationFlag = FACETS_NTH;
	numDimensions = 2;
	shadingType = FLAT;
	stretchX = stretchY = 1.0;
	
	datafile = NULL;
	datatype = Unknown;

}

/*--------------------------------------------------------------------------*/
DataLines::~DataLines()
{

	if (lineRecords)
	{
		for (unsigned int i = 0; i < numRecords; i++)
			delete lineRecords[i];
		delete lineRecords;
	}
	delete [] wrapableAttributeNames;
}

/*--------------------------------------------------------------------------*/
// Only works with FORMAT_BY_RECORD and COLOR_RGB
void DataLines::modifyLineAttrs(float sizeVal, Point3f colorRGB, unsigned int whichLine)
{
	if (whichLine < numRecords)
	{
		LineRecord *rec = lineRecords[whichLine];

		// size
		if( sizeFlags & FORMAT_BY_RECORD )
			size[whichLine] = sizeVal;

		if( (colorFlags & FORMAT_BY_RECORD) && (colorFlags & COLOR_RGB) )
		{
			COPY_POINT(rec->rgbColor[0], colorRGB);
		}

		// text...

		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
}

/*--------------------------------------------------------------------------*/
DataLines::LineRecord::LineRecord(int points, int colorFormat, int sizeFormat,
								  GLboolean tube, int nFacets, GLboolean hasRoll)
{
    unsigned int i, numColors;

	allocatedPoints = numPoints = points;
	positions.resize(allocatedPoints);
	rgbColor = NULL;
	indexColor = NULL;
	//bytewiseColor = NULL;
	colorVal = NULL;
	texture_t = NULL;
	roll = NULL;
	size = NULL;
	allocatedColors = 0;

	//tube memory allocation
	if (tube)
	{
		discs = new Point3f*[allocatedPoints];
		norms = new vNorms*[allocatedPoints];
		for (i = 0; i < numPoints; i++)
		{
			discs[i] = new Point3f[nFacets];
			norms[i] = new vNorms[nFacets];
		}
	}
	else
	{
		discs = NULL;
		norms = NULL;
	}

	//roll memory allocation
	if (hasRoll)
		roll = new float[allocatedPoints];

	//size memory allocation
	if (sizeFormat & FORMAT_BY_COLUMN)
		size = new float[allocatedPoints];

	//color memory allocation
	if (colorFormat & FORMAT_BY_COLUMN)
		numColors = allocatedPoints;
	else
		numColors = 1;

	if (colorFormat & COLOR_AS_BYTES)
		//bytewiseColor = new const unsigned char*[numColors];
            bytewiseColor.resize(numColors);
	else if (colorFormat & COLOR_TEXTURE)
	{
		colorVal = new float[numColors];
		if (colorFormat & COLOR_TEXTURE_T)
			texture_t = new float[numColors];
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
		else if (colorFormat & COLOR_CODE)
			indexColor = new int[numColors];
	}
	if (colorFormat & COLOR_ATTRIBUTE)
		colorVal = new float[numColors];
}

/*--------------------------------------------------------------------------*/
DataLines::LineRecord::~LineRecord()
{
    unsigned int i;

	if (discs) //if we have discs, we have norms.
	{
		for (i = 0; i < numPoints; i++)
		{
			delete [] discs[i];
			delete [] norms[i];
		}
		delete discs;
		delete norms;
	}
	if (roll)
		delete [] roll;
	if (size)
		delete [] size;
	if (rgbColor)
	{
		if (allocatedColors)
		{
			for (i = 0; i < allocatedColors; i++)
				delete rgbColor[i];
		}
		delete [] rgbColor;
	}
	if (indexColor)
		delete [] indexColor;
// 	if (bytewiseColor)
// 		delete [] bytewiseColor;
	if (colorVal)
		delete colorVal;
	if (texture_t)
		delete texture_t;
}

/*--------------------------------------------------------------------------*/
//#ifdef WIN32
#define EMPTY()
//#else
//#define BLANK
//#define EMPTY() BLANK
//#endif
#define COPY_RECORD(source, sourceIndex, target, targetIndex) \
{\
	COPY_POINT(target##positions[targetIndex], source##positions[sourceIndex]);\
	if (target##discs)\
	{\
		target##discs[targetIndex] = source##discs[sourceIndex];\
		target##norms[targetIndex] = source##norms[sourceIndex];\
	}\
	if (target##roll)\
		target##roll[targetIndex] = source##roll[sourceIndex];\
	if (target##size)\
		target##size[targetIndex] = source##size[sourceIndex];\
	if (colorFormat & FORMAT_BY_COLUMN)\
	{\
		if (!target##bytewiseColor.empty())\
			target##bytewiseColor[targetIndex] = source##bytewiseColor[sourceIndex];\
		else if(!(colorFormat & COLOR_TEXTURE))\
		{\
			target##rgbColor[targetIndex] = source##rgbColor[sourceIndex];\
			if (target##indexColor)\
				target##indexColor[targetIndex] = source##indexColor[sourceIndex];\
		}\
		if (target##colorVal)\
			target##colorVal[targetIndex] = source##colorVal[sourceIndex];\
		if (target##texture_t)\
			target##texture_t[targetIndex] = source##texture_t[sourceIndex];\
	}\
}

/*--------------------------------------------------------------------------*/
GLboolean DataLines::LineRecord::insertPoint(unsigned int index, int nFacets, int colorFormat)
{
	std::vector<gz4d::Point<double> > new_positions;
	Point3f **new_discs = NULL; // the tube cross sections
	vNorms **new_norms = NULL; // normals for components
	float **new_rgbColor = NULL;
	int *new_indexColor = NULL;
	//const unsigned char **new_bytewiseColor = NULL;
        std::vector<gz4d::Pixel<gz4d::RGBA8> > new_bytewiseColor;
	float *new_colorVal = NULL, *new_roll = NULL, *new_texture_t = NULL, *new_size = NULL;
        unsigned int i, newLength, copyPoint, oldNumPoints;
	int i1, i2;
	float c1, c2;

	if (index < numPoints)
	{
		newLength = numPoints + 1;
		copyPoint = index;
	}
	else
	{
		newLength = index + 1;
		copyPoint = numPoints;
	}

	//create a new arrays if we're inserting a point
	if (newLength > allocatedPoints)
	{
		new_positions.resize(newLength);
		if (discs)
		{
			new_discs = new Point3f*[newLength];
			new_norms = new vNorms*[newLength];
		}
		if (roll)
			new_roll = new float[newLength];
		if (size)
			new_size = new float[newLength];
		if (colorFormat & FORMAT_BY_COLUMN)
		{
			if (!bytewiseColor.empty())
				//new_bytewiseColor = new const unsigned char *[newLength];
                            new_bytewiseColor.resize(newLength);
			else if (!(colorFormat & COLOR_TEXTURE))
			{
				new_rgbColor = new float *[newLength];
				if (indexColor)
					new_indexColor = new int[newLength];
			}
			if (colorVal)
			{
				new_colorVal = new float[newLength];
			}
			if (texture_t)
				new_texture_t = new float[newLength];
		}
		else if (colorFormat & FORMAT_BY_RECORD)
		{
			if (!bytewiseColor.empty())
				new_bytewiseColor = bytewiseColor;
			else
			{
				new_rgbColor = rgbColor;
				if (indexColor)
					new_indexColor = indexColor;
			}
			if (colorVal)
				new_colorVal = colorVal;
			if (texture_t)
				new_texture_t = texture_t;
		}

		//copy everything to insertion point
		for (i = 0; i < copyPoint; i++)
//#ifdef WIN32
#ifdef MSVC
				COPY_RECORD(EMPTY(), i, new_, i);
#else
				COPY_RECORD(, i, new_, i);
#endif
	}
	else
	{
		new_positions = positions;
		new_discs = discs;
		new_norms = norms;
		new_roll = roll;
		new_size = size;
		new_rgbColor = rgbColor;
		new_indexColor = indexColor;
		new_bytewiseColor = bytewiseColor;
		new_colorVal = colorVal;
		new_texture_t = texture_t;
	}
	oldNumPoints = numPoints;

	//copy everything past insertion point
	for (i = numPoints; i > copyPoint; i--)
//#ifdef WIN32
#ifdef MSVC
		COPY_RECORD(EMPTY(), i - 1, new_, i);
#else
		COPY_RECORD(, i - 1, new_, i);
#endif

	//insertion in middle or at end
	{
		if (copyPoint >= numPoints) //project last two points out
		{
			i1 = copyPoint - 2;
			i2 = copyPoint - 1;
			c1 = -1;
			c2 = 2;
		}
		else //find midpoint
		{
			if (copyPoint >= 1)
			{
				i1 = copyPoint - 1;
				i2 = copyPoint + 1;
				c1 = .5;
				c2 = .5;
			}
			else
			{
				i1 = copyPoint + 2;
				i2 = copyPoint + 1;
				c1 = -1;
				c2 = 2;
			}
		}
		if (i1 < 0) //oops, not enough points!
		{
			new_positions[copyPoint][X] = 0.0f;
			new_positions[copyPoint][Y] = 0.0f;
			new_positions[copyPoint][Z] = 0.0f;
			if (new_roll)
				new_roll[copyPoint] = 0.0f;
			if (new_size)
				new_size[copyPoint] = 1.0f;
		}
		else //calculate the new point
		{
			new_positions[copyPoint][X] = c1 * new_positions[i1][X] + c2 * new_positions[i2][X];
			new_positions[copyPoint][Y] = c1 * new_positions[i1][Y] + c2 * new_positions[i2][Y];
			new_positions[copyPoint][Z] = c1 * new_positions[i1][Z] + c2 * new_positions[i2][Z];
			if (new_roll)
				new_roll[copyPoint] = c1 * new_roll[i1] + c2 * new_roll[i2];
			if (new_size)
				new_size[copyPoint] = c1 * new_size[i1] + c2 * new_size[i2];
		}
		if (new_discs)
		{
			new_discs[copyPoint] = new Point3f[nFacets];
			new_norms[copyPoint] = new vNorms[nFacets];
		}
		if (colorFormat & FORMAT_BY_COLUMN)
		{
			if (!(colorFormat & COLOR_AS_BYTES) && (colorFormat & COLOR_RGB))
			{
				new_rgbColor[copyPoint] = new Point3f;
				if (i2 >= 0)
				{
					new_rgbColor[copyPoint][X] = new_rgbColor[i2][X];
					new_rgbColor[copyPoint][Y] = new_rgbColor[i2][Y];
					new_rgbColor[copyPoint][Z] = new_rgbColor[i2][Z];
				}
			}
			if (colorFormat & COLOR_CODE)
			{
				if (copyPoint >= numPoints)
					new_indexColor[copyPoint] = indexColor[i2];
				else
					new_indexColor[copyPoint] = indexColor[copyPoint];
				new_rgbColor[copyPoint] = palette[new_indexColor[copyPoint]];
			}
			if ((colorFormat & COLOR_ATTRIBUTE) && (colorFormat & COLOR_AS_HEIGHT))
				new_colorVal[copyPoint] = new_positions[copyPoint][Z];
			if ((colorFormat & COLOR_TEXTURE) && i2 >= 0)
			{
				new_colorVal[copyPoint] = colorVal[i2];
				if (colorFormat & COLOR_TEXTURE_T)
					new_texture_t[copyPoint] = texture_t[i2];
			}
		}
	}
	//insertion past end
	for (i = numPoints + 1; i < newLength; i++)
	{
		i1 = i - 2;
		i2 = i - 1;
		if (i1 < 0) //oops, not enough points!
		{
			new_positions[i][X] = 0.0f;
			new_positions[i][Y] = 0.0f;
			new_positions[i][Z] = 0.0f;
			if (new_roll)
				new_roll[i] = 0.0f;
			if (new_size)
				new_size[i] = 1.0f;
		}
		else //calculate the new point
		{
			new_positions[i][X] = -new_positions[i1][X] + 2 * new_positions[i2][X];
			new_positions[i][Y] = -new_positions[i1][Y] + 2 * new_positions[i2][Y];
			new_positions[i][Z] = -new_positions[i1][Z] + 2 * new_positions[i2][Z];
			if (new_roll)
				new_roll[i] = -new_roll[i1] + 2 * new_roll[i2];
			if (new_size)
				new_size[i] = -new_size[i1] + 2 * new_size[i2];
		}
		if (new_discs)
		{
			new_discs[i] = new Point3f[nFacets];
			new_norms[i] = new vNorms[nFacets];
		}
		if (colorFormat & FORMAT_BY_COLUMN)
		{
			if ((colorFormat & COLOR_AS_BYTES) && (colorFormat & COLOR_RGB))
			{
				new_rgbColor[i] = new Point3f;
				if (i2 >= 0)
				{
					new_rgbColor[i][X] = new_rgbColor[i2][X];
					new_rgbColor[i][Y] = new_rgbColor[i2][Y];
					new_rgbColor[i][Z] = new_rgbColor[i2][Z];
				}
			}
			if (colorFormat & COLOR_CODE)
			{
				new_indexColor[i] = new_indexColor[i2];
				new_rgbColor[i] = palette[new_indexColor[copyPoint]];
			}
			if ((colorFormat & COLOR_ATTRIBUTE) && (colorFormat & COLOR_AS_HEIGHT))
				new_colorVal[copyPoint] = new_positions[copyPoint][Z];
			if ((colorFormat & COLOR_TEXTURE) && i2 >= 0)
			{
				new_colorVal[copyPoint] = colorVal[i2];
				if (colorFormat & COLOR_TEXTURE_T)
					new_texture_t[copyPoint] = texture_t[i2];
			}
		}
	}
	numPoints = newLength;
	if (colorFormat & COLOR_ALLOCATED)
		allocatedColors = newLength;

	if (new_positions != positions)
	{
		if (discs)
		{
			delete []discs;
			delete []norms;
		}
		if (roll)
			delete [] roll;
		if (size)
			delete [] size;
		if (colorFormat & FORMAT_BY_COLUMN)
		{
// 			if (new_bytewiseColor)
// 				delete []bytewiseColor;
// 			else 
                        if (new_bytewiseColor.empty() && !(colorFormat & COLOR_TEXTURE))
			{
				if (new_rgbColor) //we don't delete the elements, because they have been transferred to the new array.
					delete []rgbColor;
				if (new_indexColor)
					delete []indexColor;
			}
			if (new_colorVal)
				delete []colorVal;
			if (new_texture_t)
				delete []texture_t;
		}
		discs = new_discs;
		norms = new_norms;
		roll = new_roll;
		size = new_size;
		rgbColor = new_rgbColor;
		indexColor = new_indexColor;
		bytewiseColor = new_bytewiseColor;
		colorVal = new_colorVal;
		texture_t = new_texture_t;
		positions = new_positions;
		allocatedPoints = numPoints;
	}
	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
GLboolean DataLines::LineRecord::deletePoint(unsigned int index, int colorFormat)
{
    unsigned int i;

	//delete the bits that need deleting.
	if (discs)
	{
		delete [] discs[index];
		delete [] norms[index];
	}
	if ((rgbColor && allocatedColors) && (colorFormat & FORMAT_BY_COLUMN))	// hack rjk
		delete rgbColor[index];

	//move everyone down.
	for (i = index + 1; i < numPoints; i++)
//#ifdef WIN32
#ifdef MSVC
		COPY_RECORD(EMPTY(), i, EMPTY(), i - 1);
#else
		COPY_RECORD(, i,, i - 1);
#endif

	numPoints--;
	if (colorFormat & COLOR_ALLOCATED)
		allocatedColors = numPoints;

	return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DataLines::DrawLowRes(float degradation, Window3D *win)
{
	unsigned int i, point, increment = 1, gap, numPoints, start, end;
	GLboolean trailing = GL_FALSE;

	//determine degradation type
	if (renderingForPicking) //ObjectCollection takes care of degradation == OFF
		degradation = 0.0;
	if (degradationFlag == NTH || degradationFlag == FACETS_NTH || degradationFlag == ON)
		increment = (int)(2.1 / (2.1 - (2 * degradation)));
	if (degradationFlag == FACETS || degradationFlag == FACETS_NTH || degradationFlag == ON) //ON equal to FACETS_NTH
		gap = (int)((((nFacets / 3) - 1) * degradation) + 1);
	else
		gap = 1;

	//Are we drawing a trailing section?
	if (trailingSeconds > 0.0 && timeAttributeIndex >= 0)
		trailing = GL_TRUE;

	//draw
	//size
	if (sizeFlags & FORMAT_CONSTANT)
		glLineWidth(sizeFactor);

	for (i = 0; i < numRecords; i++)
	{
		if (renderingForPicking)
			glLoadName(i);
		numPoints = lineRecords[i]->numPoints;
		if (trailing)
		{
			updateTrailingIndexes(i);
			start = trailingStartIndex;
			end = trailingEndIndex + 1;
		}
		else
		{
			start = 0;
			end = numPoints;
		}
		for (point = start; point < end - 1; point += increment)
		{
			if ((point + increment) < end)
				DrawShadedRing(win, i, point, !renderingForPicking, increment, gap);
			else
				DrawShadedRing(win, i, point, !renderingForPicking, end - point - 1, gap);
		}
		if (selectedFlag && selectedRecord >= 0)
			DrawHandle(win, i, point, lineRecords[i]->positions);
	}
	glLineWidth(1);
	glColor3f(1.0,1.0,1.0);

	ResetHighRes();
}

/*--------------------------------------------------------------------------*/
int DataLines::DrawHighRes(int eye, Window3D *win)
{
	unsigned int j, newCurrentRecord, newCurrentPoint, end;
	GLboolean trailing = GL_FALSE;

	if (!doneFlag)
	{
		//Are we drawing a trailing section?
		if (trailingSeconds > 0.0 && timeAttributeIndex >= 0)
			trailing = GL_TRUE;

		//size
		if (sizeFlags & FORMAT_CONSTANT)
			glLineWidth(sizeFactor);

		if (currentRecord < numRecords)
		{
			//draw POINTS_AT_A_TIME segments at once
			newCurrentRecord = currentRecord;
			newCurrentPoint = currentPoint;
			if (trailing)
				end = trailingEndIndex;
			else
				end = lineRecords[newCurrentRecord]->numPoints - 1;
			for (j = 0; j < POINTS_AT_A_TIME; j++) 
			{
				if (newCurrentRecord < numRecords && newCurrentPoint < end)
					DrawShadedRing(win, newCurrentRecord, newCurrentPoint);
				else
					break;
				if (++newCurrentPoint >= end)
				{
					if (selectedFlag && selectedRecord >= 0)
						DrawHandle(win, newCurrentRecord, newCurrentPoint, lineRecords[newCurrentRecord]->positions);
					newCurrentPoint = 0;
					newCurrentRecord++;
					if (trailing)
					{
						updateTrailingIndexes(newCurrentRecord);
						currentPoint = trailingStartIndex;
						end = trailingEndIndex;
					}
					else if (newCurrentRecord < numRecords)
						end = lineRecords[newCurrentRecord]->numPoints - 1;
					else
						break;
				}
			}
			if (eye == dominantEye || eye == MONO_EYE)
			{
				currentPoint = newCurrentPoint;
				currentRecord = newCurrentRecord;
			}
			glLineWidth(1);
			glColor3f(1.0,1.0,1.0);
		}
		else
			doneFlag = 1;
	}
	else
		doneFlag = 1;
	
	return doneFlag;
}

/*--------------------------------------------------------------------------*/
void DataLines::DrawShadedRing(Window3D *win, int record, int seg, GLboolean inLowRes, int pointGap, int gap)
{
	// draw a single shaded triangle strip
	// need to change to row major order
    int i, ci, ci2;
    vNorms *norm1, *norm2;
    LineRecord *rec = lineRecords[record];
    Point3f *disc1, *disc2;
    std::shared_ptr<TextureMap> currentTexture;
    union
    {
        float               *f;
        //const unsigned char **u;
        const unsigned char* u;
        struct
        {
            float s;
            float t;
        } t;
    } color1, color2;
    gz4d::Pixel<gz4d::RGBA8> color1p, color2p;
    color1.u = reinterpret_cast<const unsigned char*>(&color1p);
    color2.u = reinterpret_cast<const unsigned char*>(&color2p);

    norm1 = NULL;
    norm2 = NULL;
    disc1 = NULL;
    disc2 = NULL;
    
    
//---the following macro allows for inline color-setting.
#define SET_COLOR(color) \
    if (colorFlags & COLOR_ATTRIBUTE) \
    glColor4ubv(color.u); \
    else if (colorFlags & COLOR_TEXTURE) \
    { \
    glColor3f(1.0, 1.0, 1.0); \
    if (colorFlags & FORMAT_BY_COLUMN) \
    {\
    if (colorFlags & COLOR_TEXTURE_T)\
    glTexCoord2f(color.t.s, color.t.t);\
    else\
    currentTexture->SetHeight(color.t.s);\
}\
} \
    else \
    glColor3fv(color.f);
//-----------------------------------

    if (displayTube != OFF)
    {
        disc1 = rec->discs[seg];
        disc2 = rec->discs[seg+pointGap];
	
        norm1 = rec->norms[seg];
        norm2 = rec->norms[seg+pointGap];
    }

	//Draw selection indicator
    if (selectedFlag && selectedRecord >= 0)
        DrawHandle(win, record, seg, rec->positions);

	//color
    if (colorFlags & FORMAT_CONSTANT)
    {
        if (colorFlags & COLOR_RGB)
            color1.f = color2.f = constantColor.rgb;
        else
            color1.f = color2.f = palette[constantColor.index];
    }
    else
    {
        if (colorFlags & FORMAT_BY_RECORD)
            ci = ci2 = 0;
        else
        {
            ci = seg;
            ci2 = seg + pointGap;
        }
        if (colorFlags & COLOR_AS_BYTES)
        {
//             color1.u = &rec->bytewiseColor[ci];
//             color2.u = &rec->bytewiseColor[ci2];
            color1p = rec->bytewiseColor[ci];
            color2p = rec->bytewiseColor[ci2];
            
        }
        else if (colorFlags & COLOR_TEXTURE)
        {
            if (!customtextureptr)
                currentTexture = *defaulttextureptr;
            else
                currentTexture = customtextureptr;
            if(colorFlags & FORMAT_BY_COLUMN)
            {
                EnableTexture(TEXTURE_COORD_NONE);
                color1.t.s = rec->colorVal[ci];
                color2.t.s = rec->colorVal[ci2];
                if (colorFlags & COLOR_TEXTURE_T)
                {
                    color1.t.t = rec->texture_t[ci];
                    color2.t.t = rec->texture_t[ci2];
                }
            }
            else
                EnableTexture();
        }
        else
        {
            color1.f = rec->rgbColor[ci];
            color2.f = rec->rgbColor[ci2];
        }
    }

	//Draw actual line
    if (!(renderingForPicking && selectedFlag))
    {
		//Draw line to make visible from far away.
        if (displayLine == ON || (inLowRes && displayLine == TRANSIENT)
            || (!inLowRes && displayLine == FRAGILE))
        {
            if (shadingType == SMOOTH)
                glShadeModel(GL_SMOOTH);
            else
                glShadeModel(GL_FLAT);
            if (sizeFlags & FORMAT_BY_RECORD)
                glLineWidth(size[record]);
            glBegin(GL_LINES);
            glNormal3f(0.0, 0.0, 1.0);
            SET_COLOR(color1);
            gz4d::glo::Vertex(rec->positions[seg]);
            SET_COLOR(color2);
            gz4d::glo::Vertex(rec->positions[seg + pointGap]);
            glEnd();
        }

		//do pairs of points to make quads along tube.
        if (displayTube == ON || (inLowRes && displayTube == TRANSIENT)
            || (!inLowRes && displayTube == FRAGILE))
        {
            if (shadingType == SMOOTH)
                glShadeModel(GL_SMOOTH);
            else
                glShadeModel(GL_FLAT);
            if (selectedFlag && !asWireframe())
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBegin(GL_QUAD_STRIP);
            for (i = 0; i < nFacets; i+=gap)
            {
                if (!renderingForPicking)
                {
                    SET_COLOR(color1);
					//glNormal3s(norm1[i].nx, norm1[i].ny, norm1[i].nz);
                    glNormal3sv(norm1[i]);
                }
                glVertex3fv(disc1[i]);
                if (!renderingForPicking)
                {
                    SET_COLOR(color2);
                    if (shadingType == SMOOTH)
						//glNormal3s(norm2[i].nx, norm2[i].ny, norm2[i].nz);
                        glNormal3sv(norm2[i]);
                }
                glVertex3fv(disc2[i]);
            } 
			// finish the ring
            SET_COLOR(color1);
            if (!renderingForPicking)
				//glNormal3s(norm1[0].nx, norm1[0].ny, norm1[0].nz);
                glNormal3sv(norm1[0]);
            glVertex3fv(disc1[0]);
            SET_COLOR(color2);
            if (!renderingForPicking)
				//glNormal3s(norm2[0].nx, norm2[0].ny, norm2[0].nz);
                glNormal3sv(norm2[0]);
            glVertex3fv(disc2[0]);
            glEnd();
            if (selectedFlag && !asWireframe())
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
    if (colorFlags & COLOR_TEXTURE)
        DisableTexture();
}

/*--------------------------------------------------------------------------*/
void DataLines::DrawHandle(Window3D *win, int record, int seg, std::vector<gz4d::Point<double> > const &Path)
{
	int level = 3;
	float size = 9.0; //5
		
	if (!isEditableFlag)
		return;

	if (renderingForPicking)
		glPushName(seg);

	if (record == selectedRecord)
	{
		if (seg == selectedPoint)
			level = 1;
		else
			level = 2;
	}
	if (seg == 0)
		size = 13.0; //9
	pointHandle->drawIn(win, Path[seg], level,true, size);
	if (renderingForPicking)
		glPopName();
}

/*---------------------------------------------------------------------------*/
//added by mdp 9/2/2005 to allow rotations/scales to propogate to points
//&&&currently only applies yaw, scale, and position!
void DataLines::applyTransformsToPoints()
{
        gz4d::Point<double> rotation;
	float scale, yaw = 0.0;
	OrientationInfo o;
	unsigned int lineIndex, numPoints, index;
	int mask;
	float newx, newy;

	mask = pose->supportMask(ChangeRotate | ChangeScale);

	pose->AccumulateNotifications();
	if (mask & ChangeScaleMask)
	{
		scale = pose->getScale();
		pose->setScale(1.0);
	}
	else
		scale = 1;

	if (mask & ChangeRotateMask)
	{
		rotation = pose->getOrientationAxis();
		yaw = rotation[Yaw] * M_PI/180.0;
		pose->setOrientation(o);
	}


	for (lineIndex = 0; lineIndex < numRecords; lineIndex++)
	{
		numPoints = lineRecords[lineIndex]->numPoints;
		for (index = 0; index < numPoints; index++)
		{
                    newx = cos(yaw) * lineRecords[lineIndex]->positions[index][X] + sin(yaw) * lineRecords[lineIndex]->positions[index][Y];
                    newy = -sin(yaw) * lineRecords[lineIndex]->positions[index][X] + cos(yaw) * lineRecords[lineIndex]->positions[index][Y];
                    lineRecords[lineIndex]->positions[index][X] = newx * scale + pose->getPosition()[X];
                    lineRecords[lineIndex]->positions[index][Y] = newy * scale + pose->getPosition()[Y];
                    lineRecords[lineIndex]->positions[index][Z] = lineRecords[lineIndex]->positions[index][Z] * scale + pose->getPosition()[Z];
		}
	}

	pose->setPosition(); //zero out the position
	pose->FlushNotifications();
}

/*---------------------------------------------------------------------------*/
void DataLines::updateWrapper(PosableStateWrapper *wrapper, long &changeMask)
{
	unsigned int lineIndex, numPoints, index;

	InterpolationSupport::updateWrapper(wrapper, changeMask);

	if (pointHandle->usingWrapper(wrapper))
	{
		lineIndex = wrapper->getIntegerIndex(2);

		//don't allow it to go off the end.
		if (lineIndex < 0 || lineIndex >= numRecords)
		{
			if (lineIndex < 0)
				lineIndex = 0;
			else
				lineIndex = numRecords - 1;
			wrapper->setIndex(int(lineIndex), 2);
		}
		index = (int)wrapper->getFloatIndex();
		numPoints = lineRecords[lineIndex]->numPoints;
		if (index < 0 || index >= numPoints)
		{
			if (index < 0)
				index = 0;
			else
				index = numPoints - 1;
			wrapper->setIndex(int(index));
		}

		selectedRecord = lineIndex;
		selectedPoint = index;
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void DataLines::ResetHighRes()
{
	doneFlag = currentRecord = currentPoint = 0;
	if (trailingSeconds > 0.0 && timeAttributeIndex >= 0)
	{
		updateTrailingIndexes(0);
		currentPoint = trailingStartIndex;
	}
}

/*--------------------------------------------------------------------------*/
int DataLines::processField(int option, char **fields)
{
	int i = 1;

	switch(option)
	{
		case LINE:
			displayLine = Lookup(fields[i++]);
			break;
		case TUBE:
			displayTube = Lookup(fields[i++]);
			if (displayTube != OFF && fields[i] && strcmp(fields[i++], "FACETS") == 0)
			{
				nFacets = ConvertInt(i);
				if (nFacets < 3)
				{
					cerr << "Invalid number of FACETS ("
						 << nFacets << ".  Setting value to ";
					nFacets = DEFAULT_FACETS;
					cerr << nFacets << "." << endl;
				}
			}
			break;
		case ROLL:
			if (Lookup(fields[i++]) != COLUMN)
			{
				cerr << "Expecting COLUMN after ROLL.  Ignoring ROLL line." << endl;
				return 0;
			}
			attribToColumn[ATTRIB_ROLL] = ConvertInt(i) - 1;
			hasRoll = GL_TRUE;
			break;
		case SHADING:
			shadingType = Lookup(fields[i++]);
			if (shadingType < 0)
				return 1;
			if (shadingType != FLAT && shadingType != SMOOTH)
			{
				cerr << "Expecting FLAT or SMOOTH for SHADING line.  Ignoring " << fields[i - 1] << " in SHADING line." << endl;
				return 0;
			}
			break;
			/*&&&
		case RENDER_PORTION:
			renderPortionDirection = Lookup(fields[i++]); //TO or FROM
			renderPortionType = Lookup(fields[i++]); //only INDEX is currently supported
			//renderPortionWrapper would be created to update index with time?  or use TIME directly?
			break;
			*/
		case STRETCH_X:
			stretchX = ConvertFloat(i);
			break;
		case STRETCH_Y:
			stretchY = ConvertFloat(i);
			break;
		default:
			return GeoData::processField(option, fields);
			break;
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int DataLines::readFrom(FILE *infile)
{
	char **fields;
	unsigned int i, k, point, numPoints;
	LineRecord *rec;

	if (GeoData::readFrom(infile))
		return 1;
	convertDataFields();

	if ((colorFlags & COLOR_RGB) && !(colorFlags & COLOR_AS_BYTES))
		colorFlags |= COLOR_ALLOCATED;
	if (sizeFlags & FORMAT_BY_COLUMN)
	{
		delete [] size;
		size = NULL;
	}

	//allocate necessary memory.
	lineRecords = new LineRecord *[numRecords];

	// read lines one record at a time
	for (i = 0; i < numRecords; i++)
	{
		fields = NextLine(GL_FALSE);
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
		rec = lineRecords[i] = new LineRecord(numPoints, colorFlags, sizeFlags, (displayTube != OFF), nFacets, hasRoll);
		//color
		if (colorFlags & FORMAT_BY_RECORD)
		{
			if (colorFlags & COLOR_ATTRIBUTE)
				rec->colorVal[0] = ConvertFloat(j);
			else if (colorFlags & COLOR_RGB)
				ConvertFloatPoint(rec->rgbColor[0], j);
			else if (colorFlags & COLOR_CODE)
			{
				rec->indexColor[0] = convertColorCode(fields[j++][0]);
				rec->rgbColor[0] = palette[rec->indexColor[0]];
			}
		}
		//size
		if (sizeFlags & FORMAT_BY_RECORD)
			size[i] = ConvertFloat(j);
		//text
		if (textFlags & FORMAT_BY_RECORD)
			text[i] = ReleaseField(j++);
		//data
		for (k = 0; k < numDataFields; k++)
		{
			if (dataRepository[k].format == FORMAT_BY_RECORD)
				convertData(fields, &dataRepository[k], 0, i, j++);
			else
				dataRepository[k].data[i].init(numPoints, (SmartArray::IndexType)(dataRepository[k].type & ~TAKE_LG));
		}

		//---Gather per-point data---
		for (point = 0; point < numPoints; point++)
		{
			fields = NextLine();
			if (!fields)
			{
				rec->numPoints = point;
				cerr << "File ended while looking for point " << point + 1
					 << ".  Line truncated." << endl;
				break;
			}
			//position
			rec->positions[point][0] = ConvertFloatAt(attribToColumn[ATTRIB_X]);
			rec->positions[point][1] = ConvertFloatAt(attribToColumn[ATTRIB_Y]);
			rec->positions[point][2] = ConvertFloatAt(attribToColumn[ATTRIB_Z]);
			//roll (twist)
			if (attribToColumn[ATTRIB_ROLL] != -1)
				rec->roll[point] = ConvertFloatAt(attribToColumn[ATTRIB_ROLL]);
			if (attribToColumn[ATTRIB_SIZE] != -1)
				rec->size[point] = ConvertFloatAt(attribToColumn[ATTRIB_SIZE]);
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
				{
					rec->indexColor[point] = convertColorCode(fields[attribToColumn[ATTRIB_COLOR]][0]);
					rec->rgbColor[point] = palette[rec->indexColor[point]];
				}
				else
					rec->colorVal[point] = ConvertFloatAt(attribToColumn[ATTRIB_COLOR]);
			}
			if (attribToColumn[ATTRIB_TEXTURE_S] != -1)
				rec->colorVal[point] = ConvertFloatAt(attribToColumn[ATTRIB_TEXTURE_S]);
			if (attribToColumn[ATTRIB_TEXTURE_T] != -1)
				rec->texture_t[point] = ConvertFloatAt(attribToColumn[ATTRIB_TEXTURE_T]);
			//data
			for (unsigned int j = 0; j < numDataFields; j++)
			{
				if (dataRepository[j].format == FORMAT_BY_COLUMN)
					convertData(fields, &dataRepository[j], i, point, dataRepository[j].column);
			}
		}
		projectXY(rec->positions, GL_FALSE);
	}
	for (unsigned int j = 0; j < numDataFields; j++)
	{
		if (dataRepository[j].type & (TAKE_DIFFERENTIAL | TAKE_DOUBLE_DIFFERENTIAL))
			differentiate(dataRepository[j]);
		if (dataRepository[j].type & TAKE_DOUBLE_DIFFERENTIAL)
		{
			differentiate(dataRepository[j]);
			for (i = 0; i < numRecords; i++)
			{
				dataRepository[j].data[i][1].set(0.0f);
				dataRepository[j].data[i][dataRepository[j].data[i].size - 1].set(0.0f);
			}
		}
		if (dataRepository[j].type & (TAKE_LG | TREAT_AS_OFFSET | TAKE_DIFFERENTIAL | TAKE_DOUBLE_DIFFERENTIAL))
			dataRepository[j].type = SmartMemory::Float;
	}
	projectionRequired = GL_FALSE; //just to be consistent.
	CalcAllNormals();
	setNumLines(numRecords);
	setIndexedAttributeOrdering(numWrapableAttributes, wrapableAttributeNames, DataLineAttributeOrdering);
	return(0);
}

/*---------------------------------------------------------------------------*/
void DataLines::differentiate(DataRepository &d)
{
    unsigned int i, end, rec;
	float prev, cur, next;
	SmartArray &a = d.data[0];

	if ((d.type & SmartMemory::TypeMask) != SmartMemory::Float)
		return;

	for (rec = 0; rec < numRecords; rec++)
	{
		a = d.data[rec];
		end = a.size - 1;
		if (end < 2)
			continue;
		prev = a[0].asFloat();
		cur = a[1].asFloat();
		a[0].set(0.0f);
		for (i = 1; i < end; i++)
		{
			next = a[i+1].asFloat();
			a[i].set((next - prev)/2.0f);
			prev = cur;
			cur = next;
		}
		a[end].set(0.0f);
	}
}

/*---------------------------------------------------------------------------*/
/**	The GeoData writeTo function takes care of writing out the non-specific
 *	header information appropriately.  It assumes numColumns is set to the
 *	number of child-specific columns that are to be written out as of the
 *	call to this function, and calculates how many more columns are necessary
 *	to build.  The child is to write itself out as compactly as possible,
 *	always using XYZ layout (X, Y, and Z in the first three columns of data).
 *	The order of columns after the XYZ columns is always child-specific fist,
 *	then color, size, and text.
 */
int DataLines::writeTo(FILE *file)
{
	LineRecord *rec;
	unsigned int i, j, point, numPoints;
	int retval, skipColor;
	numColumns = 0; //count the new number of columns

	//write header
	fprintf(file, "%s %d\n", Keyword(N_RECORDS), numRecords);
	fprintf(file, "%s %s\n", Keyword(LINE), Keyword(displayLine));
	fprintf(file, "%s %s", Keyword(TUBE), Keyword(displayTube));
	if (displayTube != OFF)
		fprintf(file, " FACETS %d\n", nFacets);
	else
		fprintf(file, "\n");
	if (hasRoll)
		fprintf(file, "%s %s %d\n", Keyword(ROLL), Keyword(COLUMN), 3 + ++numColumns);
	fprintf(file, "%s %s\n", Keyword(SHADING), Keyword(shadingType));
	if (stretchX != 1.0f)
		fprintf(file, "%s %g\n", Keyword(STRETCH_X), stretchX);
	if (stretchY != 1.0f)
		fprintf(file, "%s %g\n", Keyword(STRETCH_Y), stretchY);
	retval = GeoData::writeTo(file);
	if (retval != TCL_OK)
		return retval;

	//write lines out one record at a time
	for (i = 0; i < numRecords; i++)
	{
		rec = lineRecords[i];

		//generate NEW line
		fprintf(file, "NEW %d", rec->numPoints);

		//color
		if (colorFlags & FORMAT_BY_RECORD)
		{
			if (colorFlags & COLOR_ATTRIBUTE)
				fprintf(file, " %g", rec->colorVal[0]);
			else if (colorFlags & COLOR_RGB)
				fprintf(file, " %g %g %g", rec->rgbColor[0][0], rec->rgbColor[0][1], rec->rgbColor[0][2]);
			else if (colorFlags & COLOR_CODE)
				fprintf(file, " %c", convertColorCode(rec->indexColor[0]));
		}

		//size
		if (sizeFlags & FORMAT_BY_RECORD)
			fprintf(file, " %g", size[i]);

		//text
		if (textFlags & FORMAT_BY_RECORD)
			fprintf(file, " \"%s\"", text[i]);

		//data
		for (j = 0; j <numDataFields; j++)
		{
			if (dataRepository[j].format & FORMAT_BY_RECORD)
			{
				if (dataRepository[j].type == SmartMemory::String)
					fprintf(file, " \"%s\"", dataRepository[j].data[0][i].asString());
				else
					fprintf(file, " %g", dataRepository[j].data[0][i].asFloat());
			}
		}

		//done with the NEW line, add newline
		fprintf(file, "\n");

		//Write individual point data (Always XYZ, own stuff, color, size, text, data)
		numPoints = rec->numPoints;
		skipColor = GL_FALSE;
		if (attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_X] ||
			attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_Y] ||
			attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_Z])
			skipColor = GL_TRUE;
		for (point = 0; point < numPoints; point++)
		{
			//position
			fprintf(file, "%g %g %g", rec->positions[point][X], rec->positions[point][Y], rec->positions[point][Z]);
			//roll
			if (hasRoll)
				fprintf(file, " %g", rec->roll[point]);
			//color
			if (attribToColumn[ATTRIB_R] != -1)
				fprintf(file, " %g", rec->rgbColor[point][0]);
			if (attribToColumn[ATTRIB_G] != -1)
				fprintf(file, " %g", rec->rgbColor[point][1]);
			if (attribToColumn[ATTRIB_B] != -1)
				fprintf(file, " %g", rec->rgbColor[point][2]);
			if (attribToColumn[ATTRIB_COLOR] != -1 && !skipColor)
			{
				if (colorFlags & COLOR_CODE)
					fprintf(file, " %c", convertColorCode(rec->indexColor[point]));
				else
					fprintf(file, " %g", rec->colorVal[point]);
			}
			if (attribToColumn[ATTRIB_TEXTURE_S] != -1)
				fprintf(file, " %g", rec->colorVal[point]);
			if (attribToColumn[ATTRIB_TEXTURE_T] != -1)
				fprintf(file, " %g", rec->texture_t[point]);
			//size
			if (attribToColumn[ATTRIB_SIZE] != -1)
				fprintf(file, " %g", rec->size[point]);
			//data
			for (j = 0; j <numDataFields; j++)
			{
				if (dataRepository[j].format & FORMAT_BY_COLUMN)
				{
					if (dataRepository[j].type == SmartMemory::String)
						fprintf(file, " \"%s\"", dataRepository[j].data[i][point].asString());
					else
						fprintf(file, " %g", dataRepository[j].data[i][point].asFloat());
				}
			}
			fprintf(file, "\n");
		}
	}
	return retval;
}

/*---------------------------------------------------------------------------*/
void DataLines::CalcAllNormals()
{

	if (displayTube == OFF)
		return;

	for (unsigned int i = 0; i < numRecords; i++)
		updatePoints(i, 0, lineRecords[i]->numPoints - 1);
}

/*---------------------------------------------------------------------------*/
/** This function calculates the joints for all the joints between the passed
 *	pipe corner indices.
 */
void DataLines::updatePoints(int record, int start, int end)
{
	Point3f Normal = {0.0, 0.0, 1.0}, Side, Forward, Pipe1, Pipe2, LightingNormal;
	Point3f Arbitrary = {0.0, 0.0, 1.0};
	float normX, normY, normZ;
	float dia, xFactor, yFactor, distMoved;
	LineRecord *rec = lineRecords[record];
	int i, j, step, nPoints = rec->numPoints;
	float *sizes = rec->size;
	float cosTheta = 0.0, sinTheta=0.0, xSector, ySector;
	int sFactor = 32767;
   //float longFactor = 1.0;

	dia=0.0;
    
	//Recreate the joints for the points in the given range
	for (i = start; i <= end; i++)
	{
		if (i < 0 || i >= nPoints)
			continue;

		Pipe1[X] = Pipe1[Y] = Pipe1[Z] = 0.0;
		Pipe2[X] = Pipe2[Y] = Pipe2[Z] = 0.0;

		//Determine the forward vector for the circle at this
		//point by finding te vector from the previous point to the
		//next point.  If one or the other is missing, discard it
		//from the calculation.
		if (i > 0)
		{
                    Forward[X] = (Pipe1[X] = rec->positions[i][X] - rec->positions[i - 1][X]);
                    Forward[Y] = (Pipe1[Y] = rec->positions[i][Y] - rec->positions[i - 1][Y]);
                    Forward[Z] = (Pipe1[Z] = rec->positions[i][Z] - rec->positions[i - 1][Z]);
		}
		else
			Forward[X] = Forward[Y] = Forward[Z] = 0.0;

		if (i < nPoints - 1)
		{
                    Forward[X] += (Pipe2[X] = rec->positions[i + 1][X] - rec->positions[i][X]);
                    Forward[Y] += (Pipe2[Y] = rec->positions[i + 1][Y] - rec->positions[i][Y]);
                    Forward[Z] += (Pipe2[Z] = rec->positions[i + 1][Z] - rec->positions[i][Z]);
		}

		if (Forward[X] == 0.0 && Forward[Y] == 0.0 && Forward[Z] == 0.0)
			Forward[X] = 1.0; //arbitrary direction for points that overlap

		//Make certain the arbitrary vector is not in the same direction
		//as the forward vector, so we can use it to find a normal
		//to the forward (our goal is to find two orthogonal vectors, each
		//of which is normal to the Forward vector, so we can use them
		//in determining the disc points).
		distMoved = Forward[X] * Forward[X] + Forward[Y] * Forward[Y];
		if (distMoved <= fabs(Forward[Z]) / 1.0)
		{
			if (distMoved == 0)
				Arbitrary[X] = Forward[Z];
			else
				Arbitrary[X] = Normal[X];
			Arbitrary[Y] = Normal[Y];
			Arbitrary[Z] = Normal[Z];
		}
		else
		{
			Arbitrary[X] = 0.0;
			Arbitrary[Y] = 0.0;
			Arbitrary[Z] = 1.0;
		}
/*---For some reason, the following code didn't work.---
		// if the two vectors have a normal, use this for the side, and
		// also see if we can get 1/sin(theta/2), where theta is the
		// angle between the vectors.  By the half-angle formula, we
		// get that we are looking for sqrt(2/(1 - cos(theta)).
		if (CalcNormal(Pipe1, Pipe2, Side))
		{
			div = 1 + CalcDot(Pipe1, Pipe2);
			if (div > ZERO_TOLERANCE)
				longFactor = sqrt(2/div);
			if (longFactor > MAX_FACTOR)
				longFactor = MAX_FACTOR;
		}
		else
*/
			CalcNormal(Forward, Arbitrary, Side);  // Side is the resulting side vector
		CalcNormal(Side, Forward, Normal);  // This is the resulting orthogonal vector

		//now rotate the disk into the Side and Normal vectors
		if (sizeFlags & FORMAT_CONSTANT)
			dia = sizeFactor;
		else if (sizeFlags & FORMAT_BY_RECORD)
			dia = size[record];

		if(displayTube == ON)
		{
  			step = NFACETS / nFacets;
			if (hasRoll)
			{
				cosTheta = (float)cos(rec->roll[i]);
				sinTheta = (float)sin(rec->roll[i]);
			}
			else
				sFactor = -32767;

  			for (j = 0; j < nFacets; j++)
  			{
				if (hasRoll)
				{
					xSector = stretchX * circX[j * step];
					ySector = stretchY * circY[j * step];

					xFactor = xSector * sinTheta + ySector * cosTheta;
					yFactor = xSector * cosTheta - ySector * sinTheta;
				}
				else
				{
  					xFactor = circX[j * step];
  					yFactor = circY[j * step];
				}

				if (sizeFlags & FORMAT_BY_COLUMN)
					dia = sizes[i] * sizeFactor;

                                rec->discs[i][j][X] = rec->positions[i][X] + dia*(Normal[X]*xFactor + Side[X]*yFactor);
                                rec->discs[i][j][Y] = rec->positions[i][Y] + dia*(Normal[Y]*xFactor + Side[Y]*yFactor);
                                rec->discs[i][j][Z] = rec->positions[i][Z] + dia*(Normal[Z]*xFactor + Side[Z]*yFactor);

				if (shadingType == SMOOTH)
				{
  					xFactor = circX[j * step];
  					yFactor = circY[j * step];

  					// rotate it into place
  					normX = (Normal[X]*xFactor + Side[X]*yFactor);
  					normY = (Normal[Y]*xFactor + Side[Y]*yFactor);
  					normZ = (Normal[Z]*xFactor + Side[Z]*yFactor);

/*  					rec->norms[i][j].nx = short(normX*sFactor);
  					rec->norms[i][j].ny = short(normY*sFactor);
  					rec->norms[i][j].nz = short(normZ*sFactor);*/
  					rec->norms[i][j][X] = GLshort(normX*sFactor);
  					rec->norms[i][j][Y] = GLshort(normY*sFactor);
  					rec->norms[i][j][Z] = GLshort(normZ*sFactor);
				}
  			}
			if (shadingType != SMOOTH)
			{

				for (j = 1; j < nFacets; j++)
				{
					Side[0] = rec->discs[i][j-1][X] - rec->discs[i][j][X];
					Side[1] = rec->discs[i][j-1][Y] - rec->discs[i][j][Y];
					Side[2] = rec->discs[i][j-1][Z] - rec->discs[i][j][Z];

					CalcNormal(Forward, Side, LightingNormal);

/*					rec->norms[i][j].nx = short(LightingNormal[0]*sFactor);
					rec->norms[i][j].ny = short(LightingNormal[1]*sFactor);
					rec->norms[i][j].nz = short(LightingNormal[2]*sFactor);*/
					rec->norms[i][j][X] = GLshort(LightingNormal[0]*sFactor);
					rec->norms[i][j][Y] = GLshort(LightingNormal[1]*sFactor);
					rec->norms[i][j][Z] = GLshort(LightingNormal[2]*sFactor);
				}
				// do the last point
				Side[0] = rec->discs[i][nFacets-1][X] - rec->discs[i][0][X];
				Side[1] = rec->discs[i][nFacets-1][Y] - rec->discs[i][0][Y];
				Side[2] = rec->discs[i][nFacets-1][Z] - rec->discs[i][0][Z];

				CalcNormal(Forward, Side, LightingNormal);
/*				rec->norms[i][0].nx = short(LightingNormal[0]*sFactor);
				rec->norms[i][0].ny = short(LightingNormal[1]*sFactor); 
				rec->norms[i][0].nz = short(LightingNormal[2]*sFactor);*/
				rec->norms[i][0][X] = GLshort(LightingNormal[0]*sFactor);
				rec->norms[i][0][Y] = GLshort(LightingNormal[1]*sFactor); 
				rec->norms[i][0][Z] = GLshort(LightingNormal[2]*sFactor);
			}
		}
	}
	globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
int DataLines::CalcNormal(Point3f v1, Point3f v2, Point3f result)
{
	// do the cross product defined by the vectors
	// v1 - v2

	float length;

	// calculate the cross product

	result[X] = v1[Y]*v2[Z] - v1[Z]*v2[Y];
	result[Y] = v1[Z]*v2[X] - v1[X]*v2[Z];
	result[Z] = v1[X]*v2[Y] - v1[Y]*v2[X];

	// and normalize to a unit vector
	length = (float)sqrt(result[0]*result[0] 
		+ result[1]*result[1] + result[2]*result[2]);

	if (length < ZERO_TOLERANCE)
		return GL_FALSE;

	result[X] /= length;
	result[Y] /= length;
	result[Z] /= length;

	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/** This procedure calculates the circle points based on nFacets, and stores
 *	the x and y values in new arrays, passed back through the x and y
 *	argument pointers.
 */
void DataLines::CalculateCircles(float *&circX, float *&circY)
{
	float angleIncrement = (float)(2.0 * PI / NFACETS);
	int i;

	//Calculate the first disk;
	circX = new float[NFACETS+1];
	circY = new float[NFACETS+1];

	for(i = 0; i < NFACETS; ++i)
	{
		circX[i] = float(sin(i * angleIncrement));
		circY[i] = float(cos(i * angleIncrement));
	}
	circX[NFACETS] = circX[0];
	circY[NFACETS] = circY[0];
}

/*--------------------------------------------------------------------------*/
void DataLines::UpdateTexture(TextureMap *ptr)
{
	unsigned int i, j, numPoints;
	LineRecord *rec;
	float z = 0.0;

	if(!lineRecords)
		return;
	
	if (colorFlags & COLOR_ATTRIBUTE)
	{
		if (colorFlags & COLOR_AS_HEIGHT)
			z = pose->getPosition()[2];

		for (i = 0; i < numRecords; i++)
		{
			rec = lineRecords[i];
			if (rec->bytewiseColor.empty())
				continue;
			if (colorFlags & FORMAT_BY_RECORD)
				numPoints = 1;
			else
				numPoints = rec->numPoints;
			for (j = 0; j < numPoints; j++)
				rec->bytewiseColor[j] = GetColorAtValue(rec->colorVal[j] + z);
		}
	}
}


/*---------------------------------------------------------------------------*/
int DataLines::Init(int num)
{
    unsigned int i,j;
	gz4d::Point<double> point;
	
	if(numRecords>0)
	{
		return TCL_ERROR;
	}

	displayTube = OFF;
	displayLine = ON;
	sizeFlags = FORMAT_BY_RECORD;
	colorFlags = FORMAT_BY_RECORD | COLOR_RGB;
	degradationFlag = NONE;

	if(num > 0)
	{
		numRecords = num;
		lineRecords = new LineRecord *[numRecords];
		size = new float[numRecords];
		for (i = 0; i < numRecords; i++)
		{
			lineRecords[i] = new LineRecord(2, colorFlags, sizeFlags, OFF,0);
			for (j = 0; j < 2; j++)
			{
				point = lineRecords[i]->positions[0];
				point[X] = point[Y] = point[Z] = 0.0f;
			}

			lineRecords[i]->rgbColor = new float*[1];
			lineRecords[i]->rgbColor[0] = new Point3f;
			lineRecords[i]->rgbColor[0][X] = lineRecords[i]->rgbColor[0][Y] = lineRecords[i]->rgbColor[0][Z] = 1.0f;

  			size[i] = 1;
		}
	}

	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int DataLines::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	unsigned int i, j, arg;

	const char *message = argv[0];
	gz4d::Point<double> point;
	
	if (argc >= 1)
	{
		if (!strcmp(message, "applyTransforms"))
		{
			applyTransformsToPoints();
			return TCL_OK;
		}
		//----- init -----
		if (!strcmp(message, "init"))
		{
     	if(argc >= 2)
     	{
     		return Init(atoi(argv[1]));
     	}
     	else
     	{
     		return TCL_ERROR;
     	}
		}
		//----- position -----
		if (!strcmp(message, "position"))
		{
			if(argc >= 8)
			{
				i = atoi(argv[1]); 
				if (i >= numRecords)
				{
					return TCL_ERROR;
				}
				arg = 2;
				for (j = 0; j < 2; j++)
				{
					point = lineRecords[i]->positions[j];
					point[X] = atof(argv[arg++]);
					point[Y] = atof(argv[arg++]);
					point[Z] = atof(argv[arg++]);
				}
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
			if(argc >= 5)
			{
				i = atoi(argv[1]);
				if(i >= numRecords)
				{
					return TCL_ERROR;
				}
				lineRecords[i]->rgbColor[0][X] = (float) atof(argv[2]);
				lineRecords[i]->rgbColor[0][Y] = (float) atof(argv[3]);
				lineRecords[i]->rgbColor[0][Z] = (float) atof(argv[4]);
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
				if(atoi(argv[1]) >= int(numRecords))
				{
					return TCL_ERROR;
				}
				size[atoi(argv[1])] = (float) atof(argv[2]);
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


/*-----------------------InterpolationSupport Functions---------------------*/
/*--------------------------------------------------------------------------*/
std::vector<gz4d::Point<double> > &DataLines::getPoints(unsigned int whichLine)
{
    return lineRecords[whichLine]->positions;
}

/*---------------------------------------------------------------------------*/
GLboolean DataLines::getPoint(unsigned int index, Point3d& position, unsigned int whichLine)
{
	LineRecord *rec;

	if (whichLine >= 0 && whichLine < numRecords)
	{
		rec = lineRecords[whichLine];
		COPY_POINT(position, rec->positions[index]);
		return GL_TRUE;
	}
	return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
int DataLines::getNumPoints(unsigned int whichLine)
{
	if (whichLine >= 0 && whichLine < numRecords)
		return lineRecords[whichLine]->numPoints;
	return -1;
}

/*--------------------------------------------------------------------------*/
GLboolean DataLines::modifyPoint(unsigned int index, const Point3d newValue,
								 long &changeMask, unsigned int whichLine)
{
	LineRecord *rec;

	if (whichLine >= 0 && whichLine < numRecords)
	{
		rec = lineRecords[whichLine];
		COPY_POINT(rec->positions[index], newValue);
		if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
		{
			rec->colorVal[index] = newValue[Z];
			rec->bytewiseColor[index] = GetColorAtValue(newValue[Z] + pose->getPosition()[Z]);
		}
		if (displayTube != OFF)
			updatePoints(whichLine, index - 1, index + 1);
		setEdited();
		return GL_TRUE;
	}
	return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
GLboolean DataLines::modifyPoint(unsigned int index, const Point3f newValue,
								 long &changeMask, unsigned int whichLine)
{
	LineRecord *rec;

	if (whichLine >= 0 && whichLine < numRecords)
	{
		rec = lineRecords[whichLine];
		COPY_POINT(rec->positions[index], newValue);
		if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
		{
			rec->colorVal[index] = newValue[Z];
			rec->bytewiseColor[index] = GetColorAtValue(newValue[Z] + pose->getPosition()[Z]);
		}
		if (displayTube != OFF)
			updatePoints(whichLine, index - 1, index + 1);
		setEdited();
		return GL_TRUE;
	}
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
int DataLines::getAttribute(PosableStateWrapper *wrapper, SmartValue &val, int attributeIndex)
{
    unsigned int lineIndex, index, i1, i2;
	SmartMemory::IndexType type;

	lineIndex = wrapper->getIntegerIndex(2);
	if (lineIndex >= numRecords)
		return TCL_ERROR;
	index = (int)wrapper->getFloatIndex();

	switch (attributeIndex)
	{
		case WAttr_SIZE:
			if (sizeFlags & FORMAT_BY_RECORD)
				val.set(size[lineIndex]);
			else if (sizeFlags & FORMAT_BY_COLUMN)
				val.set(lineRecords[lineIndex]->size[index]);
			else
				val.set(sizeFactor);
			break;
		case WAttr_TEXT:
			if (textFlags == FORMAT_NONE)
				val.set("<none>");
			else
				val.set(text[lineIndex], GL_TRUE);
			break;
		case WAttr_ROLL:
			if (hasRoll)
				val.set(lineRecords[lineIndex]->roll[index]);
			else
				val.set("<none>");
			break;
		default:
			attributeIndex -= WAttr_DATA;
			if (attributeIndex < 0 || attributeIndex >= int(numDataFields))
				return TCL_ERROR;
			else
			{
				type = dataRepository[attributeIndex].type;
				if (dataRepository[attributeIndex].format == FORMAT_BY_RECORD)
				{
					i1 = 0;
					i2 = lineIndex;
				}
				else
				{
					i1 = lineIndex;
					i2 = index;
				}
				if (type == SmartMemory::String)
					val.set(dataRepository[attributeIndex].data[i1][i2].asString());
				else
				{
					if (wrapper->getIndexType() == SmartValue::Float) //no need to interpolate unless index is a float!
					{
						if (InterpolationSupport::getAttribute(wrapper, val, attributeIndex) == TCL_OK)
							return TCL_OK;
					}
					val.set(dataRepository[attributeIndex].data[i1][i2].asFloat());
				}
			}
	}
	return TCL_OK;
}

/*---------------------------------------------------------------------------*/
/**	If the attributeIndex is valid, sets things up to service calls to
 *	setWrapperIndexFromAttribute and returns GL_TRUE.  Otherwise returns
 *	GL_FALSE.
 */
GLboolean DataLines::requestAssociativeAttribute(int attributeIndex)
{
	if (attributeIndex < WAttr_DATA)
		return GL_FALSE;

	return InterpolationSupport::requestAssociativeAttribute(attributeIndex);
}

/*---------------------------------------------------------------------------*/
void DataLines::resetTimeBoundsOnAttribute(PosableStateWrapper *wrapper, int indexingAttribute, double offset, double scale)
{
    unsigned int lineIndex;
	float *array;

	if (indexingAttribute < WAttr_DATA)
		return;

	lineIndex = wrapper->getIntegerIndex(2);
	if (lineIndex >= 0 && lineIndex < numRecords)
	{
		array = dataRepository[indexingAttribute - WAttr_DATA].data[lineIndex].array.f;
		bbox.start = array[0] + offset;
		bbox.end = array[lineRecords[lineIndex]->numPoints - 1] * scale + offset;
	}
}

/*---------------------------------------------------------------------------*/
float *DataLines::getAttributeValues(unsigned int whichLine, int attributeIndex)
{
	if (attributeIndex < 0)
		attributeIndex = associativeAttribute - WAttr_DATA;

	if (attributeIndex < 0 || dataRepository[attributeIndex].type != SmartArray::Float ||
		dataRepository[attributeIndex].format != FORMAT_BY_COLUMN)
		return NULL;

	return dataRepository[attributeIndex].data[whichLine].array.f;
}

/*---------------------------------------------------------------------------*/
int DataLines::setAttribute(PosableStateWrapper *wrapper, SmartValue &val, int attributeIndex)
{
    unsigned int lineIndex, index, i1, i2, numPoints;
	SmartMemory::IndexType type;
	LineRecord *rec = NULL;

	lineIndex = wrapper->getIntegerIndex(2);
	index = (int)wrapper->getFloatIndex();

	switch (attributeIndex)
	{
		case WAttr_SIZE:
			if (sizeFlags & FORMAT_BY_RECORD)
			{
				size[lineIndex] = val.asFloat();
				updatePoints(lineIndex, 0, lineRecords[lineIndex]->numPoints - 1);
			}
			else if (sizeFlags & FORMAT_BY_COLUMN)
			{
				lineRecords[lineIndex]->size[index] = val.asFloat();
				updatePoints(lineIndex, index, index);
			}
			else
			{
				sizeFactor = val.asFloat();
				for (lineIndex = 0; lineIndex < numRecords; lineIndex++)
					updatePoints(lineIndex, 0, lineRecords[lineIndex]->numPoints - 1);
			}
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
			break;
		case WAttr_TEXT:
			if (!strcmp(val.asString(), "<none>")) //don't update if <none>
				break;
			if (textFlags == FORMAT_NONE)
			{
				textFlags = FORMAT_BY_RECORD;
				text = new char *[numRecords];
				for (i1 = 0; i1 < numRecords; i1++)
					text[i1] = NULL;
			}
			if (text[lineIndex])
				delete text[lineIndex];
			text[lineIndex] = strdup(val.asString());
			break;
		case WAttr_ROLL:
			if (!strcmp(val.asString(), "<none>")) //don't update if <none>
				break;
			if (!hasRoll)
			{
				hasRoll = GL_TRUE;
				for (i1 = 0; i1 < numRecords; i1++)
				{
					rec = lineRecords[i1];
					numPoints = rec->numPoints;
					rec->roll = new float[numPoints];
					for (i2 = 0; i2 < numPoints; i2++)
						rec->roll[i2] = 0.0;
				}
			}
			lineRecords[lineIndex]->roll[index] = val.asFloat();
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
			break;
		default:
			attributeIndex -= WAttr_DATA;
			if (attributeIndex < 0 || attributeIndex >= (int)numDataFields)
				return TCL_ERROR;
			else
			{
				type = dataRepository[attributeIndex].type;
				if (dataRepository[attributeIndex].format == FORMAT_BY_RECORD)
				{
					i1 = 0;
					i2 = lineIndex;
				}
				else
				{
					i1 = lineIndex;
					i2 = index;
				}
				if (type == SmartMemory::String)
					dataRepository[attributeIndex].data[i1][i2].set(val.asString());
				else
					dataRepository[attributeIndex].data[i1][i2].set(val.asFloat());
			}
	}
	return InterpolationSupport::setAttribute(wrapper, val, attributeIndex);
}

/*---------------------------------------------------------------------------*/
void DataLines::updateTrailingIndexes(unsigned int line)
{
	double now, startTime;
        unsigned int numPoints;

	if (line < numRecords)
		numPoints = lineRecords[line]->numPoints;
	else
		numPoints = 0;

	now = (Scene3D::timeControl->getGZTime() - timeAttributeOffset) / timeAttributeScale;
	startTime = now - trailingSeconds;

	while (dataRepository[timeAttributeIndex].data[line][trailingStartIndex].asFloat() < startTime)
	{
		if (++trailingStartIndex >= numPoints)
		{
			trailingStartIndex = numPoints - 1;
			break;
		}
	}
	while (dataRepository[timeAttributeIndex].data[line][trailingStartIndex].asFloat() > startTime)
	{
		if (--trailingStartIndex <= 0)
		{
			trailingStartIndex = 0;
			break;
		}
	}

	while (dataRepository[timeAttributeIndex].data[line][trailingEndIndex].asFloat() > now)
	{
		if (--trailingEndIndex <= 0)
		{
			trailingEndIndex = 0;
			break;
		}
	}
	while (dataRepository[timeAttributeIndex].data[line][trailingEndIndex].asFloat() <= now)
	{
		if (++trailingEndIndex >= numPoints)
		{
			trailingEndIndex = numPoints - 1;
			break;
		}
	}
}

/*--------------------------------------------------------------------------*/
GLboolean DataLines::insertItem(int what, unsigned int index, unsigned int whichLine)
{
	LineRecord **newRecords;
        unsigned int i, j, newLength, copyPoint, len, oldLength;
	LineRecord *rec;
	float newVal;

	if (what == LineType)
	{
		oldLength = numRecords;
		if (whichLine < numRecords)
		{
			newLength = numRecords + 1;
			copyPoint = whichLine;
		}
		else
		{
			newLength = whichLine + 1;
			copyPoint = numRecords;
		}

		//create a new lineRecord array if we're inserting a line
		if (newLength > allocatedRecords)
		{
			newRecords = new LineRecord *[newLength];
			//copy everything to insertion point
			for (i = 0; i < copyPoint; i++)
				newRecords[i] = lineRecords[i];
		}
		else
			newRecords = lineRecords;

		//copy everything past insertion point
		for (i = numRecords; i > copyPoint; i--)
			newRecords[i] = lineRecords[i - 1];
		//insertion in middle or at end
		if (index <= 0)
			index = 1;
		newRecords[copyPoint] = rec = new LineRecord(index + 1, colorFlags, sizeFlags, (displayTube != OFF), nFacets, hasRoll);
		newVal = 0.0;
		for (i = 0; i <= index; i++)
		{
			if (colorFlags & FORMAT_BY_COLUMN)
			{
				if (!rec->bytewiseColor.empty())
                                {
					//rec->bytewiseColor[i] = 0;
                                }
				else if (colorFlags & COLOR_CODE)
				{
					rec->indexColor[i] = 0;
					rec->rgbColor[i] = palette[0];
				}
				else if (colorFlags & COLOR_TEXTURE)
				{
					rec->colorVal[i] = 0.0;
					if (colorFlags & COLOR_TEXTURE_T)
						rec->texture_t[i] = 0.0;
				}
				else
				{
					COPY_POINT(rec->rgbColor[i], palette[0]);
				}
			}
			rec->positions[i][X] = rec->positions[i][Y] = rec->positions[i][Z] = newVal;
			if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
				rec->colorVal[i] = newVal;
			if (sizeFlags & FORMAT_BY_COLUMN)
				rec->size[i] = 1.0;
			newVal += 1.0f;
		}

		//insertion past end
		for (i = numRecords + 1; i < newLength; i++)
		{
			newRecords[i] = rec = new LineRecord(index + 1, colorFlags, sizeFlags, (displayTube != OFF), nFacets, hasRoll);
			newVal = 0.0;
			for (j = 0; j <= index; j++)
			{
				if (colorFlags & FORMAT_BY_COLUMN)
				{
					if (!rec->bytewiseColor.empty())
                                        {
						//rec->bytewiseColor[i] = 0;
                                        }
					else if (colorFlags & COLOR_CODE)
					{
						rec->indexColor[j] = 0;
						rec->rgbColor[j] = palette[0];
					}
					else if (colorFlags & COLOR_TEXTURE)
					{
						rec->colorVal[i] = 0.0;
						if (colorFlags & COLOR_TEXTURE_T)
							rec->texture_t[i] = 0.0;
					}
					else
					{
						COPY_POINT(rec->rgbColor[j], palette[0]);
					}
				}
				rec->positions[j][X] = rec->positions[j][Y] = rec->positions[j][Z] = newVal;
				if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
					rec->colorVal[j] = newVal;
				if (sizeFlags & FORMAT_BY_COLUMN)
					rec->size[i] = 1.0;
				newVal += 1.0f;
			}
		}

		if (newRecords != lineRecords)
		{
			delete []lineRecords;
			lineRecords = newRecords;
		}

		insertRecord(whichLine); //do standard GeoData insertion.

		//update tubes
		updatePoints(whichLine, 0, lineRecords[whichLine]->numPoints - 1);
		for (i = oldLength; i < newLength; i++)
			updatePoints(i, 0, lineRecords[i]->numPoints - 1);

		UpdateTexture(NULL);
		return GL_TRUE;
	}
	if (what == PointType)
	{
		len = lineRecords[whichLine]->numPoints - 1;
		lineRecords[whichLine]->insertPoint(index, nFacets, colorFlags);
		if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
			lineRecords[whichLine]->bytewiseColor[index] = GetColorAtValue(lineRecords[whichLine]->colorVal[index] + pose->getPosition()[Z]);

		//data
		for (i = 0; i < numDataFields; i++)
		{
			if (dataRepository[i].format & FORMAT_BY_COLUMN)
				dataRepository[i].data[whichLine].insertPosition(index);
			else
				dataRepository[i].data->insertPosition(index);
		}

		if (index > len)
			updatePoints(whichLine, len - 1, index);
		else
			updatePoints(whichLine, index - 1, index + 1);
		if (selectedRecord == (int)whichLine && selectedPoint >= (int)index)
			selectedPoint++;
		UpdateTexture(NULL);
		return GL_TRUE;
	}
	return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
GLboolean DataLines::deleteItem(int what, unsigned int index, unsigned int whichLine)
{
    unsigned int i, oldNumRecords;

	if (what == LineType)
	{
		if (whichLine < numRecords && whichLine >= 0)
		{
			if (numRecords <= 1) //don't allow deletion of last line
				return GL_FALSE;
			oldNumRecords = numRecords;
			deleteRecord(whichLine); //do standard GeoData deletion.
			delete lineRecords[whichLine];
			for (i = whichLine + 1; i < oldNumRecords; i++)
				lineRecords[i - 1] = lineRecords[i];
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
			return GL_TRUE;
		}
	}
	if (what == PointType)
	{
		if (whichLine < numRecords && whichLine >= 0 &&
			index < lineRecords[whichLine]->numPoints && index >= 0)
		{
			if (lineRecords[whichLine]->numPoints <= 2) //don't allow deletion of last 2
				return GL_FALSE;
			lineRecords[whichLine]->deletePoint(index, colorFlags);

			//data
			for (i = 0; i < numDataFields; i++)
			{
				if (dataRepository[i].format & FORMAT_BY_COLUMN)
					dataRepository[i].data[whichLine].deletePosition(index);
				else
					dataRepository[i].data->deletePosition(index);
			}

			updatePoints(whichLine, index - 1, index + 1);
			if (selectedRecord == (int)whichLine)
			{
				if (selectedPoint > (int)index)
					selectedPoint--;
				else if (selectedPoint == (int)index)
					selectedPoint = -1;
			}
		}
		return GL_TRUE;
	}
	return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
Object * DataLines::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    string text;

    GeoData::LoadXML(xml,path, ncfile);

    boost::optional<std::string> attribute;
    boost::optional<float> float_attrib;
    
    if(float_attrib = xml.get_optional<float>("<xmlattr>.size"))
    {
        sizeFactor = *float_attrib;
        sizeFlags = FORMAT_CONSTANT;
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.datafile"))
        datafilename = *attribute;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.datatype"))
        if(*attribute == "Remus")
            datatype = Remus;

    if(!datafilename.empty())
    {
        datafile = fopen((fs::path(path)/datafilename).string().c_str(),"r");
        if(!datafile)
        {
            cerr << "DataLines: error opening datafile: " << datafilename.string() << endl;
        }
        else if(datatype == Remus)
        {
            projectionRequired = GL_TRUE;
        }
    }

    return this;
}

/*--------------------------------------------------------------------------*/
void DataLines::Idle(double time)
{
	Point3d p;
	long int cm;
	float value;
	char line[2048];
	double x,y,z;


	Object::Idle(time);


	if(datafile && datatype==Remus)
	{
		if(feof(datafile))
		{
			fclose(datafile);
			datafile = NULL;
		}
		else if(fgets(line,2048,datafile))
		{
			if(sscanf(line,"%lf, %lf, %*i:%*i:%*f, %lf, %*f, %f,",&y, &x,  &z, &value)==4)
			{
				if(getNumPoints()==-1)
				{
					displayTube = ON;
					colorFlags = COLOR_TEXTURE | FORMAT_BY_COLUMN;
					insertLine(0);
				}
				z = -z;
				projectXY(1,&x,&y,GL_FALSE);
				p[0] = x;
				p[1] = y;
				p[2] = z;
				cm = ChangeScaleMask;
			}
		}
	}
}
