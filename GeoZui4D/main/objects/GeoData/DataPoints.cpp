/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// DataPoints.cpp
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

#include<gz4d/glo.h>
#include "GZincludes.h"
#include "objects/GeoData/DataPoints.h"
#include "objects/GeoData/SphereGlyph.h"
//#include "MainWindow.h"
//#include "engine/Scene3D.h"
//#include <iostream>
#include "objects/GeoData/Cone.h"
#include "Cube.h"
#include "objects/GeoData/Cylinder.h"
#include "DoublePyramid.h"
#include "InvertedCone.h"
#include "widgets/Sv_Standards.h"
#include "interactors/PointHandle.h"
#include "engine/PosableStateWrapper.h"
#include "utilities/Utilities.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/ViewManager.h"
#include "engine/TimeControl.h"
#include<gz4d/glo/Vertex.h>
#include <gz4d/glo/Color.h>

using namespace std;

extern PointHandle *pointHandle;

extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(DataPoints, "gdp");

#define POINTS_AT_A_TIME 10

const char *DataPoints::DataPointsNames[] =
{GEODATA_NAMES, "GLYPH", "DROP_LINE", "SHOW_POINT","SIMPLIFY_GLYPH","TRUNCATE",
                        /* Only glyphs from here... */
                        "POINT", "CON", "CUB", "CYL", "DIA", "INV", "SPH",
                        "UCU", "UCY", "USP",
                        /* ... to here */
                        ""};

static int DataPointAttributeOrdering[] = {DataPoints::SIZE, DataPoints::TEXT, DataPoints::GLYPH, DataPoints::DATA, -1};

