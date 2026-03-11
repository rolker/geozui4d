// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ConversionMethod.h
//
//	This class defines the interface for converting information in
//	ConversionGrids to GUTMs using information from Projections.
//
// Created by mdp 12/14/2001
///////////////////////////////////////////////////////////////////////////////

#ifndef CONVERSIONMETHOD_H
#define CONVERSIONMETHOD_H

#include "widgets/Sv_Standards.h"
#include "utilities/Addressable.h"
#include <gz4d/geo/Projection.h>

class Gutm;
class ConversionGrid;

extern DottedAddress *ConversionMethodNeighborhood;

class ConversionMethod : public Addressable
{
	public:
            ConversionMethod(std::string name = "", bool readonly = true);
		virtual ~ConversionMethod();

		//Bookkeeping
		bool isReadOnly() {return readOnly;}

		//Settings
		virtual void setNumColumns(int nCols = -1);
		virtual void setNumRows(int nRows = -1);
		virtual void setSpacing(float interval = 0.0);
		virtual void addArgument(char *arg) {}; //expected to be handled by child classes w/out storing arg.

		//Conversion
		virtual int convert(Gutm *gutm, ConversionGrid *convGrid, gz4d::geo::Projection const &proj);

	protected:
		bool readOnly;

		//setable values
		float spacing;
		int numRows;
		int numCols;
                gz4d::Point<double> mins,maxs;  //in UTM (projected coordinates)

		//values obtained from arguments to conversion
		float **targetGrid, **sourceGrid;
		int sourceRows, sourceCols;
		float sourceSpacingX, sourceSpacingY;
		float *sourceCoordsX, *sourceCoordsY;
		float zOffset;
		gz4d::geo::Projection projection;
		ConversionGrid *gridObject;

		//support functions/variables
		void determineConvertedBounds(ConversionGrid *convGrid, gz4d::geo::Projection const &proj);

		//Bookkeeping, is called by constructor (don't call this function)
		virtual void registerMethod();

		//Child Conversion Method... All values are in place for conversion method to do its thing.
		virtual int conversionMethod() = 0;

	private:
};

#endif
