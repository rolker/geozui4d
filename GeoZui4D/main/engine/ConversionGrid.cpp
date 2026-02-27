/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ConversionGrid.cpp
//
//	This class defines a grid-reader for converting to GUTMs.
//
// Created by mdp 12/12/2001
///////////////////////////////////////////////////////////////////////////////
#include "engine/ConversionGrid.h"
#include <filesystem>
#include <gz4d/util/Utilities.h>
#include <math.h>
#include <iostream>
#include <string.h>
#include <ctype.h>

using namespace std;

enum ConversionGridParams {LAYOUT, COLS_ROWS, CELLSIZE, GRID_ORIGIN, SHAPE,
						   VALID_DATA, END_HEADER, FULLGRID, PARTIAL, VARIABLE,
						   LITERAL, ROW_MAJOR, COLUMN_MAJOR, PER_RECORD, IMPLICIT,
						   SECTION_DELIMITER, LESS_THAN, GREATER_THAN, BETWEEN,
						   INVERTED_ROWS};
const char *ConversionGridNames[] = {"LAYOUT", "COLS_ROWS", "CELLSIZE", "GRID_ORIGIN", "SHAPE",
						   "VALID_DATA", "END_HEADER", "FULLGRID", "PARTIAL", "VARIABLE",
						   "LITERAL", "ROW_MAJOR", "COLUMN_MAJOR", "PER_RECORD", "IMPLICIT",
						   "SECTION_DELIMITER", "LESS_THAN", "GREATER_THAN", "BETWEEN",
						   "INVERTED_ROWS",
						   ""};

#define ALLOCATION_QUANTUM 100;

//float ConversionGrid::NaN = (float)log(0.0);

/*-----------------------------Initialization--------------------------------*/
/*---------------------------------------------------------------------------*/
ConversionGrid::ConversionGrid()
{
	// set up the reader to look for BEGIN CONVERSION_GRID 1.0, and with set of keywords
	SetTypeAndVersion("CONVERSION_GRID", 1, 0);
	SetKeywordTable(ConversionGridNames);

	grid = NULL;
	xcoords = ycoords = NULL;
	xSpacing = ySpacing = 0.0;
	minValidValue = maxValidValue = 0.0;
	minHeight = maxHeight = 0.0;

	rowMajorOrder = GL_TRUE;
	spacingInGridData = GL_FALSE;
	implicitSpacing = GL_FALSE;
	invertedRows = GL_FALSE;
	rangeDescription = NotRanged;
	yCoordsCreated = GL_FALSE;

	shape = FULLGRID;

	numRows = numCols = 0;
	originGiven = GL_FALSE;

	sentinelToken = "END";
}

