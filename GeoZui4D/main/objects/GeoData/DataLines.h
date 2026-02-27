/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// DataLines.h
/*---------------------------------------------------------------------------*/
/** 
 *  This class provides for drawing lines and tubes, with attributes
 *  associated with the lines or component points (temperature, for
 *  instance) mapped to attributes of the same (color, for instance).
 *  This class implements the following responsibilities:
 *      * Read source data from a file to generate lines
 *      * Support viewing of the lines.
 *
 *  Created by mdp 6/4/2001
 *  Derivation from InterpolationSupport added by mdp 2/19/2002
 */
/*---------------------------------------------------------------------------*/
class DataLines;

#ifndef DATALINES_H
#define DATALINES_H

#include "GeoData.h"
#include "engine/InterpolationSupport.h"


class DataLines : public GeoData, public InterpolationSupport
{
    public:
        int readFrom(FILE *file);
        int writeTo(FILE *file);
        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);

        void DrawLowRes(float degradation, Window3D *win);
        int DrawHighRes(int eye, Window3D *win);
        void DrawHandle(Window3D *win, int record, int seg, std::vector<gz4d::Point<double> > const &Path);
        virtual void ResetHighRes();

        virtual void Idle(double time);

        int perform(int argc, const char **argv, Tcl_Interp *interp);

        virtual void updateWrapper(PosableStateWrapper *wrapper, long &changeMask); //we're updating the wrapper and learning of its index change

        int Init(int num = 0);
        void modifyLineAttrs(float sizeVal, Point3f colorRGB, unsigned int whichLine = 0);
        GLboolean modifyPoint(unsigned int index, const Point3d newValue, long &changeMask, unsigned int whichLine = 0);
        GLboolean insertPoint(unsigned int index, unsigned int whichLine = 0) {return insertItem(PointType,index,whichLine);};
        GLboolean getPoint(unsigned int index, Point3d& position, unsigned int whichLine = 0);
        GLboolean deletePoint(unsigned int index, unsigned int whichLine = 0) {return deleteItem(PointType,index,whichLine);};
        GLboolean insertLine(unsigned int index, unsigned int whichLine = 0) {return insertItem(LineType,index,whichLine);};
        GLboolean deleteLine(unsigned int index, unsigned int whichLine = 0)  {return deleteItem(LineType,index,whichLine);};

        virtual int getNumPoints(unsigned int whichLine = 0);
        std::vector<gz4d::Point<double> > &getPointArray(unsigned int whichLine = 0) {return getPoints(whichLine);}

        void applyTransformsToPoints(); //added by mdp 9/2/2005 to allow rotations/scales to propogate to points; &&&currently only applies yaw, scale, and position!

        /*---wrapper attribute support---*/
        enum DataLineWrapperAttributes {WAttr_SIZE, WAttr_TEXT, WAttr_ROLL, WAttr_DATA};
        virtual int getAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);
        virtual int setAttribute(PosableStateWrapper *newWrapper, SmartValue &val, int attributeIndex);
        virtual GLboolean requestAssociativeAttribute(int attributeIndex);
        virtual void resetTimeBoundsOnAttribute(PosableStateWrapper *wrapper, int indexingAttribute, double offset = 0.0, double scale = 1.0);

        DataLines();
        ~DataLines();
        DERIVE_OBJECT(DataLines);

        enum DataLinesParams {LINE = FirstChildKeyword, TUBE, FACETS,
                               FACETS_NTH, ROLL, SHADING, FLAT, SMOOTH, STRETCH_X, STRETCH_Y};

    protected:  
        virtual int processField(int option, char **Field);
        virtual void UpdateTexture(TextureMap *ptr);
        void updateTrailingIndexes(unsigned int line);

        void differentiate(DataRepository &d);

        /*---InterpolationSupport Interface---*/
        virtual std::vector<gz4d::Point<double> > &getPoints(unsigned int whichLine = 0);
        virtual float *getAttributeValues(unsigned int whichLine = 0, int attributeIndex = -1);
        virtual GLboolean modifyPoint(unsigned int index, const Point3f newValue, long &changeMask, unsigned int whichLine = 0);
        virtual GLboolean insertItem(int what, unsigned int index, unsigned int whichLine = 0);
        virtual GLboolean deleteItem(int what, unsigned int index, unsigned int whichLine = 0);

        /*---The actual data---*/
        struct LineRecord
        {
            LineRecord(int points, int colorFormat, int sizeFormat,
                       GLboolean tube, int nFacets, GLboolean hasRoll = GL_FALSE);
            ~LineRecord();

            GLboolean insertPoint(unsigned int index, int facets, int colorFormat);
            GLboolean deletePoint(unsigned int index, int colorFormat);
            unsigned int numPoints;
            std::vector<gz4d::Point<double> >positions;
            Point3f **discs; // the tube cross sections
            vNorms **norms; // normals for components
            float **rgbColor;
            int *indexColor;
            //const unsigned char **bytewiseColor;
            std::vector<gz4d::Pixel<gz4d::RGBA8> > bytewiseColor;
            float *colorVal;
            float *texture_t;
            float *roll;
            float *size;
            unsigned int allocatedColors;
            unsigned int allocatedPoints;
        } **lineRecords;
        //friend struct DataLines::LineRecord;

        int displayLine, displayTube, shadingType;
        int currentPoint, nFacets;
        float stretchX, stretchY;

        enum LineAttributes {ATTRIB_ROLL = NumGeneralAttributes,
                              NumLineAttributes};
        static const char *DataLinesNames[];
        static float *circX, *circY;
        GLboolean hasRoll;

        virtual void CalcAllNormals();
        int CalcNormal(Point3f v1, Point3f v2, Point3f result);
        void DrawShadedRing(Window3D *win, int record, int seg, GLboolean inLowRes = GL_FALSE, int pointGap = 1, int gap = 1);
        void CalculateCircles(float *&circX, float *&circY);
        void updatePoints(int record, int start, int end);


        enum DataTypes {Unknown = 0, Remus};
        bfs::path datafilename;
        int datatype;
        FILE * datafile;

};

#endif 
