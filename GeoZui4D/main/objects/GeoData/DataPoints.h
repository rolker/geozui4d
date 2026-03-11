// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// DataPoints.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class provides for drawing points and glyphs, with attributes
 *	associated with the points (temperature, for instance) mapped
 *	to attributes of the points (color, for instance).
 *	This class implements the following responsibilities:
 *		* Read source data from a file to generate points
 *		* Support viewing of the points.
 *
 *	Created by mdp 5/25/2001
 */
/*---------------------------------------------------------------------------*/
class DataPoints;

#ifndef DATAPOINTS_H
#define DATAPOINTS_H

#include "GeoData.h"
#include "engine/InterpolationSupport.h"

class Glyph;

class DataPoints : public GeoData, public InterpolationSupport
{
	public:
		int readFrom(FILE *file);
		int writeTo(FILE *file);
		void DrawHandle(Window3D *win, int index);
		void DrawLowRes(float degradation, Window3D *win);
		int DrawHighRes(int eye, Window3D *win);
		void DrawSpecialCasePoints(int skip, GLboolean lowResMode);
		virtual void ResetHighRes();

		Glyph *convertGlyphCode(int code);
		int convertGlyphToCode(Glyph *g);

		virtual int perform(int argc, const char **argv, Tcl_Interp *interp);

		void init(int n = -1);

		virtual void updateWrapper(PosableStateWrapper *wrapper, long &changeMask); //we're updating the wrapper and learning of its index change

		/*---InterpolationSupport Interface---*/
                virtual std::vector<gz4d::Point<double> > &getPoints(unsigned int whichLine = 0);
		virtual float *getAttributeValues(unsigned int whichLine = 0, int attributeIndex = -1);
		virtual int getNumPoints(unsigned int whichLine = 0);
                virtual GLboolean modifyPoint(unsigned int index, const Point3f newValue, long &changeMask, unsigned int whichLine = 0);
                virtual GLboolean insertItem(int what, unsigned int index, unsigned int whichLine = 0);
                virtual GLboolean deleteItem(int what, unsigned int index, unsigned int whichLine = 0);
		void SetText(int i, char * t);


		/*---wrapper attribute support---*/
		enum DataPointWrapperAttributes {WAttr_SIZE, WAttr_TEXT, WAttr_GLYPH, WAttr_DATA};
		virtual int getAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);
		virtual int setAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);
		virtual GLboolean requestAssociativeAttribute(int attributeIndex);
		virtual void resetTimeBoundsOnAttribute(PosableStateWrapper *wrapper, int indexingAttribute, double offset = 0.0, double scale = 1.0);

		DataPoints();
		~DataPoints();
		DERIVE_OBJECT(DataPoints);

		void setDropline(){dropLine = 1;};
		void setPointSize(double s);

		enum DataPointsParams {GLYPH = FirstChildKeyword, DROP_LINE,SHOW_POINT,
							   SIMPLIFY_GLYPH,TRUNCATE,
							   POINT, CONE, CUBE, CYLINDER, DIAMOND,
							   INVERTED_CONE, SPHERE, UNIT_CUBE,
							   UNIT_CYLINDER, UNIT_SPHERE};

	protected:
		enum PointAttributes {ATTRIB_GLYPH = NumGeneralAttributes,
							  NumPointAttributes};

		virtual int processField(int option, char **Field);
		virtual void UpdateTexture(TextureMap *ptr);
		void updateTrailingIndexes();

		std::vector<gz4d::Point<double> > positions;
		float **rgbColor;
		//const unsigned char **bytewiseColor;
                std::vector<gz4d::Pixel<gz4d::RGBA8> > bytewiseColor;
		float *colorVal;
		float *texture_t;
		int glyphType;
		Glyph **glyph;
		int dropLine;
		int showPoint;

                int oldIncrement;

		static const char *DataPointsNames[];
		static Glyph *baseGlyph[];
                
                GLuint displayList;
                bool rebuildDisplayList;
                bool useFog;
};

#endif 
