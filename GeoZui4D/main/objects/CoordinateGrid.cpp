// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// CoordinateGrid.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing coordinate grids and axes.
 *	This class implements the following responsibilities:
 *		* Read grid/axes description from a file
 *		* Support viewing of the grid/axes.
 *
 *	Created by mdp 6/11/2001
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
//#include "MainWindow.h"
//#include "engine/Scene3D.h"
#include "engine/PosableStateBase.h"
#include "CoordinateGrid.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
//#include <iostream>
#include <math.h>

using namespace std;

extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(CoordinateGrid, "cga");

const char *CoordinateGrid::CoordinateGridNames[] =
			{OBJECT_NAMES, "DIMENSIONS", "REFERENCE", "BACKGROUND",
			 "BGCOLOR", "LINECOLOR", "BACKGROUND_OFFSET",
			 "LINES", "TICKS", "LABELS", "TICKS_LABELS",
			 "GRID", "LOCAL", "GLOBAL", "MIN", "THROUGH", "MAX",
			 "MIN_MAX", "SPACING_EVEN", "SPACING_DYNAMIC",
			 "SPACING_XYZ", "FRAGILE", "TRANSIENT",
			 ""};

const char *CoordinateGrid::LabelFormats[] =
			{"%.2f m", "%.0f m", "%.1f km", "%.0f km"};

#define MAX_GLOBAL_STEP 100000

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
CoordinateGrid::CoordinateGrid()
{
	SetTypeAndVersion("COORD", 1, 0);
	SetKeywordTable(CoordinateGridNames);

	dimensionality = DIM_NONE;
	size[X] = size[Y] = size[Z] = 0.0;

	backgroundOffset = 0.0;

	currentSpec = NULL;
	lastSpec = NULL;
	compiledSpecs = NULL;
	numPlanes = 0;
	planeSpecs = NULL;
	planePoints = NULL;

	bgColor[0] = bgColor[1] = bgColor[2] = 0.0;
	fgColor[0] = fgColor[1] = fgColor[2] = 1.0;
}