/*---------------------------------------------------------------------------*/
ConversionGrid::~ConversionGrid()
{
	delete grid;
	delete xcoords;
	delete ycoords;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This function opens this ConversionGrid's file and reads the information
 *	in it.
 */
int ConversionGrid::Load()
{
	int retval, wasOpen = GL_FALSE;
	if (!in)
	{
		if(Open())
			return 1;
	}
	else
	{
		SkipToBegin();
		wasOpen = GL_TRUE;
	}

	retval = readHeader();
	if (retval == 0)
		retval = readData();
	if (!wasOpen)
		Close();

	return retval;
}

/*---------------------------------------------------------------------------*/
int ConversionGrid::readHeader()
{
	int item, argument, variableSpacings = 0;
	char **fields = NULL;
	int headerdone = 0;
	int i;
	
	cout << "Reading " << GetFilename().string() << " ... \n";
	cout.flush();
	
	for(fields = NextLine(); !headerdone && fields != NULL; fields = NextLine())
	{
		i = 0;
		item = Lookup(fields[i++]);
		switch(item)
		{
			case LAYOUT:
				argument = Lookup(fields[i++]);
				if (argument == LITERAL)
				{
					spacingInGridData = GL_FALSE;
					rowMajorOrder = GL_TRUE;
					numRows = -1;
					numCols = -1;
				}
				else if (argument == COLUMN_MAJOR)
					rowMajorOrder = GL_FALSE;
				else
					rowMajorOrder = GL_TRUE;
				if (fields[i] && fields[i][0] == 'I') //INVERTED_ROWS
					invertedRows = GL_TRUE;
				break;
			case COLS_ROWS:
				numCols = ConvertInt(i);
				numRows = ConvertInt(i);
				break;
			case CELLSIZE:
				if (fields[i][0] == 'I') //IMPLICIT
				{
					xSpacing = 0.0;
					ySpacing = 0.0;
					implicitSpacing = GL_TRUE;
					break;
				}
				if (fields[i][0] == 'V') //VARIABLE
				{
					xSpacing = 0.0;
					i++;
					variableSpacings++;
				}
				else
					xSpacing = ConvertFloat(i);
				if (fields[i][0] == 'V') //VARIABLE
				{
					ySpacing = 0.0;
					i++;
					variableSpacings++;
				}
				else
					ySpacing = ConvertFloat(i);
				if (variableSpacings && Lookup(fields[i++]) == PER_RECORD)
					spacingInGridData = GL_TRUE;
				break;
			case GRID_ORIGIN:
				xOrigin = ConvertFloat(i);
				yOrigin = ConvertFloat(i);
				originGiven = GL_TRUE;
				break;
			case SECTION_DELIMITER:
				sentinelToken = ReleaseField(i);
				break;
			case VALID_DATA:
				argument = Lookup(fields[i++]);
				switch (argument)
				{
					case LESS_THAN:
						rangeDescription = LessThan;
						maxValidValue = ConvertFloat(i);
						break;
					case GREATER_THAN:
						rangeDescription = GreaterThan;
						minValidValue = ConvertFloat(i);
						break;
					case BETWEEN:
						rangeDescription = Between;
						minValidValue = ConvertFloat(i);
						maxValidValue = ConvertFloat(i);
						break;
					default:
						cerr << "Invalid range specifier: " << fields[i - 1];
						cerr << ".\n";
						cerr << "Must be LESS_THAN, GREATER_THAN, or BETWEEN.";
						return 1;
				}
				break;
			case SHAPE:
				shape = Lookup(fields[i++]);
				break;
			case END_HEADER:
				headerdone = 1;
				break;
			case -1:
				cerr << "Invalid keyword " << fields[i - 1] << endl;
				return 1;
				break;
		}
		if(headerdone)
			break;
    }

	return(0);	
}

/*---------------------------------------------------------------------------*/
int ConversionGrid::readData()
{
	int retval=0;

	if (implicitSpacing)
	{
		if (!numCols || !numRows)
		{
			cerr << "Missing CELLSIZE data.  CELLSIZE must be present when using CELLSIZE IMPLICIT" << endl;
			return 1;
		}
		if (shape != FULLGRID)
		{
			cerr << "SHAPE must be FULLGRID when using CELLSIZE IMPLICIT" << endl;
			return 1;
		}
		xcoords = new float[numCols];
		ycoords = new float[numRows];
	}
	else
	{
		if (rowMajorOrder)
			retval = readColumnValues();
		if (!retval && (!rowMajorOrder || !spacingInGridData))
			retval = readRowValues();
		if (!retval && (!rowMajorOrder && !spacingInGridData))
			retval = readColumnValues();
	}
	if (!retval)
		return readGridValues();
	return retval;
}

/*---------------------------------------------------------------------------*/
float *ConversionGrid::readCoordValues(int &count, long stopAtFilePos)
{
	int allocated, i, j;
	float *coords, *tempCoords;
	char token[1024], c;

	if (count > 0)
	{
		coords = new float[count];
		for (i = 0; i < count; i++)
		{
			if (feof(in))
			{
				cerr << "Unexpected end of file." << endl;
				delete coords;
				return NULL;
			}
			fscanf(in, "%f", &coords[i]);
		}
	}
	else
	{
		allocated = ALLOCATION_QUANTUM;
		coords = new float[allocated];
		i = 0;
		for (fscanf(in, "%s", token);
			 strcmp(token, sentinelToken);
			 fscanf(in, "%s", token))
		{
			if (feof(in))
			{
				cerr << "Unexpected end of file." << endl;
				delete coords;
				return NULL;
			}
			if (stopAtFilePos > 0)
			{
				if (ftell(in) > stopAtFilePos)
					break;
			}
			else
			{
				c = token[0];
				if (!isdigit(c) && c != '-' && c != '+' && c != '.')
				{
					cerr << "Unexpected token: " << token << endl;
					delete coords;
					return NULL;
				}
			}

			sscanf(token, "%f", &coords[i++]);
			if (i >= allocated)
			{
				tempCoords = new float[2 * allocated];
				for (j = 0; j < allocated; j++)
					tempCoords[j] = coords[j];
				delete coords;
				coords = tempCoords;
				allocated *= 2;
			}
		}
		count = i;
	}
	return coords;
}

/*---------------------------------------------------------------------------*/
float *ConversionGrid::createCoordValues(int count, float start, float step)
{
	int i;
	float *coords = NULL;

	if (count <= 0 || step <= 0 || !originGiven)
	{
		cerr << "Error generating regularly spaced ConversionGrid coordinates:" << endl;
		if (count <= 0)
			cerr << "Bad or missing data in COLS_ROWS declaration." << endl;

		if (step == 0.0)
			cerr << "Missing CELLSIZE data." << endl;

		if (!originGiven)
			cerr << "Missing GRID_ORIGIN data." << endl;

		return NULL;
	}

	coords = new float[count];
	if (coords)
	{
		for (i = 0; i < count; i++)
			coords[i] = start + i * step;
	}
	yCoordsCreated = GL_TRUE;

	return coords;
}

/*---------------------------------------------------------------------------*/
int ConversionGrid::readColumnValues()
{
	if (xSpacing == 0.0)
		xcoords = readCoordValues(numCols);
	else
		xcoords = createCoordValues(numCols, xOrigin, xSpacing);

	if (xcoords)
		return 0;
	else
		return 1;
}

/*---------------------------------------------------------------------------*/
int ConversionGrid::readRowValues()
{
	if (ySpacing == 0.0)
		ycoords = readCoordValues(numRows);
	else
		ycoords = createCoordValues(numRows, yOrigin, ySpacing);

	if (ycoords)
		return 0;
	else
		return 1;
}

/*---------------------------------------------------------------------------*/
int ConversionGrid::readGridValues()
{
	int rowsAllocated, colsAllocated, currentRow, currentCol, temp;
	int start, end;
	long fpos, lineLen;
	float *junkData, height, ftemp;
	float **newGrid;
	double percentageFactor;
	char c, token[1024];
	GLboolean firstInRow = GL_TRUE, warningGiven = GL_FALSE;

	if (!rowMajorOrder) //swap cols/rows, then we'll swap back later.
	{
		temp = numRows;
		numRows = numCols;
		numCols = temp;
		ftemp = xSpacing;
		xSpacing = ySpacing;
		ySpacing = ftemp;
	}

	if (numRows <= 0 && numCols <=0) //LITERAL LAYOUT
	{
		fpos = ftell(in);
		for (lineLen = 0; fgetc(in) != '\n'; lineLen++);
		fseek(in, fpos, SEEK_SET);
		junkData = readCoordValues(numCols, fpos + lineLen);
		delete junkData;
		fseek(in, fpos, SEEK_SET);
	}

	if (numRows > 0)
		rowsAllocated = numRows;
	else
		rowsAllocated = ALLOCATION_QUANTUM;

	if (numCols > 0)
		colsAllocated = numCols;
	else
		colsAllocated = ALLOCATION_QUANTUM;

	grid = new float*[rowsAllocated];
	if (grid == NULL)
    {
		cerr << "Can't allocate conversion grid!\n";
		return 1;
    }
	if (!ycoords)
		ycoords = new float[rowsAllocated];
	
	minHeight = 100000000.0;
	maxHeight = -100000000.0;
	currentRow = -1;
	currentCol = numCols + 1;
	if (shape == FULLGRID)
		end = numCols;

    createProgress(); //don't put it up until we know we're going to get here.
	percentageFactor = 47.0/numRows;

	// load the heights (we should always know numCols by this point)
	for (fscanf(in, "%s", token);
		 strcmp(token, sentinelToken);
		 fscanf(in, "%s", token))
	{
		if (feof(in)) //EOF is a valid delimiter
		{
			if (currentRow + 1 < numRows)
				cerr << "Short file (expected " << numRows << " rows, but found " << currentRow + 1 << ")\n";
			break;
		}

		if (currentCol >= end)
		{
			currentRow++;
			updateProgress(currentRow*percentageFactor);
			firstInRow = GL_TRUE;
			if (currentRow >= rowsAllocated) // make more rows
			{
				if (numRows > 0) //done if we knew how many rows to expect
					break;
				newGrid = new float*[2 * rowsAllocated];
				for (temp = 0; temp < rowsAllocated; temp++)
					newGrid[temp] = grid[temp];
				delete grid;
				grid = newGrid;
				if (ySpacing == 0) //need to make more row headings, too
				{
					junkData = new float[2 * rowsAllocated];
					for (temp = 0; temp < rowsAllocated; temp++)
						junkData[temp] = ycoords[temp];
					delete ycoords;
					ycoords = junkData;
				}
				rowsAllocated *= 2;
			}
			grid[currentRow] = new float[numCols];
			currentCol = 0;
			if (spacingInGridData)
			{
				sscanf(token, "%f", &ycoords[currentRow]);
				fscanf(in, "%s", token);
			}
			if (shape == PARTIAL) //find valid strip, place holes on either side
			{
				sscanf(token, "%d", &start);
				fscanf(in,"%d", &end);
				fscanf(in, "%s", token);

				for (temp = 0; temp < start; temp++)
                    grid[currentRow][temp] = gz4d::Nan<float>();
				for (temp = end; temp < numCols; temp++)
					grid[currentRow][temp] = gz4d::Nan<float>();
				currentCol = start;
			}
		}
		if (implicitSpacing)
		{
			if (currentRow == 0)
				sscanf(token, "%f", &xcoords[currentCol]);
			else //skip it, but check it...
			{
				sscanf(token, "%f", &ftemp);
				if (!warningGiven && ftemp != xcoords[currentCol])
				{
					cerr << "Warning!  New x-coordinate of " << ftemp
						 << " given at " << xcoords[currentCol] << ", "
						 << ycoords[currentRow] << endl;
					cerr << "Supressing further such warnings..." << endl;
					warningGiven = GL_TRUE;
				}
			}
			fscanf(in, "%s", token);

			if (firstInRow)
			{
				sscanf(token, "%f", &ycoords[currentRow]);
				firstInRow = GL_FALSE;
			}
			else //skip it
				sscanf(token, "%f", &ftemp);
			fscanf(in, "%s", token);
		}
		c = token[0];
		if (!isdigit(c) && c != '-' && c != '+' && c != '.')
			grid[currentRow][currentCol] = gz4d::Nan<float>();
		else
		{
			sscanf(token, "%f", &height);
			grid[currentRow][currentCol] = height;
			if (height > maxHeight)
				maxHeight = height;
			if (height < minHeight)
				minHeight = height;
		}
		currentCol++;
	}
	numRows = currentRow + 1;

	//transpose, if necessary
	if (!rowMajorOrder)
	{
		cerr << "Transposing...\n";
        updateProgress(48.0, "Transposing...");
		newGrid = new float*[numCols];
		for (currentCol = 0; currentCol < numCols; currentCol++)
		{
			newGrid[currentCol] = new float[numRows];
			for (currentRow = 0; currentRow < numRows; currentRow++)
				newGrid[currentCol][currentRow] = grid[currentRow][currentCol];
		}
		for (currentRow = 0; currentRow < numRows; currentRow++)
			delete grid[currentRow];
		delete grid;
		grid = newGrid;
		temp = numRows;
		numRows = numCols;
		numCols = temp;
		ftemp = xSpacing;
		xSpacing = ySpacing;
		ySpacing = ftemp;
	}

	//Invert the rows, if necessary.
	if (invertedRows)
	{
		end = numRows / 2; //if it is odd, don't need to do middle one anyway...
		temp = numRows - 1;
        updateProgress(49.0, "Inverting...");
		for (currentRow = 0; currentRow < end; currentRow++, temp--)
		{
			junkData = grid[currentRow];
			grid[currentRow] = grid[temp];
			grid[temp] = junkData;

			if (!yCoordsCreated)
			{
				ftemp = ycoords[currentRow];
				ycoords[currentRow] = ycoords[temp];
				ycoords[temp] = ftemp;
			}
		}
	}

	//Check bounds of seemingly valid data
	if (rangeDescription)
	{
		cerr << "Checking valid data bounds...\n";
		minHeight = 100000000.0;
		maxHeight = -100000000.0;
        updateProgress(50.0, "Checking valid data bounds...");
		for (currentRow = 0; currentRow < numRows; currentRow++)
		{
			for (currentCol = 0; currentCol < numCols; currentCol++)
			{
				ftemp = grid[currentRow][currentCol];
                if (gz4d::IsNan(ftemp))
					continue;
				if ((rangeDescription & LessThan) && ftemp >= maxValidValue)
					ftemp = gz4d::Nan<float>();
				if ((rangeDescription & GreaterThan) && ftemp <= minValidValue)
					ftemp = gz4d::Nan<float>();
                if (!gz4d::IsNan(ftemp))
				{
					if (ftemp > maxHeight)
						maxHeight = ftemp;
					if (ftemp < minHeight)
						minHeight = ftemp;
				}
				grid[currentRow][currentCol] = ftemp;
			}
		}
	}

	cerr << "Dimensions: " << numRows << " x " << numCols << "\n";
	cerr << "Range: [" << minHeight << ", " << maxHeight << "]\n";

	// done reading file
	printf("\nDone loading.\n");
	return 0;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setNumColumns(int nCols)
{
	numCols = nCols;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setNumRows(int nRows)
{
	numRows = nRows;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setSentinelToken(const char *token)
{
	sentinelToken = token;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setValidAs(RangeType type, float val1, float val2)
{
	rangeDescription = type;
	if (type == LessThan)
		maxValidValue = val1;
	else if (type == GreaterThan)
		minValidValue = val1;
	else if (type == Between)
	{
		minValidValue = val1;
		maxValidValue = val2;
	}
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setRowMajorOrder(GLboolean rowsFirst)
{
	rowMajorOrder = rowsFirst;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setXspacing(float xInterval)
{
	xSpacing = xInterval;
	if (ySpacing != 0.0)
		spacingInGridData = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setYspacing(float yInterval)
{
	ySpacing = yInterval;
	if (xSpacing != 0.0)
		spacingInGridData = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void ConversionGrid::setSpacingInGridData(GLboolean inData)
{
	spacingInGridData = inData;
}
