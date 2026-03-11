// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Caption.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This class provides for drawing captions that change over time.
 *  This class implements the following responsibilities:
 *      * Read caption description from a file
 *      * Support viewing of the caption.
 *
 *  Created by mdp 1/4/2007
 */
/*---------------------------------------------------------------------------*/

#include "GZincludes.h"
#include "Caption.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
#include "engine/gzCommandInterface.h"
#include "engine/PosableStateBase.h"
//#include <iostream>
#include <math.h>
#include "utilities/DottedAddress.h"
#include "engine/Scene3D.h"
#include "engine/TimeControl.h"

using namespace std;

extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(Caption, "cap");

const char *Caption::CaptionNames[] =
            {OBJECT_NAMES, "TEXT", "FONT", "COLOR", "CAPTION",
             "LINE", "BACKGROUND", "LOCATION",
             "ALWAYS", "AUTO", "DEFAULT", "SHADOW",
             "BOX", "OUTLINED_BOX",
             "TRANSIENT", "FRAGILE", "ROBUST", "OFF", "N_RECORDS",
             "NEW", "STANDARD_9_BY_15", "STANDARD_8_BY_13",
             "TIMES_ROMAN_10", "TIMES_ROMAN_24", "HELVETICA_10",
             "HELVETICA_12", "HELVETICA_18",
             ""};

//#define FONT GLUT_BITMAP_9_BY_15
//#define FONT GLUT_BITMAP_8_BY_13
//#define FONT GLUT_BITMAP_TIMES_ROMAN_10
//#define FONT GLUT_BITMAP_TIMES_ROMAN_24
//#define FONT GLUT_BITMAP_HELVETICA_10
//#define FONT GLUT_BITMAP_HELVETICA_12
//#define FONT GLUT_BITMAP_HELVETICA_18

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*--------------------------------------------------------------------------*/
Caption::Caption()
{
    SetTypeAndVersion("CAPTION", 1, 0);
    SetKeywordTable(CaptionNames);

    numSpecs = 0;
    timeMin = timeMax = 0;
    timeSpecs = NULL;
}

/*--------------------------------------------------------------------------*/
Caption::~Caption()
{
    int i;
    if (numSpecs)
    {
        for (i = 0; i < numSpecs; i++)
        {
            if (timeSpecs[i].text)
                delete timeSpecs[i].text;
        }
        delete []timeSpecs;
    }
    if (defaultSpec.text)
        delete defaultSpec.text;
}