/*--------------------------------------------------------------------------*/
CoordinateGrid::~CoordinateGrid()
{
	CompiledSpec *temp;

	while (compiledSpecs)
	{
		temp = compiledSpecs;
		compiledSpecs = temp->next;
		delete temp;
	}
	if (planeSpecs)
		delete [] planeSpecs;
	if (planePoints)
		delete [] planePoints;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void CoordinateGrid::DrawLowRes(float degradation, Window3D *win)
{
	ViewSpec viewSpec;

	generateViewSpec(win->getViewInfo(), viewSpec);

	DrawPlanes();
	currentSpec = compiledSpecs;
	while (currentSpec)
	{
		if (currentSpec->robustness != FRAGILE)
			DrawDynamicOrNot(win, viewSpec);
		currentSpec = currentSpec->next;
	}
	glColor3f(1.0,1.0,1.0);
	glLineWidth(1.0);

	ResetHighRes();
}

/*--------------------------------------------------------------------------*/
int CoordinateGrid::DrawHighRes(int eye, Window3D *win)
{
	ViewSpec viewSpec;

	if (!doneFlag)
	{
		generateViewSpec(win->getViewInfo(), viewSpec);

		DrawPlanes();
		if (!currentSpec)
		{
			doneFlag = 1;
			return doneFlag;
		}
		if (currentSpec->robustness != TRANSIENT)
			DrawDynamicOrNot(win, viewSpec);

		if (eye == dominantEye || eye == MONO_EYE)
		{
			currentSpec = currentSpec->next;
		}
		glColor3f(1.0,1.0,1.0);
		glLineWidth(1.0);
	}
	else
		doneFlag = 1;
	
	return doneFlag;
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::DrawDynamicOrNot(Window3D *win, ViewSpec &viewSpec)
{
	float step, minStop, maxStop, temp, lineSize, alongStep;

    step = 0.0;
    
	if (!currentSpec->dynamicSpacing)
	{
		factorLevel = 1.0;
		glLineWidth(1.0);
		DrawSpec(win);
	}
	else //dynamic spacing
	{
		//what's the outermost step bounded by?
		if (reference == GLOBAL)
			maxStop = MAX_GLOBAL_STEP;
		else
			maxStop = maxSize;
		//now deterimine the innermost step.
		alongStep = currentSpec->alongStep;
		minStop = 15 * viewSpec.pixelSizeAtScreen;
		if (alongStep < minStop)
		{
			for (step = alongStep; step < minStop; step *= alongStep)
				continue;
		}
		else
		{
			for (temp = alongStep; temp >= minStop; temp /= alongStep)
				step = temp;
		}
		factorLevel = step / alongStep;
		//draw from smallest to largest.
		lineSize = 1.0f;
		while (step < maxStop)
		{
			generateLoops(*currentSpec, step, viewSpec);
			glLineWidth(lineSize);
			DrawSpec(win);
			lineSize += 1.0f; //alongStep * 0.1f;
			step *= alongStep;
			factorLevel *= alongStep;
		}
		currentSpec->alongStep = alongStep;
	}
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::DrawPlanes()
{
	int i, numPoints = 4 * numPlanes;

	if (!numPoints)
		return;

	glColor3fv(bgColor);
	glBegin(GL_QUADS);
	for (i = 0; i < numPoints; i++)
		glVertex3fv(planePoints[i]);
	glEnd();
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::DrawSpec(Window3D *win)
{
	int alongDim, dim0, dim1, labelIndex;
	Point3f point, tickPoint;
	float val0, val1, val2, outval;
	float start0, end0, step0, start1, end1, step1;
	float alongStart, alongEnd, alongStep;
	float distance, halfDistance, labelDistance;
	int type = currentSpec->type;
	char label[32];

	//set up some often-used variables to speed the loop.
	distance = factorLevel * currentSpec->distance;
	halfDistance = distance / 2.0f;
	labelDistance = halfDistance + factorLevel / 10.0f;
	alongDim = currentSpec->alongDim;
	dim0 = currentSpec->dim0;
	dim1 = currentSpec->dim1;
	start0 = currentSpec->start0;
	end0 = currentSpec->end0;
	step0 = currentSpec->step0;
	start1 = currentSpec->start1;
	end1 = currentSpec->end1;
	step1 = currentSpec->step1;
	alongStart = currentSpec->alongStart;
	alongEnd = currentSpec->alongEnd;
	alongStep = currentSpec->alongStep;
	labelIndex = currentSpec->labelIndex;

	glColor3fv(fgColor);
	glNormal3f(0.0, 0.0, 1.0);

	//Draw things.
	if (type == LINES)
		glBegin(GL_LINES);
	for (val0 = start0; val0 <= end0; val0 += step0)
	{
		point[dim0] = val0;
		for (val1 = start1; val1 <= end1; val1 += step1)
		{
			point[dim1] = val1;
			if (type == LINES)
			{
				point[alongDim] = alongStart;
				glVertex3fv(point);
				point[alongDim] = alongEnd;
				glVertex3fv(point);
			}
			else
			{
				for (val2 = alongStart; val2 <= alongEnd; val2 += alongStep)
				{
					point[alongDim] = val2;
					tickPoint[alongDim] = point[alongDim];
					if (type == TICKS || type == TICKS_LABELS)
					{
						glBegin(GL_LINES);
						tickPoint[dim1] = point[dim1];
						tickPoint[dim0] = point[dim0] - halfDistance;
						glVertex3fv(tickPoint);
						tickPoint[dim0] += distance;
						glVertex3fv(tickPoint);
						tickPoint[dim0] = point[dim0];
						tickPoint[dim1] = point[dim1] - halfDistance;
						glVertex3fv(tickPoint);
						tickPoint[dim1] += distance;
						glVertex3fv(tickPoint);
						if (val0 != start0 && val0 != end0)
						{
							tickPoint[dim1] = point[dim1];
							tickPoint[alongDim] = point[alongDim] - halfDistance;
							glVertex3fv(tickPoint);
							tickPoint[alongDim] += distance;
							glVertex3fv(tickPoint);
						}
						glEnd();
					}
					if (type == LABELS || type == TICKS_LABELS)
					{
						if (val0 > start0)
						{
							if (val0 <= end0 - step0) //don't draw labels THROUGH
								continue;
							tickPoint[dim0] = point[dim0] + labelDistance;
						}
						else
							tickPoint[dim0] = point[dim0] - labelDistance;
						tickPoint[dim1] = point[dim1];
						if (val1 > start1)
						{
							if (val1 <= end1 - step1) //dont' draw labels THROUGH
								continue;
							if (alongDim == Z)
								tickPoint[dim1] += distance;
						}
						else if (alongDim == Z)
							tickPoint[dim1] -= distance;
						if (val2 <= 0.0)
							tickPoint[alongDim] -= labelDistance;
						if (reference == LOCAL) //what do we write?
							outval = val2;
						else
							outval = (float)(GetPosition(alongDim) + val2);
						if (labelIndex >= 2)
							outval /= 1000.0f;
						sprintf(label, LabelFormats[labelIndex], outval);
						glColor3fv(fgColor);
						win->printAt(tickPoint[X], tickPoint[Y], tickPoint[Z],
									 label);
					}
				}
			}
		}
	}
	if (type == LINES)
		glEnd();
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void CoordinateGrid::ResetHighRes()
{
	doneFlag = 0;
	currentSpec = compiledSpecs;
}

/*--------------------------------------------------------------------------*/
int CoordinateGrid::processField(int option, char **fields)
{
	int i = 1;
	const char *dimensions;
	int j, dim;

    dim = 0;
    
	switch(option)
	{
		case DIMENSIONS:
			dimensions = fields[i++];
			maxSize = 0.0;
			while (*dimensions)
			{
				switch (*dimensions)
				{
					case 'X':
						dimensionality |= DIM_X;
						dim = X;
						break;
					case 'Y':
						dimensionality |= DIM_Y;
						dim = Y;
						break;
					case 'Z':
						dimensionality |= DIM_Z;
						dim = Z;
						break;
				}
				size[dim] = ConvertFloat(i);
				if (size[dim] > maxSize)
					maxSize = size[dim];
				dimensions++;
			}
			break;
		case REFERENCE:
			reference = Lookup(fields[i++]);
			break;
		case BACKGROUND:
			numPlanes = ConvertInt(i);
			planeSpecs = new int[numPlanes];
			for (j = 0; j < numPlanes; j++)
			{
				dimensions = fields[i++];
				planeSpecs[j] = DIM_NONE;
				if (!dimensions)
				{
					cerr << "Not enough plane specifications.  "
						 << numPlanes << " expected, but only "
						 << j << " found." << endl;
					numPlanes = j;
					break;
				}
				while (*dimensions && *dimensions != 'M')
				{
					switch (*dimensions)
					{
						case 'X':
							planeSpecs[j] |= DIM_X;
							break;
						case 'Y':
							planeSpecs[j] |= DIM_Y;
							break;
						case 'Z':
							planeSpecs[j] |= DIM_Z;
							break;
					}
					dimensions++;
				}
				if (strcmp(dimensions, "MIN") == 0)
					planeSpecs[j] |= DIM_MIN;
				else
					planeSpecs[j] |= DIM_MAX;
			}
			break;
		case BGCOLOR:
			ConvertFloatPoint(bgColor, i);
			break;
		case LINECOLOR:
			ConvertFloatPoint(fgColor, i);
			break;
		case BACKGROUND_OFFSET:
			backgroundOffset = ConvertFloat(i);
			break;
		case PROJECT: case REVERSED_XY:
			cerr << fields[0] << " is not yet supported by CoordinateGrid." << endl;
			return 1;
			break;
		default:
			return Object::processField(option, fields);
			break;
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int CoordinateGrid::readFrom(FILE *infile)
{
	char **fields;
	int i, secondaries, seenDims, item;
	const char *dimensions;
	CoordSpec newSpec;
	GLboolean seenDash, labelsOrTicks;
	int robustness;
	
	//parse header
	if (Object::readFrom(infile))
		return 1;

	//parse coord-specs.
	for (fields = NextLine(); fields; fields = NextLine())
	{
		i = 0;
		item = Lookup(fields[i++]);
		if (item == FRAGILE || item == TRANSIENT)
		{
			robustness = item;
			item = Lookup(fields[i++]);
		}
		else
			robustness = THROUGH;
		if (item < LINES || item > GRID)
		{
			cerr << "Invalid coordinate specifier.  Must be one of "
				 << "LINES, TICKS, LABELS, TICKS_LABELS, or GRID."
				 << endl;
			break;
		}
		if (item == TICKS || item == LABELS || item == TICKS_LABELS)
			labelsOrTicks = GL_TRUE;
		else
			labelsOrTicks = GL_FALSE;

		//fill the spec.
		newSpec.type = item;
		newSpec.robustness = robustness;
		//get direction
		newSpec.direction = seenDims = DIM_NONE;
		dimensions = fields[i++];
		secondaries = 0;
		seenDash = GL_FALSE;
		while (*dimensions)
		{
			switch(*dimensions)
			{
				case 'X':
					if (!seenDash)
						newSpec.direction |= DIM_X;
					if (seenDash || (item == GRID))
					{
						newSpec.direction |= DIM_THRU_X;
						secondaries++;
					}
					seenDims |= DIM_THRU_X;
					break;
				case 'Y':
					if (!seenDash)
						newSpec.direction |= DIM_Y;
					if (seenDash || (item == GRID))
					{
						newSpec.direction |= DIM_THRU_Y;
						secondaries++;
					}
					seenDims |= DIM_THRU_Y;
					break;
				case 'Z':
					if (!seenDash)
						newSpec.direction |= DIM_Z;
					if (seenDash || (item == GRID))
					{
						newSpec.direction |= DIM_THRU_Z;
						secondaries++;
					}
					seenDims |= DIM_THRU_Z;
					break;
				case '-':
					seenDash = GL_TRUE;
					break;
				default:
					cerr << "Invalid character '" << *dimensions
						 << "' in direction specification "
						 << fields[i - 1];
					return 1;
					break;
			}
			if (labelsOrTicks)
				break;
			dimensions++;
		}
		//get locations
		if (item == GRID)
		{
			newSpec.direction |= DIM_THROUGH;
			newSpec.locationOrthogonal = convertLocation(fields[i++])
										| (DIM_THRU_XYZ ^ seenDims);
		}
		else
		{
			newSpec.direction |= convertLocation(fields[i++]);
			if (secondaries <= 1) //TICKS, LABELS, TICKS_LABELS, LINES in one dim
			{
				//assign an arbitrary dimension as the secondary
				if (seenDims & DIM_THRU_X)
				{
					newSpec.direction |= DIM_THRU_Y;
					seenDims |= DIM_THRU_Y;
				}
				else
				{
					newSpec.direction |= DIM_THRU_X;
					seenDims |= DIM_THRU_X;
				}
				newSpec.locationOrthogonal = DIM_THRU_XYZ ^ seenDims;
				newSpec.locationOrthogonal |= convertLocation(fields[i++]);
			}
			else
				newSpec.locationOrthogonal = DIM_NONE;
		}
		//get distance, if applicable
		if (labelsOrTicks)
			newSpec.distance = ConvertFloat(i);
		//get spacing info
		newSpec.spacingType = item = Lookup(fields[i++]);
		if (item == -1)
		{
			newSpec.spacing = NULL;
			if ((newSpec.direction & DIM_THROUGH) || (newSpec.locationOrthogonal & DIM_THROUGH))
			{
				cerr << "THROUGH specified for " << CoordinateGridNames[newSpec.type]
					 << " without SPACING_EVEN, SPACING_XYZ, or SPACING_DYNAMIC."
					 << endl << "This " << CoordinateGridNames[newSpec.type]
					 << " will not be displayed.";
				continue;
			}
		}
		else
		{
			if (item == SPACING_XYZ)
			{
				newSpec.spacing = new float[3];
				ConvertFloatPoint(newSpec.spacing, i);
			}
			else
			{
				newSpec.spacing = new float;
				*newSpec.spacing = ConvertFloat(i);
			}
		}
		compileSpec(newSpec);
	}
	GeneratePlanes();

	return(0);
}

/*--------------------------------------------------------------------------*/
int CoordinateGrid::convertLocation(char *locationString)
{
	int result = DIM_NONE;

	switch (Lookup(locationString))
	{
		case MIN:
			result |= DIM_MIN;
			break;
		case MAX:
			result |= DIM_MAX;
			break;
		case MIN_MAX:
			result |= DIM_MIN_MAX;
			break;
		case THROUGH:
			result |= DIM_THROUGH;
			break;
		default:
			cerr << "Invalid location " << locationString
				 << ".  Must be one of MIN, MAX, MIN_MAX, or THROUGH"
				 << endl;
			break;
	}

	return result;
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::GeneratePlanes()
{
	int i, j, parity, point, numPoints = 4 * numPlanes;
	int dimensions[3] = {DIM_X, DIM_Y, DIM_Z};
	float location;

	if (!numPoints)
		return;

	planePoints = new Point3f[numPoints];

	point = 0;
	for (i = 0; i < numPlanes; i++)
	{
		parity = 0;
		for (j = X; j <= Z; j++)
		{
			if (planeSpecs[i] & dimensions[j])
			{
				planePoints[point]         [j] = 0.0;
				planePoints[point+1+parity][j] = 0.0;
				planePoints[point+2]       [j] = size[j];
				planePoints[point+3-parity][j] = size[j];
				parity += 2; //parity is for generating square corners properly
			}
			else
			{
				if (planeSpecs[i] & DIM_MIN)
					location = -backgroundOffset;
				else
					location = size[j] + backgroundOffset;
				planePoints[point]  [j] = location;
				planePoints[point+1][j] = location;
				planePoints[point+2][j] = location;
				planePoints[point+3][j] = location;
			}
		}
		point += 4;
	}
}

/*--------------------------------------------------------------------------*/
/**	compileSpec takes a CoordSpec, encapsulating the information from one
 *	line of the file, and generates one or more CompiledSpec's, which
 *	will later be rendered.  The format of the CoordSpec is a bit confusing,
 *	especially since there are a number of ways of representing the same
 *	structure, and the distinction between secondary dimension and
 *	orthogonal dimension is somewhat arbitrary.  But the format is
 *	essentially this:
 *	   type      -- LINES, GRID, TICKS, etc.
 *	   robustness-- FRAGILE, TRANSIENT, or robust (using THROUGH for robust)
 *	   direction -- composed of primary direction (what we're drawing
 *	                a line through ticks through, a grid through, etc.)
 *                  and a secondary direction (where this line, tick, etc.
 *	                is replicated.  The location component (DIM_MIN, etc.)
 *	                applies to the secondary direction only.
 *	   locationOrthogonal
 *	             -- Has direction and location components, similar to
 *	                the secondary direction.
 *	   distance  -- The distance of a label from an axis or the
 *	                length of a tick mark.
 *	   spacingType 
 *	             -- SPACING_EVEN, SPACING_XYZ, etc. (SPACING_DYNAMIC
 *	                not implemented yet.
 *	   spacing   -- One to three floats indicating how far apart to space
 *	                ticks, lines, etc.
 *	When compiled, this specification is transformed into one or more
 *	CompiledSpec records, which holds loop specifications for each
 *	dimension.  The format is as follows:
 *	   type      -- LINES, GRID, TICKS, etc. (same as CoordSpec)
 *	   robustness-- same as CoordSpec
 *	   distance  -- same as CoordSpec
 *	   alongDim  -- dimension as an index (X=0, Y=1, Z=2) along
 *	                which ticks, lines, etc. are drawn.
 *	   dim0, dim1-- The other dimensions (secondary and orthogonal)
 *	   alongStart, alongStep, alongEnd
 *	             -- Loop variables for generating lines, ticks, etc.
 *     start0/1, step0/1, end0/1
 *	             -- Loop variables for loops surrounding the
 *	                "along" loop.  These replicate the line/ticks/etc.
 *	                through orthogonal dimensions.
 *	   next      -- CompiledSpec's are joined in a link list, so they
 *	                will be rendered in the order they appear in the file.
 */
void CoordinateGrid::compileSpec(CoordSpec &spec)
{
	int i, j, k, location[2], dimension[2];
	int primaryDim[3] = {DIM_X, DIM_Y, DIM_Z};
	int secondaryDim[3] = {DIM_THRU_X, DIM_THRU_Y, DIM_THRU_Z};
	float start[2], end[2], step[2], val;
	int type = spec.type, direction;
	CompiledSpec *newSpec;

	if (type == GRID)
		type = LINES;

	for (i = 0; i < 3; i++) //grids get turned into two or three LINES specs.
	{
		if (!(spec.direction & primaryDim[i])) //any lines in this dim?
			continue;

		//drawing lines along primaryDim[i], through which dimensions?
		k = 0;
		for (j = 0; j < 3; j++)
		{
			if (j == i)
				continue;
			direction = spec.direction;
			if (!(spec.direction & secondaryDim[j])) //any lines through this dim?
			{
				//check orthogonal direction
				if (!(spec.locationOrthogonal & secondaryDim[j])) //find orthogonal dim
					continue;
				direction = spec.locationOrthogonal;
				if (type == LINES)
					k = 1;
			}
			location[k] = direction & DIM_LOCATION;
			dimension[k] = j;
			generateLoop(j, location[k], start[k], step[k], end[k], spec);
			k ^= 1; //if orthogonal, k goes back to 0.  If not, k goes to 1.
		}

		//we have all the info, now append new CompiledSpec on the list...
		newSpec = new CompiledSpec;
		if (!lastSpec)
			compiledSpecs = newSpec;
		else
			lastSpec->next = newSpec;
		newSpec->next = NULL;
		lastSpec = newSpec;

		//...and fill the spec
		newSpec->type = type;
		newSpec->robustness = spec.robustness;
		newSpec->distance = spec.distance;
		newSpec->alongDim = i;
		if (type == LINES)
			generateLoop(i, DIM_MIN_MAX, newSpec->alongStart,
						 newSpec->alongStep, newSpec->alongEnd, spec);
		else
			generateLoop(i, DIM_THROUGH, newSpec->alongStart,
						 newSpec->alongStep, newSpec->alongEnd, spec);
		newSpec->dim0 = dimension[0];
		newSpec->dim1 = dimension[1];
		newSpec->start0 = start[0];
		newSpec->start1 = start[1];
		newSpec->step0 = step[0];
		newSpec->step1 = step[1];
		newSpec->end0 = end[0];
		newSpec->end1 = end[1];
		newSpec->dimThrough0 = (spec.direction & DIM_THROUGH)? GL_TRUE : GL_FALSE;
		newSpec->dimThrough1 = (spec.locationOrthogonal & DIM_THROUGH)? GL_TRUE : GL_FALSE;
		if (spec.spacingType == SPACING_DYNAMIC)
		{
			if (spec.spacing[0] > 1.0)
				newSpec->alongStep = spec.spacing[0];
			else
				newSpec->alongStep = 10.0;
			newSpec->dynamicSpacing = GL_TRUE;
		}
		else
		{
			newSpec->dynamicSpacing = GL_FALSE;
			//determine how to generate labels
			val = 1.0;
			for (k = 0; k < 4; k++)
			{
				if (fmod(newSpec->alongStep, val) > 0.0)
				{
					newSpec->labelIndex = k;
					break;
				}
				if (k % 2)
					val *= 10.0f;
				else
					val *= 100.0f;
			}
		}
	}
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::generateLoop(int dimension, int location, float &start,
								  float &step, float &end,
								  CoordSpec &currentSpec)
{
	if (location == DIM_THROUGH)
	{
		if (currentSpec.spacingType == SPACING_XYZ) //set up the spacing
			step = currentSpec.spacing[dimension];
		else if (currentSpec.spacingType == SPACING_EVEN)
			step = currentSpec.spacing[0];
		if (reference == LOCAL) //set up the bounds
			start = 0.0;
		else
		{
//			start = step - (float)fmod(geoRef[dimension], step);
   			start = (step - fmodf((float)GetPosition(dimension), step));
			if (start > step)
				start -= step;
		}
		end = size[dimension];
	}
	else
	{
		step = size[dimension];
		if (location & DIM_MIN)
			start = 0.0;
		else
			start = size[dimension];
		if (location & DIM_MAX)
			end = size[dimension];
		else
			end = 0.0;
	}
	if (step <= 0.0)
		step = 1.0;
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::generateViewSpec(ViewManager *view, ViewSpec &spec)
{
	int dim;
        gz4d::Point<double> centerPoint;

	centerPoint = view->getReferencePoint();
	for (dim = X; dim <= Z; dim++)
		spec.centerPoint[dim] = (float)(centerPoint[dim] - pose->getPosition()[dim]);
	spec.centerToScreen = (float)view->getCenterToScreenDistance();
	spec.eyeToScreen = (float)view->getDistanceToScreen();
	spec.pixelSizeAtScreen = (float)view->getPixelSize();
}

/*--------------------------------------------------------------------------*/
/** This procedure fills the passed CompiledSpec with a new set of loops
 *	based on the passed step and viewSpec.  The loops are bounded by how
 *	well they'll show up in the best case, using the following formula:
 *		e / p >= 10 * (e + c + x) / n
 *	where
 *		e == eyeToScreen distance
 *		p == pixelSize at screen
 *		10== minimum number of pixels to resolve
 *		c == centerToScreen distance
 *		x == distance from center to set the bound
 *		n == new step.
 *	Solving for x, we get
 *		(e * n)/(10 * p) - e - c >= x
 *	For our purposes below, we let
 *		e/(10*p) == pixelizationFactor
 *		-(e + c) == pixelizationConstant
 */
void CoordinateGrid::generateLoops(CompiledSpec &spec, float step,
								   ViewSpec &viewSpec)
{
	float val = 1.0, stepTimes8;
	float pixelizationFactor, pixelizationConstant, limit;
	int k;

	pixelizationFactor = viewSpec.eyeToScreen/(10 * viewSpec.pixelSizeAtScreen);
	pixelizationConstant = - (viewSpec.eyeToScreen + viewSpec.centerToScreen);

	limit = pixelizationFactor * step + pixelizationConstant;
	if (limit > viewSpec.centerToScreen)
		limit /= 8; //not sure why I need this.
	else
		limit /= 8 * (limit / viewSpec.centerToScreen);
	//make the small bit grow quickly to fill center of workspace
	stepTimes8 = 8 * step;
	if (limit < stepTimes8)
	{
		if (limit < 4 * step)
			limit *= 2;
		else
			limit = stepTimes8;
	}

	//generate the loops.
	spec.alongStep = step;
	generateLimitedLoop(spec.alongDim, limit, spec.alongStart, step,
		spec.alongEnd, viewSpec, (spec.type == LINES)? GL_TRUE : GL_FALSE);

	if (spec.dimThrough0) //if DIM_THROUGH
	{
		spec.step0 = step;
		generateLimitedLoop(spec.dim0, limit, spec.start0, step,
							spec.end0, viewSpec);
	}
	if (spec.dimThrough1) //if DIM_THROUGH
	{
		spec.step1 = step;
		generateLimitedLoop(spec.dim1, limit, spec.start1, step,
							spec.end1, viewSpec);
	}

	//determine how to generate labels
	for (k = 0; k < 4; k++)
	{
		if (fmod(step, val) > 0.0)
		{
			spec.labelIndex = k;
			break;
		}
		if (k % 2)
			val *= 10.0f;
		else
			val *= 100.0f;
	}
}

/*--------------------------------------------------------------------------*/
void CoordinateGrid::generateLimitedLoop(int dimension, float limit,
										 float &start, float step, float &end,
										 ViewSpec &viewSpec, GLboolean along)
{
	float offset;

	start = viewSpec.centerPoint[dimension] - limit;
	end = viewSpec.centerPoint[dimension] + limit;
	if (start > size[dimension] || end < 0.0)
	{
		start = 0.0;
		end = 0.0;
		return;
	}
	if (start < 0.0)
		start = 0.0;
	if (reference == GLOBAL && !along)
	{
//		offset = step - (float)fmod(geoRef[dimension] + start, step);
		offset = (step - fmodf((float)GetPosition(dimension) + start, step));
		if (offset > step)
			offset -= step;
		start += offset;
	}
	else if (start != 0.0)
		start += step - (float)fmod(start, step);
	if (end > size[dimension])
		end = size[dimension];
}
