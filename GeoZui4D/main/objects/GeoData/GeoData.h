// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// GeoData.h
/*---------------------------------------------------------------------------*/
/** 
 *  This class provides some base functionality for the Data*
 *  Objects (DataPoints, DataLines, DataTriangles).  Each has
 *  attributes associated with points or records (temperature,
 *  for instance) mapped to attributes of the points or
 *  records (color, for instance).
 *  This class implements the following responsibilities:
 *      * Read & parse some source data from a file
 *      * Create some structures
 *
 *  Created by mdp 5/24/2001
 */
/*---------------------------------------------------------------------------*/
class GeoData;

#ifndef GEODATA_H
#define GEODATA_H

#include "engine/Object.h"
#include "utilities/SmartArray.h"

class GeoData : public Object
{
    public:
        virtual int processField(int option, char **Field);
        //void DrawLowRes(float degradation, Window3D *win);
        //int DrawHighRes(int eye, Window3D *win);
        virtual void ResetHighRes();

        virtual void ShowInfo(GLuint *names, int numNames);
        virtual void PoseChanged(PosableStateBase *whichPose, long whatChanged);

                virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);

        virtual int writeTo(FILE *file);
        int convertColorCode(char code);
        char convertColorCode(int code);
        int convertRGBToColorCode(float *color);

        Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);


        void insertRecord(unsigned int index);
        void deleteRecord(unsigned int index);

        void showTrailing(float seconds = 300.0);
        void resetTimeAttribute(int timeAttribute = -1);
        void setTimeAttributeOffset(double seconds);
        void setTimeAttributeScale(double scale);

        virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
        virtual int list(Tcl_Interp *interp);

        /*---Attribute Accessors and Mutators---*/
        //virtual int getNumAttributes();
        virtual int getAttributeIndex(std::string attributeName);
        virtual std::string getAttributeName(int attributeIndex);
        //virtual int getAttribute(SmartValue &val, int attributeIndex);
        //virtual int setAttribute(SmartValue &val, int attributeIndex);

        GeoData(int numAttributes, const char *type,
                const char **keywords, int major = 1, int minor = 0);
        ~GeoData();

        //enumerations for internal data handling
        enum WhichFormat {FORMAT_NONE      = 0x00,
                          FORMAT_BY_COLUMN = 0x01,
                          FORMAT_BY_RECORD = 0x02,
                          FORMAT_BY_RECORD_OR_COLUMN = 0x03,
                          FORMAT_CONSTANT  = 0x04,
                          FORMAT_DIMENSION = 0x08,
                          FORMAT_MASK      = 0x0F,
                         }; 
        enum ColorType {COLOR_ATTRIBUTE    = 0x10,
                        COLOR_RGB          = 0x20,
                        COLOR_CODE         = 0x40,
                        COLOR_TEXTURE      = 0x80,
                        COLOR_AS_BYTES     = 0x100,
                        COLOR_ALLOCATED    = 0x200,
                        COLOR_TEXTURE_T    = 0x400,
                        COLOR_AS_HEIGHT    = 0x1000
                       };   
        enum GeneralAttributes {ATTRIB_X, ATTRIB_Y, ATTRIB_Z,
                                ATTRIB_R, ATTRIB_G, ATTRIB_B,
                                ATTRIB_COLOR, ATTRIB_SIZE,
                                ATTRIB_X_SIZE, ATTRIB_Y_SIZE, ATTRIB_Z_SIZE,
                                ATTRIB_TEXT, ATTRIB_TEXTURE_S,
                                ATTRIB_TEXTURE_T, ATTRIB_DATA,
                                NumGeneralAttributes};

        enum GeoDataParams {N_RECORDS = FirstObjectKeyword, N_COLUMNS, XYZ,
                            XYZ_COLUMNS, COLOR, SIZE, TEXT,
                            DATA, AS_NUMBER, AS_STRING, AS_EXPONENTIAL, AS_OFFSET,
                            DIFFERENTIATE, DOUBLE_DIFFERENTIATE,
                            NEW, ATTRIBUTE, AS_HEIGHT, RGB_COLUMNS, CODE,
                            TEXTURE, TEXTURE_COLUMNS,
                            RGB, COLUMN, COLUMNS, PER_RECORD, CONSTANT,
                            NONE, FRAGILE, TRANSIENT,
                            NTH, FirstChildKeyword
                            };

    protected:
        union Color
        {
            Point3f rgb;
            //float val;
            int index;
        } constantColor;

        struct DataSpec {
            char *name;
            int column;
            WhichFormat format;
            int type;
            DataSpec *next;
        } *dataSpec, *lastDataSpec;
        unsigned int numDataFields;

        enum {TAKE_LG = 0x1000, TREAT_AS_OFFSET = 0x2000, TAKE_DIFFERENTIAL = 0x4000, TAKE_DOUBLE_DIFFERENTIAL = 0x8000};
        struct DataRepository {
            SmartArray *data;
            WhichFormat format;
            int column;
            SmartMemory::IndexType type;
        } *dataRepository;
        char **dataNames;

        int assignColumn(int &i, int attrib, int field, int modifier = -1);
        void convertDataFields();
        void convertData(char **fields, struct DataRepository *drep, int i1, int i2, int column);
        void setIndexedAttributeOrdering(unsigned int &numAttributes, const char **&attributeNames, int *names);

        int numAttribs;
        unsigned int numRecords;
        unsigned int allocatedRecords;
        int numColumns;
        int *attribToColumn;
        float *size, sizeFactor, *dimensionalSize[3];
        char **text;

        int colorFlags, sizeFlags, textFlags;

        unsigned int currentRecord;
        int selectedRecord;
        int selectedPoint;
        int numDimensions;

        float trailingSeconds;
        unsigned int trailingStartIndex;
        unsigned int trailingEndIndex;

        int timeAttributeIndex;
        double timeAttributeOffset, timeAttributeScale;
};

#define GEODATA_NAMES \
    OBJECT_NAMES, \
    "N_RECORDS", "N_COLUMNS", "XYZ", "XYZ_COLUMNS", \
    "COLOR", "SIZE", "TEXT", \
    "DATA", "AS_NUMBER", "AS_STRING", "AS_EXPONENTIAL",\
    "AS_OFFSET", "DIFFERENTIATE", "DOUBLE_DIFFERENTIATE", \
    "NEW", "ATTRIBUTE", \
    "AS_HEIGHT", "RGB_COLUMNS", "CODE", \
    "TEXTURE", "TEXTURE_COLUMNS", \
    "RGB", "COLUMN", "COLUMNS", "PER_RECORD", "CONSTANT", \
    "NONE", "FRAGILE", "TRANSIENT", \
    "NTH"
#define DEFAULT_COLOR 8

#endif 
