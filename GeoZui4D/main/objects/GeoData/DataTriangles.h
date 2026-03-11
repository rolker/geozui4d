// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// DataTriangles.h
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
class DataTriangles;

#ifndef DATATRIANGLES_H
#define DATATRIANGLES_H

#include "GeoData.h"

class DataTriangles : public GeoData
{
	public:
		int readFrom(FILE *file);
		void DrawLowRes(float degradation, Window3D *win);
		int DrawHighRes(int eye, Window3D *win);
		virtual void ResetHighRes();
		virtual void useNewHeightExaggeration(GLfloat heightEx);
		int perform(int argc, const char **argv, Tcl_Interp *interp);

		DataTriangles();
		~DataTriangles();
		DERIVE_OBJECT(DataTriangles);

		enum DataTrianglesParams {FILL = FirstChildKeyword, NORMALS, SCALE_Z,
								  RIGHT_HANDED, LEFT_HANDED, UP, DOWN, FAN};

	protected:	
		enum TriangleAttributes {ATTRIB_X_NORMALS = NumGeneralAttributes,
								 ATTRIB_Y_NORMALS, ATTRIB_Z_NORMALS,
							     NumTriangleAttributes};

		virtual int processField(int option, char **Field);
		virtual void UpdateTexture(TextureMap *ptr);

		struct TriangleRecord
		{
			TriangleRecord(int points, int colorFormat);
			~TriangleRecord();

			int numPoints;
			std::vector<gz4d::Point<double> > positions;
			struct TextureCoordinate
			{
				float s;
				float t;
			} *texturePosition;
			vNorms *norms; // normals for each triangle point, depending on normalsRule
			float **rgbColor;
			//const unsigned char **bytewiseColor;
                        std::vector<gz4d::Pixel<gz4d::RGBA8> > bytewiseColor;
			float *colorVal;
                        unsigned int allocatedColors;
			int displayFill, normalsRule;
		} **triangleRecords;

		int displayFill, normalsRule, triangleFan;
		int currentPoint;
		float normal_z_factor;

		static const char *DataTrianglesNames[];
		static float *circX, *circY;

		virtual void CalcAllNormals();
		void updatePoints(int record, int start, int end);
		void DrawStrip(int record, GLboolean inLowRes = GL_FALSE, int pointGap = 1);
};

#endif 