static Cone cone;
static Cube cube;
static Cube unitCube(1.0f);
static Cylinder cylinder;
static Cylinder unitCylinder(1.0f);
static DoublePyramid diamond;
static InvertedCone invertedCone;
static SphereGlyph sphere;
static SphereGlyph unitSphere(1.0f);
Glyph *DataPoints::baseGlyph[] = {&cone, &cube, &cylinder, &diamond, &invertedCone, &sphere, &unitCube, &unitCylinder, &unitSphere};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
DataPoints::DataPoints()
: GeoData(NumPointAttributes, "DATA_POINTS", DataPointsNames)
{
    rgbColor = NULL;
    //bytewiseColor = NULL;
    colorVal = NULL;
    texture_t = NULL;
    glyphType = POINT;
    glyph = NULL;
    dropLine = FRAGILE;
    showPoint = OFF;
    numDimensions = 1;
    displayList = 0;
    rebuildDisplayList = true;
    oldIncrement = 1;
    useFog = false;
//    if(glewInit() == GLEW_OK)
//    {}//useFog = true;

}

    /*--------------------------------------------------------------------------*/
    DataPoints::~DataPoints()
{

    if ((colorFlags & COLOR_RGB) && rgbColor)
    {
        for (unsigned int i = 0; i < numRecords; i++)
            delete rgbColor[i];
        delete [] rgbColor;
    }
//     if (bytewiseColor)
//         delete [] bytewiseColor;
    if (colorVal)
        delete colorVal;
    if (texture_t)
        delete texture_t;
    delete [] wrapableAttributeNames;
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DataPoints::DrawLowRes(float degradation, Window3D *win)
{
    int i, increment = 1, start = 0, end = numRecords;
    float scaleBy = 1.0;
    std::shared_ptr<TextureMap> currentTexture;
    GLboolean trailing = GL_FALSE;

    if(!displayList)
        displayList = glGenLists(numRecords);

    if (degradationFlag == NTH || degradationFlag == ON) //NTH equal to ON
        //increment = (int)(2.1 / (2.1 - (2 * degradation)));
        //increment = oldIncrement+ (int)(3*degradation)-1;
        increment = (int)(numRecords*degradation)/250;

    if (degradationFlag == TRUNCATE && degradation > 0.1)
        end = (int)(end*(1.1-degradation));

    if(increment < 1)
        increment = 1;


    oldIncrement = increment;

        //Are we drawing a trailing section?
    if (trailingSeconds > 0.0 && timeAttributeIndex >= 0)
        trailing = GL_TRUE;

    glPushAttrib(GL_POINT_BIT|GL_LIGHTING_BIT);

    glShadeModel(GL_FLAT);
                
    if (colorFlags & FORMAT_CONSTANT)
    {
        if (colorFlags & COLOR_RGB)
            glColor3fv(constantColor.rgb);
        else
            glColor3fv(palette[constantColor.index]);
    }
    else if (colorFlags & COLOR_TEXTURE)
    {
        if (!customtextureptr)
            currentTexture = *defaulttextureptr;
        else
            currentTexture = customtextureptr;
        if(colorFlags & FORMAT_BY_COLUMN)
            EnableTexture(TEXTURE_COORD_NONE);
        else
            EnableTexture();
        glColor3f(1.0, 1.0, 1.0);
    }

    if (glyphType == POINT && (sizeFlags & FORMAT_CONSTANT) && (!renderingForPicking || (textFlags & FORMAT_MASK) == FORMAT_NONE)  && (!(colorFlags & COLOR_TEXTURE) || !(colorFlags & FORMAT_BY_COLUMN)))
        DrawSpecialCasePoints(increment, GL_TRUE);
    else
    {
                //constant size handling
        if (sizeFlags & FORMAT_CONSTANT)
        {
            if (increment > 1)
                scaleBy = sizeFactor * 2;
            else
                scaleBy = sizeFactor;
        }
        if (trailing)
        {
            updateTrailingIndexes();
            start = trailingStartIndex;
            end = trailingEndIndex;// + 1;
        }

        Point3d localEye;
        DIFF_POINT(win->getViewInfo()->getEyePoint(),GetPosition(),localEye);
        //double s = win->getPoseInfo()->getScale();
        double s = 1.0/win->getViewInfo()->getCenterToEyepoint();

        if(useFog)
            glEnable(GL_FOG);

            /*        if(rebuildDisplayList)
        {
        for (i = 0; i < numRecords; i++)*/
        for (i = start; i < end; i += increment)
        {
//                glNewList(displayList+i,GL_COMPILE);
            glPushMatrix();
            glTranslatef(positions[i][X], positions[i][Y], positions[i][Z]);
    
                            //size
            if (sizeFlags & FORMAT_BY_COLUMN)
                scaleBy = size[i] * sizeFactor;
            if (glyphType == POINT || showPoint == ON || showPoint == TRANSIENT)
                glPointSize(scaleBy);
            else if (scaleBy != 1.0 || sizeFlags & FORMAT_DIMENSION)
            {
                if (sizeFlags & FORMAT_DIMENSION)
                {
                    if (size)
                        scaleBy = size[i] * sizeFactor;
                    else
                        scaleBy = sizeFactor;
                    glScalef((dimensionalSize[X]? dimensionalSize[X][i] * scaleBy : scaleBy),
                              (dimensionalSize[Y]? dimensionalSize[Y][i] * scaleBy : scaleBy),
                               (dimensionalSize[Z]? dimensionalSize[Z][i] * scaleBy : scaleBy));
                }
                else
                    glScalef(scaleBy, scaleBy, scaleBy);
            }

            //color
            if (!(colorFlags & FORMAT_CONSTANT))
            {
                if (colorFlags & COLOR_AS_BYTES)
                    //glColor4ubv(bytewiseColor[i]);
                    gz4d::glo::Color(bytewiseColor[i]);
                else if (colorFlags & COLOR_TEXTURE)
                {
                    if (colorFlags & FORMAT_BY_COLUMN)
                    {
                        if (colorFlags & COLOR_TEXTURE_T)
                            glTexCoord2f(colorVal[i], texture_t[i]);
                        else
                            currentTexture->SetHeight(colorVal[i]);
                    }
                    else
                        currentTexture->beginTextureOffset(positions[i][X], positions[i][Y], positions[i][Z], scaleBy);
                }
                else
                    glColor3fv(rgbColor[i]);
            }

                //draw the point.
            if(useFog)
            {
                double d = sqrt(pow(positions[i][X]-localEye[X],2)+pow(positions[i][Y]-localEye[Y],2)+pow(positions[i][Y]-localEye[Y],2))*s;
                glFogCoordd(d);
            }

            glLoadName(i);
            if (glyphType == POINT	|| (showPoint == ON || showPoint == TRANSIENT))
            {
                glBegin(GL_POINTS);
                glVertex3f(0.0, 0.0, 0.0);
                glEnd();
            }
            if(glyphType != POINT)
            {
                if (glyphType == COLUMN)
                    glyph[i]->draw(degradation);
                else
                    baseGlyph[glyphType]->draw(degradation);
            }
            if (dropLine == ON || dropLine == TRANSIENT)
            {
                glShadeModel(GL_FLAT);
                glBegin(GL_LINES);
                glNormal3f(0.0, 0.0, 1.0);
                glVertex3f(0.0, 0.0, 0.0);
                glVertex3f(0.0, 0.0, -50.0);
                glEnd();
            }
            if ((colorFlags & COLOR_TEXTURE) && !(colorFlags & FORMAT_BY_COLUMN))
                currentTexture->endTextureOffset();
            glPopMatrix();
/*                glEndList();

        }
            rebuildDisplayList = false;*/
        }


        for (i = start; i < end; i += increment) // i < end - 1;
        {
            glCallList(displayList+i);
        }
        glDisable(GL_FOG);
    }

    if (colorFlags & COLOR_TEXTURE)
        DisableTexture();

        //draw point handles, if necessary.  Done out here so it doesn't get textured.
    if (selectedFlag && selectedRecord >= 0)
    {
        for(i = start; i < end; i += increment)
        {
            DrawHandle(win, i);
            if (oldIncrement > 1)
            {
                if (i >= (selectedRecord - oldIncrement) &&
                    i <= (selectedRecord + oldIncrement))
                    increment = 1;
                else
                    increment = oldIncrement;
            }
        }
    }

    doneFlag = currentRecord = 0;
    glColor3f(1.0,1.0,1.0);

}

/*--------------------------------------------------------------------------*/
int DataPoints::DrawHighRes(int eye, Window3D *win)
{
    unsigned int i, lastRecord, oldCurrentRecord, end = numRecords;
    float scaleBy = 1.0;
    std::shared_ptr<TextureMap> currentTexture;
    GLboolean trailing = GL_FALSE;

    glShadeModel(GL_FLAT);
    if (!doneFlag)
    {
                //Are we drawing a trailing section?
        if (trailingSeconds > 0.0 && timeAttributeIndex >= 0)
            trailing = GL_TRUE;

        if (trailing)
            end = trailingEndIndex;
        else
            end = numRecords;
        if (currentRecord < end)
        {
            if (colorFlags & FORMAT_CONSTANT)
            {
                if (colorFlags & COLOR_RGB)
                    glColor3fv(constantColor.rgb);
                else
                    glColor3fv(palette[constantColor.index]);
            }
            else if (colorFlags & COLOR_TEXTURE)
            {
                if (!customtextureptr)
                    currentTexture = *defaulttextureptr;
                else
                    currentTexture = customtextureptr;
                if(colorFlags & FORMAT_BY_COLUMN)
                    EnableTexture(TEXTURE_COORD_NONE);
                else
                    EnableTexture();
                glColor3f(1.0, 1.0, 1.0);
            }
            oldCurrentRecord = currentRecord;
            lastRecord = numRecords;
            if (glyphType == POINT && (sizeFlags & FORMAT_CONSTANT) && (!(colorFlags & COLOR_TEXTURE) || !(colorFlags & FORMAT_BY_COLUMN)))
            {
                DrawSpecialCasePoints(1, GL_FALSE);
                if (eye == dominantEye || eye == MONO_EYE)
                    currentRecord = numRecords;
            }
            else
            {
                Point3d localEye;
                DIFF_POINT(win->getViewInfo()->getEyePoint(),GetPosition(),localEye);
                //double s = win->getPoseInfo()->getScale();
                double s = 1.0/win->getViewInfo()->getCenterToEyepoint();

                lastRecord = currentRecord + POINTS_AT_A_TIME;
                if (lastRecord > end)
                    lastRecord = end;
                if (sizeFlags & FORMAT_CONSTANT)
                    scaleBy = sizeFactor;
                for(i = currentRecord; i < lastRecord; i++)
                {
                    glPushMatrix();
                    glTranslatef(positions[i][X], positions[i][Y], positions[i][Z]);

                                        //size
                    if (sizeFlags & FORMAT_BY_COLUMN)
                        scaleBy = size[i] * sizeFactor;
                    if (glyphType == POINT)
                        glPointSize(scaleBy);
                    else if (scaleBy != 1.0 || sizeFlags & FORMAT_DIMENSION)
                    {
                        if (sizeFlags & FORMAT_DIMENSION)
                        {
                            if (size)
                                scaleBy = size[i] * sizeFactor;
                            else
                                scaleBy = sizeFactor;
                            glScalef((dimensionalSize[X]? dimensionalSize[X][i] * scaleBy : scaleBy),
                                      (dimensionalSize[Y]? dimensionalSize[Y][i] * scaleBy : scaleBy),
                                       (dimensionalSize[Z]? dimensionalSize[Z][i] * scaleBy : scaleBy));
                        }
                        else
                            glScalef(scaleBy, scaleBy, scaleBy);
                    }

                                        //color
                    if (!(colorFlags & FORMAT_CONSTANT))
                    {
                        if (colorFlags & COLOR_AS_BYTES)
                            //glColor4ubv(bytewiseColor[i]);
                            gz4d::glo::Color(bytewiseColor[i]);
                        else if (colorFlags & COLOR_TEXTURE)
                        {
                            glColor3f(1.0, 1.0, 1.0);
                            if (colorFlags & FORMAT_BY_COLUMN)
                            {
                                if (colorFlags & COLOR_TEXTURE_T)
                                    glTexCoord2f(colorVal[i], texture_t[i]);
                                else
                                    currentTexture->SetHeight(colorVal[i]);
                            }
                            else
                                currentTexture->beginTextureOffset(positions[i][X], positions[i][Y], positions[i][Z], scaleBy);
                        }
                        else
                            glColor3fv(rgbColor[i]);
                    }


                    if(useFog)
                    {
                        glEnable(GL_FOG);
                        double d = sqrt(pow(positions[i][X]-localEye[X],2)+pow(positions[i][Y]-localEye[Y],2)+pow(positions[i][Y]-localEye[Y],2))*s;
                        glFogCoordd(d);
                    }

                                        //draw the point.
                    if (glyphType == POINT || (showPoint == ON || showPoint == FRAGILE))
                    {
                        glBegin(GL_POINTS);
                        glVertex3f(0.0, 0.0, 0.0);
                        glEnd();
                    }
                    if(glyphType != POINT)
                    {
                        if (glyphType == COLUMN)
                            glyph[i]->draw(0.0);
                        else
                            baseGlyph[glyphType]->draw(0.0);
                    }
                    if (dropLine == ON || dropLine == FRAGILE)
                    {
                        glShadeModel(GL_FLAT);
                        glBegin(GL_LINES);
                        glNormal3f(0.0, 0.0, 1.0);
                        glVertex3f(0.0, 0.0, 0.0);
                        glVertex3f(0.0, 0.0, -50.0);
                        glEnd();
                    }
                    glDisable(GL_FOG);
                    glPopMatrix();
                    if ((colorFlags & COLOR_TEXTURE) && !(colorFlags & FORMAT_BY_COLUMN))
                        currentTexture->endTextureOffset();
                }
                if (eye == dominantEye || eye == MONO_EYE)
                    currentRecord += POINTS_AT_A_TIME;
            }
            if (colorFlags & COLOR_TEXTURE)
                DisableTexture();

                        //draw point handles, if necessary.  Done out here so it doesn't get textured.
            if (selectedFlag && selectedRecord >= 0)
            {
                for(i = oldCurrentRecord; i < lastRecord; i++)
                    DrawHandle(win, i);
            }

        }
        else
            doneFlag = 1;
    }
    else
        doneFlag = 1;
        
    glPopAttrib();
    
    return doneFlag;
}

    /*--------------------------------------------------------------------------*/
    void DataPoints::DrawSpecialCasePoints(int skip, GLboolean lowResMode)
{

    if (skip > 1)
        glPointSize(sizeFactor * 2);
    else
        glPointSize(sizeFactor);
    glBegin(GL_POINTS);
    glNormal3f(0.0, 0.0, 1.0);
    for(unsigned int i = 0; i < numRecords; i += skip)
    {
                //color
        if (!(colorFlags & FORMAT_CONSTANT))
        {
            if (colorFlags & COLOR_AS_BYTES)
                gz4d::glo::Color(bytewiseColor[i]);
                //glColor4ubv(bytewiseColor[i]);
            else if (colorFlags & COLOR_TEXTURE)
                glColor3f(1.0, 1.0, 1.0);
            else
                glColor3fv(rgbColor[i]);
        }
        gz4d::glo::Vertex(positions[i]);
    }
    glEnd();

    if (dropLine == ON || (lowResMode && dropLine == TRANSIENT)
        || (!lowResMode && dropLine == FRAGILE))
    {
        glBegin(GL_LINES);
        for(unsigned int i = 0; i < numRecords; i += skip)
        {
                        //color
            if (!(colorFlags & FORMAT_CONSTANT) && !(colorFlags & COLOR_TEXTURE))
            {
                if (colorFlags & COLOR_AS_BYTES)
                    //glColor4ubv(bytewiseColor[i]);
                    gz4d::glo::Color(bytewiseColor[i]);
                else
                    glColor3fv(rgbColor[i]);
            }
            gz4d::glo::Vertex(positions[i]);
            glVertex3d(positions[i][X], positions[i][Y], positions[i][Z] - 50.0);
        }
        glEnd();
    }
}

    /*--------------------------------------------------------------------------*/
    int DataPoints::processField(int option, char **fields)
{
    int i = 1;
    char glyphStr[4];

    switch(option)
    {
        case GLYPH:
            glyphType = Lookup(fields[i], GL_FALSE);
            if (glyphType < 0)
            {
                strncpy(glyphStr, fields[i], 4);
                glyphStr[3] = '\0';
                glyphType = Lookup(glyphStr, GL_FALSE);
            }
            i++;
            if (glyphType == COLUMN)
            {
                glyph = new Glyph *[numRecords];
                attribToColumn[ATTRIB_GLYPH] = ConvertInt(i) - 1;
            }
            else if (glyphType < 0)
            {
                cerr << "Invalid glyph identifier " << fields[i - 1]
                        << ".  Expecting POINT, CON, CUB, CYL, DIA, INV, or SPH."
                        << endl << "Defaulting to POINT" << endl;
                glyphType = POINT;
            }
            else if (glyphType != POINT)
                glyphType -= CONE; //this only works up to POINT (25)!
            break;
        case DROP_LINE:
            dropLine = Lookup(fields[i++]);
            break;
        case SHOW_POINT:
            showPoint = Lookup(fields[i++]);
            break;
        default:
            return GeoData::processField(option, fields);
            break;
    }
    return 0;
}

    /*--------------------------------------------------------------------------*/
// documented in Object
    int DataPoints::readFrom(FILE *infile)
{
    char **fields;

    if (GeoData::readFrom(infile))
        return 1;
    convertDataFields();

        //allocate necessary memory.
    positions.resize(numRecords);
    if (colorFlags & COLOR_ATTRIBUTE)
    {
        //bytewiseColor = new const unsigned char*[numRecords];
        bytewiseColor.resize(numRecords);
        colorVal = new float[numRecords];
    }
    else if ((colorFlags & COLOR_TEXTURE) && (colorFlags & FORMAT_BY_COLUMN))
    {
        colorVal = new float[numRecords];
        if (colorFlags & COLOR_TEXTURE_T)
            texture_t = new float[numRecords];
    }
    else if ((colorFlags & COLOR_RGB) || (colorFlags & COLOR_CODE))
    {
        rgbColor = new float*[numRecords];
        if (colorFlags & COLOR_RGB)
        {
            colorFlags |= COLOR_ALLOCATED;
            for (unsigned int i = 0; i < numRecords; i++)
                rgbColor[i] = new Point3f;
        }
    }

        // read points one at a time
    for (unsigned int i = 0; i < numRecords; i++)
    {
        fields = NextLine(GL_FALSE);
        if (fields == NULL)
        {
            numRecords = i;
            cerr << "Short file.  Truncated number of records to "
                    << numRecords << endl;
            break;
        }
                //position
        positions[i][X] = ConvertFloatAt(attribToColumn[ATTRIB_X]);
        positions[i][Y] = ConvertFloatAt(attribToColumn[ATTRIB_Y]);
        positions[i][Z] = ConvertFloatAt(attribToColumn[ATTRIB_Z]);
                //color
        if (attribToColumn[ATTRIB_R] != -1)
            rgbColor[i][X] = ConvertFloatAt(attribToColumn[ATTRIB_R]);
        if (attribToColumn[ATTRIB_G] != -1)
            rgbColor[i][Y] = ConvertFloatAt(attribToColumn[ATTRIB_G]);
        if (attribToColumn[ATTRIB_B] != -1)
            rgbColor[i][Z] = ConvertFloatAt(attribToColumn[ATTRIB_B]);
        if (attribToColumn[ATTRIB_COLOR] != -1)
        {
            if (colorFlags & COLOR_CODE)
                rgbColor[i] = palette[convertColorCode(fields[attribToColumn[ATTRIB_COLOR]][0])];
            else
                colorVal[i] = ConvertFloatAt(attribToColumn[ATTRIB_COLOR]);
        }
        if (attribToColumn[ATTRIB_TEXTURE_S] != -1)
            colorVal[i] = ConvertFloatAt(attribToColumn[ATTRIB_TEXTURE_S]);
        if (attribToColumn[ATTRIB_TEXTURE_T] != -1)
            texture_t[i] = ConvertFloatAt(attribToColumn[ATTRIB_TEXTURE_T]);
                //size
        if (attribToColumn[ATTRIB_SIZE] != -1)
            size[i] = ConvertFloatAt(attribToColumn[ATTRIB_SIZE]);
        for (int j = X; j <= Z; j++)
        {
            if (attribToColumn[ATTRIB_X_SIZE + j] != -1)
                dimensionalSize[j][i] = ConvertFloatAt(attribToColumn[ATTRIB_X_SIZE + j]);
        }
                //text
        if (attribToColumn[ATTRIB_TEXT] != -1)
            text[i] = ReleaseField(attribToColumn[ATTRIB_TEXT]);
                //glyph
        if (attribToColumn[ATTRIB_GLYPH] != -1)
            glyph[i] = convertGlyphCode(Lookup(fields[attribToColumn[ATTRIB_GLYPH]]));
                //data
        for (unsigned int j = 0; j < numDataFields; j++)
            convertData(fields, &dataRepository[j], 0, i, dataRepository[j].column);
    }
    for (unsigned int j = 0; j < numDataFields; j++)
    {
        if (dataRepository[j].type & (TAKE_LG | TREAT_AS_OFFSET))
            dataRepository[j].type = SmartMemory::Float;
    }
    projectXY(positions);

    if (numRecords <= 1) //don't use the internal stuff if there's just one point
        canUseInternalHandleFlag = GL_FALSE;
    setIndexedAttributeOrdering(numWrapableAttributes, wrapableAttributeNames, DataPointAttributeOrdering);

    return(0);
}

    /*--------------------------------------------------------------------------*/
    Glyph *DataPoints::convertGlyphCode(int code)
{
    return baseGlyph[code - CONE];
}

    /*--------------------------------------------------------------------------*/
    int DataPoints::convertGlyphToCode(Glyph *g)
{
    int i, numGlyphs;

    numGlyphs = UNIT_SPHERE - CONE + 1;
    for (i = 0; i < numGlyphs; i++)
    {
        if (g == baseGlyph[i])
            return i + CONE;
    }
    return CUBE;
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
    int DataPoints::writeTo(FILE *file)
{
    int retval, skipColor;
    numColumns = 0; //count the new number of columns

        //write header
    fprintf(file, "%s %d\n", Keyword(N_RECORDS), numRecords);
    fprintf(file, "%s ", Keyword(GLYPH));
    if (glyphType == COLUMN)
        fprintf(file, "%s %d\n", Keyword(COLUMN), ++numColumns);
    else if (glyphType == POINT)
        fprintf(file, "%s\n", Keyword(POINT));
    else
        fprintf(file, "%s\n", Keyword(glyphType + CONE)); //since we transformed in processField

    fprintf(file, "%s %s\n", Keyword(DROP_LINE), Keyword(dropLine));

    fprintf(file, "%s %s\n", Keyword(SHOW_POINT), Keyword(showPoint));

    retval = GeoData::writeTo(file);
    if (retval != TCL_OK)
        return retval;

        //Write individual point data (Always XYZ, own stuff, color, size, text, data)
    skipColor = GL_FALSE;
    if (attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_X] ||
        attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_Y] ||
        attribToColumn[ATTRIB_COLOR] == attribToColumn[ATTRIB_Z])
        skipColor = GL_TRUE;
    for (unsigned int i = 0; i < numRecords; i++)
    {
                //position
        fprintf(file, "%g %g %g", positions[i][X], positions[i][Y], positions[i][Z]);
                //glyph
        if (glyphType == COLUMN)
            fprintf(file, " %s", Keyword(convertGlyphToCode(glyph[i])));
                //color
        if (attribToColumn[ATTRIB_R] != -1)
            fprintf(file, " %g", rgbColor[i][0]);
        if (attribToColumn[ATTRIB_G] != -1)
            fprintf(file, " %g", rgbColor[i][1]);
        if (attribToColumn[ATTRIB_B] != -1)
            fprintf(file, " %g", rgbColor[i][2]);
        if (attribToColumn[ATTRIB_COLOR] != -1 && !skipColor)
        {
            if (colorFlags & COLOR_CODE)
                fprintf(file, " %c", convertColorCode(convertRGBToColorCode(rgbColor[i])));
            else
                fprintf(file, " %g", colorVal[i]);
        }
        if (attribToColumn[ATTRIB_TEXTURE_S] != -1)
            fprintf(file, " %g", colorVal[i]);
        if (attribToColumn[ATTRIB_TEXTURE_T] != -1)
            fprintf(file, " %g", texture_t[i]);
                //size
        if (attribToColumn[ATTRIB_SIZE] != -1)
        {
            if (sizeFlags & FORMAT_DIMENSION)
            {
                // following commented out because ATTRIB_X_SIZE is > size of dimentionalSize, which is 3
/*                if (attribToColumn[ATTRIB_X_SIZE] >= 0)
                    fprintf(file, " %g", dimensionalSize[ATTRIB_X_SIZE][i]);
                if (attribToColumn[ATTRIB_Y_SIZE] >= 0)
                    fprintf(file, " %f", dimensionalSize[ATTRIB_X_SIZE][i]);
                if (attribToColumn[ATTRIB_Z_SIZE] >= 0)
                    fprintf(file, " %f", dimensionalSize[ATTRIB_X_SIZE][i]);*/
            }
            fprintf(file, " %g", size[i]);
        }
                //text
        if (attribToColumn[ATTRIB_TEXT] != -1)
            fprintf(file, " \"%s\"", text[i]);
                //data
        for (unsigned int j = 0; j <numDataFields; j++)
        {
            if (dataRepository[j].format & FORMAT_BY_COLUMN)
            {
                if (dataRepository[j].type == SmartMemory::String)
                    fprintf(file, " \"%s\"", dataRepository[j].data[0][i].asString());
                else
                    fprintf(file, " %g", dataRepository[j].data[0][i].asFloat());
            }
        }
        fprintf(file, "\n");
    }
    return retval;
}

    /*--------------------------------------------------------------------------*/
    void DataPoints::UpdateTexture(TextureMap *ptr)
{
    float z = 0.0;

    if (colorFlags & COLOR_ATTRIBUTE)
    {
        if (bytewiseColor.empty())
            return;

        if (colorFlags & COLOR_AS_HEIGHT)
            z = pose->getPosition()[Z];

        for (unsigned int i = 0; i < numRecords; i++)
            bytewiseColor[i] = GetColorAtValue(colorVal[i] + z);
    }
}

    /*---------------------------------------------------------------------------*/
    int DataPoints::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    const char *message = argv[0];

    if (argc >= 1)
    {
                //----- init -----
        if (!strcmp(message, "init"))
        {
            if(numRecords>0)
            {
                return TCL_ERROR;
            }
            if(argc >= 2)
            {
                numRecords=atoi(argv[1]);
            }
            else
            {
                return TCL_ERROR;
            }
            init();

            return TCL_OK;
        }
                //----- position -----
        if (!strcmp(message, "position"))
        {
            if(argc >= 5)
            {
                if((unsigned int)(atoi(argv[1])) >= numRecords)
                {
                    return TCL_ERROR;
                }
                positions[atoi(argv[1])][X] = atof(argv[2]);
                positions[atoi(argv[1])][Y] = atof(argv[3]);
                positions[atoi(argv[1])][Z] = atof(argv[4]);
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
                if(atoi(argv[1]) >= (int)numRecords)
                {
                    return TCL_ERROR;
                }
                rgbColor[atoi(argv[1])][X] = atof(argv[2]);
                rgbColor[atoi(argv[1])][Y] = atof(argv[3]);
                rgbColor[atoi(argv[1])][Z] = atof(argv[4]);
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
                if(atoi(argv[1]) >= (int)numRecords)
                {
                    return TCL_ERROR;
                }
                size[atoi(argv[1])] = atof(argv[2]);
            }
            else
            {
                return TCL_ERROR;
            }
            return TCL_OK;
        }
                //----- dropline -----
        if (!strcmp(message, "dropline"))
        {
            if(argc >= 2)
            {
                                
                if(isTrue(argv[1]))
                {
                    dropLine = ON;
                }
                else
                {
                    dropLine = OFF;
                }
                return TCL_OK;
            }
            return TCL_ERROR;
        }
                //----- showpoint -----
        if (!strcmp(message, "showpoint"))
        {
            if(argc >= 2)
            {
                                
                if(isTrue(argv[1]))
                {
                    showPoint = ON;
                }
                else
                {
                    showPoint = OFF;
                }
                return TCL_OK;
            }
            return TCL_ERROR;
        }

    }
    return GeoData::perform(argc, argv, interp);
        
}

    /*--------------------------------------------------------------------------*/
    void DataPoints::DrawHandle(Window3D *win, int index)
{
    int level = 2;
    float size = 5.0;
                
    if (!isEditableFlag)
        return;
/*
    if (renderingForPicking)
    glPushName(index);
*/
    if (index == selectedRecord)
        level = 1;

    pointHandle->drawIn(win, positions[index], level,true, size);

/*
    if (renderingForPicking)
    glPopName();
*/
}

    /*---------------------------------------------------------------------------*/
    void DataPoints::updateWrapper(PosableStateWrapper *wrapper, long &changeMask)
{
    int lineIndex;
    unsigned int index;

    InterpolationSupport::updateWrapper(wrapper, changeMask);

    if (pointHandle->usingWrapper(wrapper))
    {
                //don't allow it to go off the end.
        lineIndex = wrapper->getIntegerIndex(2);
        if (lineIndex != 0)
        {
            lineIndex = 0;
            wrapper->setIndex(lineIndex, 2);
        }
        index = wrapper->getIntegerIndex();
        if (index < 0 || index >= numRecords)
        {
            if (index < 0)
                index = 0;
            else
                index = numRecords - 1;
            wrapper->setIndex(int(index));
        }

        selectedRecord = index;
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
}

    /*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
    void DataPoints::ResetHighRes()
{
    doneFlag = currentRecord = 0;
    if (trailingSeconds > 0.0 && timeAttributeIndex >= 0)
    {
        updateTrailingIndexes();
        currentRecord = trailingStartIndex;
    }
}

    /*-----------------------InterpolationSupport Functions---------------------*/
    /*--------------------------------------------------------------------------*/
std::vector<gz4d::Point<double> > &DataPoints::getPoints(unsigned int whichLine)
{
    return positions;
}

    /*--------------------------------------------------------------------------*/
    int DataPoints::getNumPoints(unsigned int whichLine)
{
    if (whichLine == 0)
        return numRecords;
    return -1;
}

    /*--------------------------------------------------------------------------*/
    GLboolean DataPoints::modifyPoint(unsigned int index, const Point3f newValue, long &changeMask, unsigned int whichLine)
{
    if (whichLine == 0)
    {
        COPY_POINT(positions[index], newValue);
        if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
        {
            colorVal[index] = newValue[Z];
            bytewiseColor[index] = GetColorAtValue(colorVal[index] + pose->getPosition()[Z]);
        }
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        return GL_TRUE;
    }
    return GL_FALSE;
}

                                      /*---------------------------------------------------------------------------*/
                                      int DataPoints::getAttribute(PosableStateWrapper *wrapper, SmartValue &val, int attributeIndex)
{
    int index;
    SmartMemory::IndexType type;

    index = (int)wrapper->getFloatIndex();

    switch (attributeIndex)
    {
        case WAttr_SIZE:
            if (sizeFlags & FORMAT_BY_COLUMN)
                val.set(size[index]);
            else
                val.set(sizeFactor);
            break;
        case WAttr_TEXT:
            if (textFlags == FORMAT_NONE)
                val.set("<none>");
            else
                val.set(text[index], GL_TRUE);
            break;
        case WAttr_GLYPH:
            if (glyphType != COLUMN)
            {
                if (glyphType == POINT)
                    val.set(Keyword(POINT));
                else
                    val.set(Keyword(convertGlyphToCode(baseGlyph[glyphType])));
            }
            else
                val.set(Keyword(convertGlyphToCode(glyph[index])));
            break;
        default:
            attributeIndex -= WAttr_DATA;
            if (attributeIndex < 0 || attributeIndex >= (int)numDataFields)
                return TCL_ERROR;
            else
            {
                type = dataRepository[attributeIndex].type;
                if (type == SmartMemory::String)
                    val.set(dataRepository[attributeIndex].data[0][index].asString());
                else
                {
                    if (wrapper->getIndexType() == SmartValue::Float) //no need to interpolate unless index is a float!
                    {
                        if (InterpolationSupport::getAttribute(wrapper, val, attributeIndex) == TCL_OK)
                            return TCL_OK;
                    }
                    val.set(dataRepository[attributeIndex].data[0][index].asFloat());
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
                                      GLboolean DataPoints::requestAssociativeAttribute(int attributeIndex)
{
    if (attributeIndex < WAttr_DATA)
        return GL_FALSE;

    return InterpolationSupport::requestAssociativeAttribute(attributeIndex);
}

                                      /*---------------------------------------------------------------------------*/
                                      void DataPoints::resetTimeBoundsOnAttribute(PosableStateWrapper *wrapper, int indexingAttribute, double offset, double scale)
{
    float *array;

    if (indexingAttribute < WAttr_DATA)
        return;

    array = dataRepository[indexingAttribute - WAttr_DATA].data[0].array.f;
    bbox.start = array[0] + offset;
    bbox.end = array[numRecords - 1] * scale + offset;
}

                                      /*---------------------------------------------------------------------------*/
float *DataPoints::getAttributeValues(unsigned int whichLine, int attributeIndex)
{
    if (attributeIndex < 0)
        attributeIndex = associativeAttribute - WAttr_DATA;

    if (attributeIndex < 0 || whichLine != 0 || dataRepository[attributeIndex].type != SmartArray::Float)
        return NULL;

    return dataRepository[attributeIndex].data[0].array.f;
}

void DataPoints::SetText(int i, char * t)
{
    unsigned int i1;
    if (textFlags == FORMAT_NONE)
    {
        textFlags = FORMAT_BY_RECORD;
        text = new char *[numRecords];
        for (i1 = 0; i1 < numRecords; i1++)
            text[i1] = NULL;
    }
    if (text[i])
        delete text[i];
    text[i] = strdup(t);
}

                                      /*---------------------------------------------------------------------------*/
int DataPoints::setAttribute(PosableStateWrapper *wrapper, SmartValue &val, int attributeIndex)
{
    unsigned int index, i1, newGlyphType;
    SmartMemory::IndexType type;
    char glyphStr[4];

    index = (int)wrapper->getFloatIndex();

    switch (attributeIndex)
    {
        case WAttr_SIZE:
            if (sizeFlags & FORMAT_BY_COLUMN)
                size[index] = val.asFloat();
            else
                sizeFactor = val.asFloat();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        case WAttr_TEXT:
            if (textFlags == FORMAT_NONE)
            {
                textFlags = FORMAT_BY_RECORD;
                text = new char *[numRecords];
                for (i1 = 0; i1 < numRecords; i1++)
                    text[i1] = NULL;
            }
            if (text[index])
                delete text[index];
            text[index] = strdup(val.asString());
            break;
        case WAttr_GLYPH:
            strncpy(glyphStr, val.asString(), 4);
            glyphStr[3] = '\0';
            newGlyphType = Lookup(glyphStr, GL_TRUE, GL_TRUE);
            if (newGlyphType < 0)
                return TCL_ERROR; //don't change it if we can't decipher it.
            else if (newGlyphType != POINT)
                newGlyphType -= CONE;
            if (glyphType != COLUMN)
                glyphType = newGlyphType;
            else
                glyph[index] = baseGlyph[newGlyphType];
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        default:
            attributeIndex -= WAttr_DATA;
            if (attributeIndex < 0 || attributeIndex >= (int)numDataFields)
                return TCL_ERROR;
            else
            {
                type = dataRepository[attributeIndex].type;
                if (type == SmartMemory::String)
                    dataRepository[attributeIndex].data[0][index].set(val.asString());
                else
                    dataRepository[attributeIndex].data[0][index].set(val.asFloat());
            }
    }
    return InterpolationSupport::setAttribute(wrapper, val, attributeIndex);
}

                                      /*---------------------------------------------------------------------------*/
                                      void DataPoints::updateTrailingIndexes()
{
    double now, startTime;

    now = (Scene3D::timeControl->getGZTime() - timeAttributeOffset) / timeAttributeScale;
    startTime = now - trailingSeconds;

    while (dataRepository[timeAttributeIndex].data[0][trailingStartIndex].asFloat() < startTime)
    {
        if (++trailingStartIndex >= numRecords)
        {
            trailingStartIndex = numRecords - 1;
            break;
        }
    }
    while (dataRepository[timeAttributeIndex].data[0][trailingStartIndex].asFloat() > startTime)
    {
        if (--trailingStartIndex <= 0)
        {
            trailingStartIndex = 0;
            break;
        }
    }

    while (dataRepository[timeAttributeIndex].data[0][trailingEndIndex].asFloat() > now)
    {
        if (--trailingEndIndex <= 0)
        {
            trailingEndIndex = 0;
            break;
        }
    }
    while (dataRepository[timeAttributeIndex].data[0][trailingEndIndex].asFloat() <= now)
    {
        if (++trailingEndIndex >= numRecords)
        {
            trailingEndIndex = numRecords - 1;
            break;
        }
    }
}

                                      /*--------------------------------------------------------------------------*/
#define EMPTY()
#define COPY_RECORD(source, sourceIndex, target, targetIndex) \
{\
    COPY_POINT(target##positions[targetIndex], source##positions[sourceIndex]);\
    if (target##glyph)\
        target##glyph[targetIndex] = source##glyph[sourceIndex];\
    if (colorFlags & FORMAT_BY_COLUMN)\
    {\
        if (!target##bytewiseColor.empty())\
            target##bytewiseColor[targetIndex] = source##bytewiseColor[sourceIndex];\
        else\
            target##rgbColor[targetIndex] = source##rgbColor[sourceIndex];\
        if (target##colorVal)\
            target##colorVal[targetIndex] = source##colorVal[sourceIndex];\
        if (target##texture_t)\
            target##texture_t[targetIndex] = source##texture_t[sourceIndex];\
    }\
}

/*--------------------------------------------------------------------------*/
GLboolean DataPoints::insertItem(int what, unsigned int index, unsigned int whichLine)
{
    std::vector<gz4d::Point<double> > new_positions;
    float **new_rgbColor = NULL;
    //const unsigned char **new_bytewiseColor = NULL;
    std::vector<gz4d::Pixel<gz4d::RGBA8> > new_bytewiseColor;
    float *new_colorVal = NULL;
    float *new_texture_t = NULL;
    Glyph **new_glyph = NULL;
    unsigned int i, newLength, copyPoint, oldLength;
    int i1, i2;
    float c1, c2;

    if (what == PointType)
    {
        oldLength = numRecords;
        if (index < numRecords)
        {
            newLength = numRecords + 1;
            copyPoint = index;
        }
        else
        {
            newLength = index + 1;
            copyPoint = numRecords;
        }

                //create new arrays if we're inserting a line
        if (newLength > allocatedRecords)
        {
            new_positions.resize(newLength);
            if (glyphType == COLUMN)
                new_glyph = new Glyph *[newLength];
            if (colorFlags & COLOR_ATTRIBUTE)
            {
                //new_bytewiseColor = new const unsigned char *[newLength];
                new_bytewiseColor.resize(newLength);
                new_colorVal = new float[newLength];
            }
            else if ((colorFlags & COLOR_TEXTURE) && (colorFlags & FORMAT_BY_COLUMN))
            {
                new_colorVal = new float[newLength];
                if (colorFlags & COLOR_TEXTURE_T)
                    new_texture_t = new float[newLength];
            }
            else if ((colorFlags & COLOR_RGB) || (colorFlags & COLOR_CODE))
                new_rgbColor = new float *[newLength];

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
            new_glyph = glyph;
            new_bytewiseColor = bytewiseColor;
            new_colorVal = colorVal;
            new_texture_t = texture_t;
            new_rgbColor = rgbColor;
        }

                //copy everything past insertion point
        for (i = numRecords; i > copyPoint; i--)
//#ifdef WIN32
#ifdef MSVC
            COPY_RECORD(EMPTY(), i - 1, new_, i);
#else
        COPY_RECORD(, i - 1, new_, i);
#endif

                //insertion in middle or at end
        {
            if (copyPoint >= numRecords) //project last two points out
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
            }
            else //calculate the new point
            {
                new_positions[copyPoint][X] = c1 * new_positions[i1][X] + c2 * new_positions[i2][X];
                new_positions[copyPoint][Y] = c1 * new_positions[i1][Y] + c2 * new_positions[i2][Y];
                new_positions[copyPoint][Z] = c1 * new_positions[i1][Z] + c2 * new_positions[i2][Z];
            }
            if (colorFlags & FORMAT_BY_COLUMN)
            {
                if (!(colorFlags & COLOR_AS_BYTES) && (colorFlags & COLOR_RGB))
                {
                    new_rgbColor[copyPoint] = new Point3f;
                    if (i2 >= 0)
                    {
                        new_rgbColor[copyPoint][X] = new_rgbColor[i2][X];
                        new_rgbColor[copyPoint][Y] = new_rgbColor[i2][Y];
                        new_rgbColor[copyPoint][Z] = new_rgbColor[i2][Z];
                    }
                }
                if (colorFlags & COLOR_CODE)
                    new_rgbColor[copyPoint] = new_rgbColor[i2];
                if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
                    new_colorVal[copyPoint] = new_positions[copyPoint][Z];
                if (colorFlags & COLOR_TEXTURE)
                {
                    new_colorVal[copyPoint] = new_colorVal[i2];
                    if (colorFlags & COLOR_TEXTURE_T)
                        new_texture_t[copyPoint] = new_texture_t[i2];
                }
            }
            if (glyphType == COLUMN)
                new_glyph[copyPoint] = glyph[i2];
        }
                //insertion past end
        for (i = numRecords + 1; i < newLength; i++)
        {
            i1 = i - 2;
            i2 = i - 1;
            if (i1 < 0) //oops, not enough points!
            {
                new_positions[i][X] = 0.0f;
                new_positions[i][Y] = 0.0f;
                new_positions[i][Z] = 0.0f;
            }
            else //calculate the new point
            {
                new_positions[i][X] = -new_positions[i1][X] + 2 * new_positions[i2][X];
                new_positions[i][Y] = -new_positions[i1][Y] + 2 * new_positions[i2][Y];
                new_positions[i][Z] = -new_positions[i1][Z] + 2 * new_positions[i2][Z];
            }
            if (colorFlags & FORMAT_BY_COLUMN)
            {
                if ((colorFlags & COLOR_AS_BYTES) && (colorFlags & COLOR_RGB))
                {
                    new_rgbColor[i] = new Point3f;
                    if (i2 >= 0)
                    {
                        new_rgbColor[i][X] = new_rgbColor[i2][X];
                        new_rgbColor[i][Y] = new_rgbColor[i2][Y];
                        new_rgbColor[i][Z] = new_rgbColor[i2][Z];
                    }
                }
                if (colorFlags & COLOR_CODE)
                    new_rgbColor[i] = new_rgbColor[i2];
                if ((colorFlags & COLOR_ATTRIBUTE) && (colorFlags & COLOR_AS_HEIGHT))
                    new_colorVal[copyPoint] = new_positions[copyPoint][Z];
                if (colorFlags & COLOR_TEXTURE)
                {
                    new_colorVal[copyPoint] = new_colorVal[i2];
                    if (colorFlags & COLOR_TEXTURE_T)
                        new_texture_t[copyPoint] = new_texture_t[i2];
                }
            }
            if (glyphType == COLUMN)
                new_glyph[i] = glyph[i2];
        }

        if (new_positions != positions)
        {
            if (colorFlags & FORMAT_BY_COLUMN)
            {
//                 if (new_bytewiseColor)
//                     delete []bytewiseColor;
//                 else
                if(new_bytewiseColor.empty())
                {
                    if (new_rgbColor)
                    {
                        if (!(colorFlags & COLOR_CODE))
                        {
                            for (i = 0; i < numRecords; i++)
                                delete rgbColor[i];
                        }
                        delete []rgbColor;
                    }
                }
                if (new_colorVal)
                    delete []colorVal;
                if (new_texture_t)
                    delete []texture_t;
            }
            if (new_glyph)
                delete []glyph;

            rgbColor = new_rgbColor;
            glyph = new_glyph;
            bytewiseColor = new_bytewiseColor;
            colorVal = new_colorVal;
            texture_t = new_texture_t;
            positions = new_positions;
        }

        insertRecord(index); //do standard GeoData insertion.

        UpdateTexture(NULL);
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());

        return GL_TRUE;
    }
    return GL_FALSE;
}

/*--------------------------------------------------------------------------*/
GLboolean DataPoints::deleteItem(int what, unsigned int index, unsigned int whichLine)
{
    unsigned int i, oldNumRecords;

    if (what == PointType)
    {
        if (index < numRecords && index >= 0)
        {
            if (numRecords <= 1) //don't allow deletion of last point
                return GL_FALSE;
            oldNumRecords = numRecords;
            deleteRecord(index); //do standard GeoData deletion.
            if (colorFlags & FORMAT_BY_COLUMN && !(colorFlags & COLOR_AS_BYTES)
                && (colorFlags & COLOR_RGB))
                delete rgbColor[index];
            for (i = index + 1; i < oldNumRecords; i++)
//#ifdef WIN32
#ifdef MSVC
                COPY_RECORD(EMPTY(), i, EMPTY(), i - 1);
#else
            COPY_RECORD(, i,, i - 1);
#endif
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return GL_TRUE;
        }
    }
    return GL_FALSE;
}

                                      void DataPoints::setPointSize(double s)
{
    sizeFactor = s;
}


void DataPoints::init(int n)
{
    unsigned int i;

    if(n>0)
    {
        numRecords = n;
    }

    positions.resize(numRecords);
    sizeFactor = 1.0;
    rgbColor = new float*[numRecords];
    sizeFlags = FORMAT_CONSTANT;
    glyphType = UNIT_SPHERE;

    for (i = 0; i < numRecords; i++)
    {
        positions[i][X] = 0.0;
        positions[i][Y] = 0.0;
        positions[i][Z] = 0.0;
        rgbColor[i] = new Point3f;
        rgbColor[i][X] = 1.0;
        rgbColor[i][Y] = 1.0;
        rgbColor[i][Z] = 1.0;
    }
}