/*--------------------------------------------------------------------------*/
Caption::CaptionSpec::CaptionSpec()
{
    startTime = endTime = 0;
    text = NULL;
    font = GLUT_BITMAP_HELVETICA_18;
    color[0] = color[1] = color[2] = color[3] = 0xFF;
    backgroundColor[0] = backgroundColor[1] = backgroundColor[2] = 0x00;
    backgroundColor[3] = 0x00;
    behaviorFlags = UseDefaultMask;
    robustness = ROBUST;
    usageFlags = Blank;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Caption::DrawLowRes(float degradation, Window3D *win)
{
    int robustVal = ROBUST;

    DetermineSpec(win);

    if (!currentSpec)
        return;
    if (currentSpec->usageFlags & UseRobustness)
        robustVal = currentSpec->robustness;
    else if (defaultSpec.usageFlags & UseRobustness)
        robustVal = defaultSpec.robustness;

    if (robustVal != OFF && robustVal != FRAGILE)
    {
        if (isEditableFlag)
            DrawEditGhosts(win, UseTransientVal);
        else
            DrawSpec(win, UseTransientVal);
    }

    ResetHighRes();
}

/*--------------------------------------------------------------------------*/
int Caption::DrawHighRes(int eye, Window3D *win)
{
    int robustVal = ROBUST;

    if (!doneFlag)
    {
        DetermineSpec(win);

        if (!currentSpec)
        {
            doneFlag = 1;
            return doneFlag;
        }

        if (currentSpec->usageFlags & UseRobustness)
            robustVal = currentSpec->robustness;
        else if (defaultSpec.usageFlags & UseRobustness)
            robustVal = defaultSpec.robustness;

        if (robustVal != OFF && robustVal != TRANSIENT)
        {
            if (isEditableFlag)
                DrawEditGhosts(win, UseFragileVal);
            else
                DrawSpec(win, UseFragileVal);
        }
        else
        {
            if (isEditableFlag)
                DrawEditGhosts(win, UseFragileVal);
            doneFlag = 1;
        }

        if (eye != dominantEye && eye != MONO_EYE)
            return doneFlag;
        else
            doneFlag = 1;
    }
    else
        doneFlag = 1;
    
    return doneFlag;
}

/*--------------------------------------------------------------------------*/
void Caption::DrawSpec(Window3D *win, int robustnessType, GLboolean asEditGhost)
{
    char *text, *nextPortion;
    void *font;
    Point2d pos = {0.0,0.0};
    GLboolean heightAndWidthValid = GL_FALSE;
    float xOffset, yOffset;
    int usageFlags = currentSpec->usageFlags;
    int behaviorFlags = (defaultSpec.behaviorFlags & ~usageFlags) |
                        (currentSpec->behaviorFlags & usageFlags);
    int backgroundRobustness = UseBackgroundRobustnessMask & robustnessType & behaviorFlags;
    GLboolean shadow = ((behaviorFlags & UseBoxOrShadowMask) == UseShadow && backgroundRobustness);
    GLboolean outline = ((behaviorFlags & UseBoxOrShadowMask) == UseBoxWithOutline && backgroundRobustness);
    GLboolean filledBox = ((behaviorFlags & UseBoxOrShadowMask) == UseBackgroundBox && backgroundRobustness);
    //float scale = win->getPoseInfo()->getScale(), scaleFactor = 1.0;
    float w, h, lineHeight, lineSpacing;
    int linesLeft;

    ViewManager *view = win->getViewInfo();
    const GLint *viewport = view->getViewport();
/*
    if (isEditable())
    {
        scaleFactor = scale / currentSpec->scaleMin;
        if (scaleFactor > 1.0f)
            scaleFactor = 1.0f;
    }
*/
    glShadeModel(GL_FLAT);
    glNormal3f(0.0, 0.0, 1.0);

//  glDepthMask(GL_FALSE);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (usageFlags & UseText)
    {
        text = currentSpec->text;
        heightAndWidthValid = GL_TRUE;
        w = (float)currentSpec->width;
        h = (float)currentSpec->height;
        lineHeight = (float)currentSpec->lineHeight;
        lineSpacing = (float)currentSpec->lineSpacing;
        linesLeft = currentSpec->numLines;
    }
    else
    {
        text = defaultSpec.text;
        w = (float)defaultSpec.width;
        h = (float)defaultSpec.height;
        lineHeight = (float)defaultSpec.lineHeight;
        lineSpacing = (float)defaultSpec.lineSpacing;
        linesLeft = defaultSpec.numLines;
    }

    if (usageFlags & UseFont)
    {
        font = currentSpec->font;
        heightAndWidthValid = GL_TRUE;
    }
    else
        font = defaultSpec.font;

    if (usageFlags & UseColor)
        glColor4ubv(currentSpec->color);
    else
        glColor4ubv(defaultSpec.color);

    if (behaviorFlags & UseLocation)
    {
        pos[X] = currentSpec->location[X];
        pos[Y] = currentSpec->location[Y];
    }
    else
    {
        pos[X] = defaultSpec.location[X];
        pos[Y] = defaultSpec.location[Y];
    }

    /*
    if (heightAndWidthValid)
    {
        xOffset = -currentSpec->width / 2;
        yOffset = -currentSpec->height / 2;
    }
    else
    {
        xOffset = -defaultSpec.width / 2;
        yOffset = -defaultSpec.height / 2;
    }
    */

    lineSpacing = (lineHeight + lineSpacing) / viewport[3]; //convert to proper coordinates
    if (!asEditGhost && !renderingForPicking)
    {
        nextPortion = text;
        bool flipped = false;
        while (*nextPortion)
        {
            while (*nextPortion && *nextPortion != '\n')
                nextPortion++; //fast-forward
            if (*nextPortion)
            {
                *nextPortion = '\0';
                flipped = true;
            }
            linesLeft--;
            win->printAt(pos[X], pos[Y] + linesLeft * lineSpacing, text, shadow, font);
            if (flipped)
            {
                *nextPortion = '\n';
                flipped = false;
                nextPortion++;
            }
            text = nextPortion;
        }
    }

    if (outline || filledBox || asEditGhost || renderingForPicking)
    {
        GLboolean drawBackground = GL_TRUE;

        glLineWidth(1.0);
        xOffset = 10.0 / viewport[2]; //How much buffer to give the box
        yOffset = 10.0 / viewport[3]; 
        w /= viewport[2];
        h /= viewport[3];

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
//      gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        if (outline || isEditableFlag)
        {
            if (outline || asEditGhost)
            {
                glBegin(GL_LINE_LOOP);
                glVertex3d(pos[X] + w + xOffset, pos[Y] + h + yOffset, -1.0);
                glVertex3d(pos[X] - xOffset, pos[Y] + h + yOffset, -1.0);
                glVertex3d(pos[X] - xOffset, pos[Y] - yOffset, -1.0);
                glVertex3d(pos[X] + w + xOffset, pos[Y] - yOffset, -1.0);
                glEnd();
            }
        }

        if (usageFlags & UseBackgroundColor)
        {
            if (currentSpec->backgroundColor[3] <= 0.0)
                drawBackground = GL_FALSE;
            else
                glColor4ubv(currentSpec->backgroundColor);
        }
        else
        {
            if (defaultSpec.backgroundColor[3] <= 0.0)
                drawBackground = GL_FALSE;
            else
                glColor4ubv(defaultSpec.backgroundColor);
        }

        if (drawBackground || renderingForPicking)
        {
            glBegin(GL_QUADS);
            glVertex3d(pos[X] - xOffset, pos[Y] - yOffset, -1.0f);
            glVertex3d(pos[X] + w + xOffset, pos[Y] - yOffset, -1.0f);
            glVertex3d(pos[X] + w + xOffset, pos[Y] + h + yOffset, -1.0f);
            glVertex3d(pos[X] - xOffset, pos[Y] + h + yOffset, -1.0f);
            glEnd();
        }
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
//  glBlendFunc(GL_ONE, GL_ZERO);
//  glDepthMask(GL_TRUE);
}

/*--------------------------------------------------------------------------*/
void Caption::DetermineSpec(Window3D *win)
{
    double now;
    int i = -1;
    CaptionSpec *nextSpec;

    now = win->getScene3D()->timeControl->getGZTime();

    currentSpec = NULL;
    nextSpec = &defaultSpec;
    if (timeMin > now || timeMax < now)
        return;

    while (1)
    {
        if (nextSpec->startTime > now)
            return;

        currentSpec = nextSpec;

        if (currentSpec->startTime <= now && currentSpec->endTime >= now)
            return;
        else
            currentSpec = &defaultSpec;

        i++;
        if (i >= numSpecs)
            break;
        nextSpec = &(timeSpecs[i]);
    }
}

/*--------------------------------------------------------------------------*/
void Caption::DrawEditGhosts(Window3D *win, int robustnessType)
{
    int i = -1;
    CaptionSpec *validSpec;

    validSpec = currentSpec;
    currentSpec = NULL;

    for (currentSpec = &defaultSpec; i < numSpecs; currentSpec = &(timeSpecs[++i]))
    {
        if (currentSpec == validSpec)
            DrawSpec(win, robustnessType);
        else
            DrawSpec(win, robustnessType, GL_TRUE);
    }
}

/*--------------------------------------------------------------------------*/
int Caption::processField(int option, char **fields)
{
    int i = 1;
    int val, numArgs;

    switch(option)
    {
        case N_RECORDS:
            if (currentSpec != &defaultSpec)
            {
                cerr << "N_RECORDS keyword not allowed after LOCAL_ORIGIN line." << endl;
                return 1;
            }
            numSpecs = ConvertInt(i);
            timeSpecs = new CaptionSpec[numSpecs];
            break;
        case CAPTION:
            currentSpec->robustness = Lookup(fields[i++]);
            currentSpec->usageFlags |= UseRobustness;
            break;
        case TEXT:
            currentSpec->text = ReleaseField(i);
            currentSpec->usageFlags |= UseText;
            determinePrintedSize();
            break;
        case FONT:
            currentSpec->font = ConvertFont(fields[i++]);
            if (defaultSpec.font == NULL)
                return 1;
            currentSpec->usageFlags |= UseFont;
            determinePrintedSize();
            break;
        case COLOR: //applies to text, line, and box outline
            ConvertFloatColor(currentSpec->color, i);
            currentSpec->usageFlags |= UseColor;
            break;
        case LINE:
            //line display policy
            val = Lookup(fields[i++]);
            currentSpec->behaviorFlags &= ~UseLineMask;
            switch (val)
            {
                case ALWAYS:
                    currentSpec->behaviorFlags |= UseLineAlways;
                    break;
                case AUTO:
                    currentSpec->behaviorFlags |= UseLineAuto;
                    break;
                case OFF:
                    currentSpec->behaviorFlags |= UseLineNever;
                    break;
                case DEFAULT:
                    currentSpec->behaviorFlags |= UseDefaultMask & UseLineMask;
                    break;
                default:
                    cerr << "Invalid LINE specification.  Should be" << endl;
                    cerr << "one of ALWAYS, AUTO, DEFAULT, or OFF." << endl;
                    return 1;
                    break;
            }
            if (val != DEFAULT)
                currentSpec->usageFlags |= UseLineMask;
            //optional line robustness
            numArgs = getNumFields();
            if (numArgs <= i)
                break;
            currentSpec->behaviorFlags &= ~UseLineRobustnessMask;
            val = ConvertRobustness(fields[i++]);
            if (val == -1)
                return 1;
            if (val == -2)
                currentSpec->behaviorFlags |= UseDefaultMask & UseLineRobustnessMask;
            else
            {
                currentSpec->behaviorFlags |= (val & UseLineRobustnessMask);
                currentSpec->usageFlags |= UseLineRobustnessMask;
            }
            break;
        case BACKGROUND:
            //background display policy
            numArgs = getNumFields();
            val = Lookup(fields[i++]);
            if (val == COLOR)
            {
                ConvertFloatColor(currentSpec->backgroundColor, i);
                currentSpec->usageFlags |= UseBackgroundColor;
                if (numArgs <= i)
                    break;
                val = Lookup(fields[i++]);
            }
            currentSpec->behaviorFlags &= ~UseBoxOrShadowMask;
            switch (val)
            {
                case SHADOW:
                    currentSpec->behaviorFlags |= UseShadow;
                    break;
                case BOX:
                    currentSpec->behaviorFlags |= UseBackgroundBox;
                    break;
                case OUTLINED_BOX:
                    currentSpec->behaviorFlags |= UseBoxWithOutline;
                    break;
                case OFF:
                    break;
                case DEFAULT:
                    currentSpec->behaviorFlags |= UseDefaultMask & UseBoxOrShadowMask;
                    break;
                default:
                    cerr << "Invalid BACKGROUND specification.  Should be" << endl;
                    cerr << "one of SHADOW, BOX, OUTLINED_BOX, DEFAULT, or OFF." << endl;
                    return 1;
                    break;
            }
            if (val != DEFAULT)
                currentSpec->usageFlags |= UseBoxOrShadowMask;
            //optional background robustness
            if (numArgs <= i)
                break;
            currentSpec->behaviorFlags &= ~UseBackgroundRobustnessMask;
            val = ConvertRobustness(fields[i++]);
            if (val == -1)
                return 1;
            if (val == -2)
                currentSpec->behaviorFlags |= UseDefaultMask & UseBackgroundRobustnessMask;
            else
            {
                currentSpec->behaviorFlags |= (val & UseBackgroundRobustnessMask);
                currentSpec->usageFlags |= UseBackgroundRobustnessMask;
            }
            break;
        case LOCATION:
            currentSpec->location[X] = ConvertFloat(i);
            currentSpec->location[Y] = ConvertFloat(i);
            currentSpec->usageFlags |= UseLocation;
            break;
        case PROJECT: case REVERSED_XY:
            if (currentSpec == &defaultSpec)
            {
                cerr << fields[0] << " is not yet supported by Caption." << endl;
                return 1;
            }
            break;
        default:
            if (currentSpec == &defaultSpec)
                return Object::processField(option, fields);
            break;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int Caption::readFrom(FILE *infile)
{
    char **fields;
    int spec = 0, i, item, retval;
    int numArgs;
    bool haveTimes = false;
    
    //parse header
    currentSpec = &defaultSpec;
    if (Object::readFrom(infile))
        return 1;

    //parse scale-specs.
    for (fields = NextLine(); fields; fields = NextLine())
    {
        i = 0;
        item = Lookup(fields[i++]);
        if (item == NEW) //note: these should be in chronological order.
        {
            numArgs = getNumFields();
            if (spec >= numSpecs) //skip any more records than N_RECORDS
                return 0;
            
            //get start time
            currentSpec = &(timeSpecs[spec++]);
            currentSpec->startTime = ConvertDouble(i);

            //get end time
            if (i < numArgs)
            {
                currentSpec->endTime = ConvertDouble(i);
            }
            else
                currentSpec->endTime = currentSpec->startTime + 10.0;

            if (!haveTimes)
            {
                timeMin = currentSpec->startTime;
                timeMax = currentSpec->endTime;
                haveTimes = true;
            }
            else
            {
                if (currentSpec->startTime < timeMin)
                    timeMin = currentSpec->startTime;
                if (currentSpec->endTime > timeMax)
                    timeMax = currentSpec->endTime;
            }

            //get text
            if (i < numArgs)
            {
                currentSpec->text = ReleaseField(i);
                currentSpec->usageFlags |= UseText;
                determinePrintedSize();
            }
        }
        else
        {
            retval = processField(item, fields);
            if (retval != 0)
                return retval;
        }
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
int Caption::writeTo(FILE *file)
{
    int i;

    fprintf(file, "%s %d\n", Keyword(N_RECORDS), numSpecs);
    if (defaultSpec.usageFlags & UseText)
        fprintf(file, "%s \"%s\"\n", Keyword(TEXT), defaultSpec.text);
    currentSpec = &defaultSpec;
    writeSpecTo(file);

    Object::writeTo(file);

    for (i = 0; i < numSpecs; i++)
    {
        currentSpec = &(timeSpecs[i]);
        if (currentSpec->usageFlags & UseText)
            fprintf(file, "%s %g %g \"%s\"\n", Keyword(NEW), currentSpec->startTime, currentSpec->endTime, currentSpec->text);
        else
            fprintf(file, "%s %g %g\n", Keyword(NEW), currentSpec->startTime, currentSpec->endTime);
        writeSpecTo(file);
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int Caption::writeSpecTo(FILE *file)
{
    int word;

    if (currentSpec->usageFlags & UseRobustness)
        fprintf(file, "%s %s\n", Keyword(CAPTION), Keyword(currentSpec->robustness));
    if (currentSpec->usageFlags & UseFont)
        fprintf(file, "%s %s\n", Keyword(FONT), ConvertFont(currentSpec->font));
    if (currentSpec->usageFlags & UseColor)
    {
        fprintf(file, "%s", Keyword(COLOR));
        WriteFloatColor(file, currentSpec->color);
        fprintf(file, "\n");
    }
    if ((currentSpec->usageFlags & UseLineMask) || (currentSpec->usageFlags & UseLineRobustnessMask))
    {
        if (currentSpec->usageFlags & UseLineMask)
        {
            switch (currentSpec->behaviorFlags & UseLineMask)
            {
                case UseLineAlways:
                    word = ALWAYS;
                    break;
                case UseLineAuto:
                    word = AUTO;
                    break;
                case UseLineNever:
                    word = OFF;
                    break;
                default:
                    word = DEFAULT;
                    break;
            }
        }
        else
            word = DEFAULT;
        fprintf(file, "%s %s", Keyword(LINE), Keyword(word));
        if (currentSpec->usageFlags & UseLineRobustnessMask)
            fprintf(file, " %s", ConvertRobustness(currentSpec->behaviorFlags, UseLineRobustnessMask));
        fprintf(file, "\n");
    }
    if ((currentSpec->usageFlags & UseBackgroundColor)
        || (currentSpec->usageFlags & UseBoxOrShadowMask)
        || (currentSpec->usageFlags & UseBackgroundRobustnessMask))
    {
        fprintf(file, "%s", Keyword(BACKGROUND));
        if (currentSpec->usageFlags & UseBackgroundColor)
        {
            fprintf(file, " %s", Keyword(COLOR));
            WriteFloatColor(file, currentSpec->backgroundColor, GL_TRUE);
        }
        if (currentSpec->usageFlags & UseBoxOrShadowMask)
        {
            switch (currentSpec->behaviorFlags & UseBoxOrShadowMask)
            {
                case Blank:
                    word = OFF;
                    break;
                case UseShadow:
                    word = SHADOW;
                    break;
                case UseBackgroundBox:
                    word = BOX;
                    break;
                case UseBoxWithOutline:
                    word = OUTLINED_BOX;
                    break;
                default: //should be impossible
                    word = DEFAULT;
                    break;
            }
        }
        else
            word = DEFAULT;
        fprintf(file, " %s", Keyword(word));
        if (currentSpec->usageFlags & UseBackgroundRobustnessMask)
            fprintf(file, " %s", ConvertRobustness(currentSpec->behaviorFlags, UseBackgroundRobustnessMask));
        fprintf(file, "\n");
    }
    if (currentSpec->usageFlags & UseLocation)
    {
        fprintf(file, "%s %g %g\n", Keyword(LOCATION), currentSpec->location[X], currentSpec->location[Y]);
    }
    return TCL_OK;
}

/*--------------------------------------------------------------------------*/
void *Caption::ConvertFont(char *fontName)
{
    int result = Lookup(fontName);

    switch (result)
    {
        case STANDARD_9_BY_15:
            return GLUT_BITMAP_9_BY_15;
            break;
        case STANDARD_8_BY_13:
            return GLUT_BITMAP_8_BY_13;
            break;
        case TIMES_ROMAN_10:
            return GLUT_BITMAP_TIMES_ROMAN_10;
            break;
        case TIMES_ROMAN_24:
            return GLUT_BITMAP_TIMES_ROMAN_24;
            break;
        case HELVETICA_10:
            return GLUT_BITMAP_HELVETICA_10;
            break;
        case HELVETICA_12:
            return GLUT_BITMAP_HELVETICA_12;
            break;
        case HELVETICA_18:
            return GLUT_BITMAP_HELVETICA_18;
            break;
        default:
            cerr << "Invalid FONT: " << fontName << "." << endl;
            cerr << "Should be one of STANDARD_9_BY_15, STANDARD_8_BY_13, TIMES_ROMAN_10," << endl;
            cerr << "TIMES_ROMAN_24, HELVETICA_10, HELVETICA_12, or HELVETICA_18" << endl;
            break;
    }
    return NULL;
}

/*--------------------------------------------------------------------------*/
const char *Caption::ConvertFont(void *font)
{
    int result;

    if (font == GLUT_BITMAP_9_BY_15)
        result = STANDARD_9_BY_15;
    else if (font == GLUT_BITMAP_8_BY_13)
        result = STANDARD_8_BY_13;
    else if (font == GLUT_BITMAP_TIMES_ROMAN_10)
        result = TIMES_ROMAN_10;
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24)
        result = TIMES_ROMAN_24;
    else if (font == GLUT_BITMAP_HELVETICA_10)
        result = HELVETICA_10;
    else if (font == GLUT_BITMAP_HELVETICA_12)
        result = HELVETICA_12;
    else if (font == GLUT_BITMAP_HELVETICA_18)
        result = HELVETICA_18;
    else
        result = HELVETICA_10;

    return Keyword(result);
}

/*--------------------------------------------------------------------------*/
int Caption::ConvertRobustness(char *field)
{
    int result = Lookup(field);

    switch (result)
    {
        case TRANSIENT:
            return UseTransientVal;
            break;
        case FRAGILE:
            return UseFragileVal;
            break;
        case ROBUST:
            return UseRobustVal;
            break;
        case OFF:
            return Blank;
            break;
        case DEFAULT:
            return -2;
        default:
            cerr << "Invalid robustness value: " << field << ".  Should be" << endl;
            cerr << "one of TRANSIENT, FRAGILE, ROBUST, DEFAULT, or OFF." << endl;
            break;
    }
    return -1;
}

/*--------------------------------------------------------------------------*/
const char *Caption::ConvertRobustness(int robustness, int mask)
{
    int result = DEFAULT;

    robustness &= mask;

    if (robustness == (UseTransientVal & mask))
        result = TRANSIENT;
    else if (robustness == (UseFragileVal & mask))
        result = FRAGILE;
    else if (robustness == (UseRobustVal & mask))
        result = ROBUST;
    else if (robustness == (Blank & mask))
        result = OFF;

    return Keyword(result);
}

/*--------------------------------------------------------------------------*/
void Caption::determinePrintedSize()
{
    unsigned char *text, *nextPortion;
    void *font;
    int usageFlags = currentSpec->usageFlags;
    int lines, currentWidth, width, height;
    
    if (usageFlags & UseText)
        text = (unsigned char *)currentSpec->text;
    else
        text = (unsigned char *)defaultSpec.text;

    if (!text)
    {
        currentSpec->height = currentSpec->width = 0;
        return;
    }

    if (usageFlags & UseFont)
        font = currentSpec->font;
    else
        font = defaultSpec.font;

    nextPortion = text;
    bool flipped = false;
    width = 0;
    lines = 0;
    while (*nextPortion)
    {
        while (*nextPortion && *nextPortion != '\n')
            nextPortion++; //fast-forward
        if (*nextPortion)
        {
            *nextPortion = '\0';
            flipped = true;
        }
        currentWidth = glutBitmapLength(font, text) * 2;
        lines++;
        if (width < currentWidth)
            width = currentWidth;
        if (flipped)
        {
            *nextPortion = '\n';
            flipped = false;
            nextPortion++;
        }
        text = nextPortion;
    }
    currentSpec->width = width;
    currentSpec->numLines = lines;
    height = 0;
    if(font == GLUT_BITMAP_9_BY_15)
    {
        height = 20;
    }
    else if (font == GLUT_BITMAP_8_BY_13)
    {
        height = 18;
    }
    else if (font == GLUT_BITMAP_TIMES_ROMAN_10)
    {
        height = 15;
    }
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24)
    {
        height = 29;
    }
    else if (font == GLUT_BITMAP_HELVETICA_10)
    {
        height = 15;
    }
    else if (font == GLUT_BITMAP_HELVETICA_12)
    {
        height = 17;
    }
    else if (font == GLUT_BITMAP_HELVETICA_18)
    {
        height = 23;
    }
    currentSpec->lineHeight = height;
    currentSpec->lineSpacing = (height * 0.8);
    currentSpec->height = (int)(height * lines + (currentSpec->lineSpacing * (lines - 1))); //calculate for space between
}
