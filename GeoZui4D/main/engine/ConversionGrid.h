/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ConversionGrid.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CONVERSIONGRID_H
#define CONVERSIONGRID_H

#include "engine/Persistence.h"

/**
 *	This class defines a grid-reader for converting to GUTMs.
 *  \author Matt Plumlee (mdp)
 *  \date 12/12/2001
 */
class ConversionGrid : public Persistence
{
		friend class ConversionMethod;

	public:
		//File-reading
		virtual int Load();
		virtual int readHeader();
		virtual int readData();
		virtual int readColumnValues();
		virtual int readRowValues();
		virtual int readGridValues();

		//Settings
		virtual void setNumColumns(int nCols = -1);
		virtual void setNumRows(int nRows = -1);
		virtual void setSentinelToken(const char *token = "END");
		enum RangeType {NotRanged = 0, LessThan = 1, GreaterThan = 2, Between = 3};
		virtual void setValidAs(RangeType type, float val1 = 0.0, float val2 = 0.0);
		virtual void setRowMajorOrder(GLboolean rowsFirst = GL_TRUE);
		virtual void setXspacing(float xInterval = 0.0);
		virtual void setYspacing(float yInterval = 0.0);
		virtual void setSpacingInGridData(GLboolean inData = GL_TRUE);

		ConversionGrid();
		virtual ~ConversionGrid();

		//static float NaN;

	protected:
		float **grid;
		float *xcoords, *ycoords;
		float xSpacing, ySpacing, xOrigin, yOrigin;
		float minValidValue, maxValidValue, minHeight, maxHeight;

		GLboolean rowMajorOrder;
		GLboolean spacingInGridData;
		GLboolean implicitSpacing;
		GLboolean invertedRows;
		GLboolean yCoordsCreated;
		RangeType rangeDescription;

		int numRows;
		int numCols;
		GLboolean originGiven;

		int shape;

		const char *sentinelToken;

		virtual float *readCoordValues(int &count, long stopAtFilePos = -1);
		float *createCoordValues(int count, float start, float step);

	private:
};
#endif
