// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// CoordinateGrid.h
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
class CoordinateGrid;

#ifndef COORDINATEGRID_H
#define COORDINATEGRID_H

#include "engine/Object.h"

class ViewManager;

class CoordinateGrid : public Object
{
	public:
		int readFrom(FILE *file);
		virtual int processField(int option, char **Field);
		void DrawLowRes(float degradation, Window3D *win);
		int DrawHighRes(int eye, Window3D *win);
		virtual void ResetHighRes();

		CoordinateGrid();
		~CoordinateGrid();
		DERIVE_OBJECT(CoordinateGrid);

		enum DimSpec
			{DIM_NONE    = 0x000,
			 //Primary direction
			 DIM_X       = 0x010,
			 DIM_Y       = 0x020,
			 DIM_XY      = 0x030,
			 DIM_Z       = 0x040,
			 DIM_XZ      = 0x050,
			 DIM_YZ      = 0x060,
			 DIM_XYZ     = 0x070,
			 //Secondary direction
			 DIM_THRU_X  = 0x001,
			 DIM_THRU_Y  = 0x002,
			 DIM_THRU_XY = 0x003,
			 DIM_THRU_Z  = 0x004,
			 DIM_THRU_XZ = 0x005,
			 DIM_THRU_YZ = 0x006,
			 DIM_THRU_XYZ= 0x007,
			 //Min/Max/Through
			 DIM_MIN     = 0x100,
			 DIM_MAX     = 0x200,
			 DIM_MIN_MAX = 0x300,
			 DIM_THROUGH = 0x400,
			 DIM_LOCATION= 0x700,
			};

	protected:	
		struct CoordSpec
		{
			int type;
			int robustness;
			int direction;
			int locationOrthogonal;
			float distance;
			int spacingType;
			float *spacing;
		};

		struct CompiledSpec
		{
			int type;
			int robustness;
			float distance;
			int alongDim, dim0, dim1;
			float alongStart, start0, start1;
			float alongStep, step0, step1;
			float alongEnd, end0, end1;
			int labelIndex;
			GLboolean dynamicSpacing, dimThrough0, dimThrough1;
			CompiledSpec *next;
		} *compiledSpecs;

		struct ViewSpec
		{
			float centerPoint[3];
			float centerToScreen;
			float eyeToScreen;
			float pixelSizeAtScreen;
		};

		int dimensionality;
		int reference;
		Point3f size, fgColor, bgColor, *planePoints;
		float maxSize;
		int numPlanes, *planeSpecs;
		CompiledSpec *currentSpec, *lastSpec;
		float backgroundOffset;
		float factorLevel;

		enum CoordinateGridParams {DIMENSIONS = FirstObjectKeyword, REFERENCE, BACKGROUND,
								   BGCOLOR, LINECOLOR, BACKGROUND_OFFSET,
								   LINES, TICKS, LABELS, TICKS_LABELS,
								   GRID, LOCAL, GLOBAL, MIN, THROUGH, MAX,
								   MIN_MAX, SPACING_EVEN, SPACING_DYNAMIC,
								   SPACING_XYZ, FRAGILE, TRANSIENT
								   };
		static const char *CoordinateGridNames[];
		static const char *LabelFormats[];

		int convertLocation(char *locationString);
		void GeneratePlanes();
		void DrawPlanes();
		void DrawSpec(Window3D *win);
		void DrawDynamicOrNot(Window3D *win, ViewSpec &viewSpec);
		void generateLoop(int dimension, int location, float &start,
						  float &step, float &end, CoordSpec &currentSpec);
		void compileSpec(CoordSpec &spec);
		void generateViewSpec(ViewManager *view, ViewSpec &spec);
		void generateLoops(CompiledSpec &spec, float step, ViewSpec &viewSpec);
		void generateLimitedLoop(int dimension, float limit, float &start,
								 float step, float &end, ViewSpec &viewSpec,
								 GLboolean along = GL_FALSE);
};

#endif 
