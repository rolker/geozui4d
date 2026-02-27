/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// Gutm.cpp: implementation of the Gridded UTM class.
//
// In this class data values of 0.0 represent holes
// negative data values represent temporarily hidden areas
//
//////////////////////////////////////////////////////////////////////

#include "GZincludes.h"
#include "objects/NavigationPath.h"
#include <math.h>
#include <stdio.h>
#include "objects/gutm.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "widgets/Sv_Standards.h"
#include "engine/ConversionGrid.h"
#include "engine/ConversionMethod.h"
#include "utilities/DottedAddress.h"
#include "TimeVaryingObject.h"
#include <filesystem>
#include "objects/DynamicTides.h"
#include "engine/gzCommandInterface.h"
#include "interactors/RubberbandInteractor.h"
#include "engine/PosableStateBase.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/TimeControl.h"

#ifdef USE_BAG_FILES
//#define BEE_STATIC //for bag.h
#include "bag.h"
#endif

// only for MSDEV 6.0
#ifdef WIN32
#if (_MSC_VER == 1200) && (WINVER < 0x0500)
extern "C" long _ftol( double ); //defined by VC6 C libs
extern "C" long _ftol2( double dblSource ) { return _ftol( dblSource ); }
#endif
#endif

//These defines are for generating files from the High-res pass of rendering
//#define GENERATE_GDT
//#define GENERATE_FULLGRID
#if defined(GENERATE_GDT) || defined(GENERATE_FULLGRID)
#include <stdio.h>
FILE *tempFile = NULL;
int fileDone = 0;
#endif

extern int dominantEye;
extern MainWindow * mainWindow;

#define HIGH_RES_ROW_STEP 10
//#define HIGH_RES_SIZE 200  // the size of the high res window
#define HIGH_RES_SIZE 2000  // the size of the high res window
#define HYBRID_HIGHRES 2

#define LOW_TO_HIGH_RES_RATIO 6

#define FUSION_CELLSIZE_RATIO_THRESHOLD 10  //this can be no larger than 11 and still work with the code used (because of the char array for keeping track of holes)--can't use unsigned either, but could increase to short or int or long.

#define NEAR_MOD(val, modulus) ((val < 0)? val + modulus : ((val >= modulus)? (val - modulus) : val))

REGISTER_OBJECT_WITH_MENU(Gutm, "gutm") = "{"
DEFAULT_OBJECT_MENU_ITEMS
"{Fuse} fuse "
"{Restore holes} restore "
"{Rubberband} {rubberband} "
"}";

#ifdef USE_BAG_FILES
REGISTER_ALIAS(Bag, Gutm, "bag");
#endif

const char *GutmNames[] = {OBJECT_NAMES, "COLS_ROWS", "CELLSIZE",
                        "SHAPE", "FULLGRID", "PARTIAL",
                        "USE_ALTERNATE_DATA", "OFFSET", "BINARY", "SHADE_EXAGGERATION", ""};

int Gutm::useHighResZone = GL_FALSE;
GLboolean Gutm::normalsOnTheFly = GL_TRUE;
GLfloat Gutm::surfaceColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
Gutm::Gutm(int rt)
{
    // set up the reader to look for BEGIN GUTM 1.1, and with set of keywords
    SetTypeAndVersion("GUTM", 1, 1);
    SetKeywordTable(GutmNames);
    
    maxx = 0;
    maxy = 0;
    drawrow = 0;
    cellsize = 0;
    dem = NULL;
    dem_data = NULL;
    alternateData = NULL;
    shape = PARTIAL;
    highResSize = HIGH_RES_SIZE;
    hasAlternateData = false;
    bounds = allBounds[OuterBounds];
    currentView = NULL;
    alternateDataOffset = 0.0f;
    binary = 0;
    frontLengthFactor = 1.0;
    
    priority = 10;
    
    norms_array = NULL;
    norms_data = NULL;
    
    rtflag = rt;
    rtdata = NULL;
    east = NULL;
    north = NULL;

    shadeExaggeration = 1.0;

    dTides = NULL;
    tideTimeOffsets = NULL;
    tideMultipliers = NULL;
    lastDynamicTides = NULL;
    plannedPathTime = NULL;
    lastNavPath = NULL;
    navPathUpdateCounter = -1;

    rubberBand = NULL;

    isBag = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void Gutm::initializeAs(const char* alias)
{
#ifdef USE_BAG_FILES
    if (strcmp(alias, "Bag") == 0)
        isBag = GL_TRUE;
#endif
}

/*---------------------------------------------------------------------------*/
Gutm::~Gutm()
{
    delete[] dem;
    delete[] dem_data;
    delete[] norms_data;
    delete[] norms_array;
    delete[] tideTimeOffsets;
    delete[] tideMultipliers;
    delete[] plannedPathTime;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Gutm::DrawLowRes(float degradation, Window3D *win)
{
    PosableStateBase *winPose;
    float heightAboveHighestPoint;

    if (!dem)
        return;

    glColor3f(1.0,1.0,1.0);
    glShadeModel(GL_SMOOTH);
    currentView = win->getViewInfo();
    if (useHighResZone == GL_TRUE)
        DrawUsingHighResZone(degradation, win);
    else
    {
        int innerStep, outerStep;  // inner and outer step size
        GLfloat pitch, zComp;

        winPose = currentView->getPosable();

        if (useHighResZone == HYBRID_HIGHRES)
        {
            pitch = winPose->getOrientationAxis()[Pitch];
            zComp = (float)currentView->getZComponentOfUnderPlane();
            if ((pitch >= 0 && zComp >= 0) || (pitch < 0 && zComp <= 0))
            {
                DrawUsingHighResZone(degradation, win);
                goto doneWithGutmDrawLowRes;
            }
        }

        drewBox = GL_FALSE;

        //determine stepping
        float PixSize, cellsPerPixel, scale;

        PixSize = (float)currentView->getPixelSize();
        if (PixSize < 0) //shouldn't happen unless our scale is wacky
            return;
        cellsPerPixel = (PixSize / cellsize);
        //following added by mdp 5/21/2004 to make more degradation occur from way far away.
        scale = pose->getScale();
        if (scale <= 0)
            scale = 1.0;
        heightAboveHighestPoint = (winPose->getPosition()[Z] - (bbox.t + pose->getPosition()[Z]) * scale) * winPose->getScale();
        if (heightAboveHighestPoint > cellsize)
                cellsPerPixel *= heightAboveHighestPoint / cellsize;
        if (cellsPerPixel < 1.0) //added by mdp 3/18/2008 to get GeoCoastPilot moving a bit faster
            cellsPerPixel = sqrt(cellsPerPixel);

        innerStep = (int) ((2 + 18 * Scene3D::interactivityHint * degradation) * cellsPerPixel) + 1;

        outerStep = (int) (2.0 + 16 * degradation) - innerStep * 2;
        if (outerStep <= 2 * innerStep)
            outerStep = (int) ((innerStep) * (2.0 + 6.5f * degradation));

        //currentView = mainWindow->getViewInfo(); //^^^hack to use overview window to see how GUTM is clipped in MainWindow
        renderingBounds(degradation);
        if (generateBoundingLines(FrontBounds))
            renderWithinBounds(FrontBounds, innerStep);
        if (generateBoundingLines(BackBounds))
        {
            if (minyBound < maxyBound - 1.5 * outerStep)
                renderWithinBounds(BackBounds, outerStep);
        }
        currentView = NULL;
    }

doneWithGutmDrawLowRes:
    ;
    /*---&&&example code for tracking cursor in rendering...---
    //Draw a box around the cell the cursor is at.
    const GLdouble *pos;
    static Point3f coord = {0.0, 0.0, 0.0};
    pos = win->getCursorPosOnSurfaceOrPlane(coord[Z]);
    if (pos && (GetHeightAt(pos[X], pos[Y], coord) != invalidHeight))
    {
        glNormal3f(0.0, 0.0, 1.0);
        glColor4f(1.0, 1.0, 0.5, 0.5);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUAD_STRIP);
            glVertex3f(coord[X], coord[Y], coord[Z] - cellsize);
            glVertex3f(coord[X], coord[Y], coord[Z] + cellsize);
            glVertex3f(coord[X] + cellsize, coord[Y], coord[Z] - cellsize);
            glVertex3f(coord[X] + cellsize, coord[Y], coord[Z] + cellsize);
            glVertex3f(coord[X] + cellsize, coord[Y] + cellsize, coord[Z] - cellsize);
            glVertex3f(coord[X] + cellsize, coord[Y] + cellsize, coord[Z] + cellsize);
            glVertex3f(coord[X], coord[Y] + cellsize, coord[Z] - cellsize);
            glVertex3f(coord[X], coord[Y] + cellsize, coord[Z] + cellsize);
            glVertex3f(coord[X], coord[Y], coord[Z] - cellsize);
            glVertex3f(coord[X], coord[Y], coord[Z] + cellsize);
        glEnd();
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_ONE, GL_ZERO);
    }*/
    if (rubberBand && !renderingForPicking)
        rubberBand->DrawLowRes(degradation, win);
}

/*---------------------------------------------------------------------------*/
void Gutm::DrawUsingHighResZone(float degradation, Window3D *win)
{
    int i;
    int HiResCx, HiResCy;  // center of hi-res spot
    float PolySize; // the size of a hi-res polygon in pixels
    float PixPerGrid; // the number of pixels per grid cell
    float innerStride;
    int innerStep, outerStep;  // inner and outer step size
    int offsetForStep;
    int halfSteps;  // the size of the high res grid in steps
    int hxl,hxr, hyb, hyt;  // left right bottom top of high res
    int lxl,lyb;  // left, bottom of low res
    gz4d::Point<double> centerPoint; //center of the widgets/workspace
    gz4d::Point<double> offset = pose->getPosition();
    float PixSize;

    currentView = win->getViewInfo();
    PixSize = (float)currentView->getPixelSize();
    PixPerGrid = cellsize / PixSize;

    centerPoint = currentView->getReferencePoint();

    PolySize = 1.0f + degradation*(maxx+maxy)/800.0f;
    innerStride = PolySize/PixPerGrid;
    //if (innerStep < 1) innerStep = 1;
    halfSteps = (int)(highResSize/(PolySize));

    //cerr << PixSize << "  " << innerStep << "  ";
    //cerr << PolySize << "  " << halfSteps << "\n";

    //halfSteps -= halfSteps%LOW_TO_HIGH_RES_RATIO; // was 4 cw

    halfSteps = (int)(halfSteps*innerStride);                 // determine size of region in halfsteps
    offsetForStep = halfSteps % LOW_TO_HIGH_RES_RATIO; // remember how much we're subtracting
    halfSteps -= offsetForStep;                           // Size up to be a multiple of the RATIO
    //Don't let it get too small!
    while (halfSteps < 2 * LOW_TO_HIGH_RES_RATIO)
        halfSteps += LOW_TO_HIGH_RES_RATIO;

    //Convert to an integer for loops below
    innerStep = (int)innerStride;
    if (innerStep < 1) innerStep = 1;
    //Define the step to use for the low res portion.
    outerStep = innerStep*LOW_TO_HIGH_RES_RATIO;

    if(outerStep < 0)

    {

        outerStep = maxx;

    }

    HiResCx = (int)((centerPoint[0] - offset[0])/cellsize);
    HiResCx = HiResCx - (HiResCx % outerStep) + (outerStep / 2);
    HiResCy = (int)((centerPoint[1] - offset[1])/cellsize);
    HiResCy = HiResCy - (HiResCy % outerStep) + (outerStep / 2);

    //Specify the region, using the offset to match up the center
    hxl = (HiResCx - halfSteps) + offsetForStep;
    hxr = (HiResCx + halfSteps) + offsetForStep;
    hyb = (HiResCy - halfSteps) + offsetForStep;
    hyt = (HiResCy + halfSteps) + offsetForStep;

    // clip to data boundaries
    if (hxl < 0) hxl = 0;
    if (hxl > maxx) hxl = maxx;

    if (hxr > maxx) hxr = maxx-1;
    if (hxr < 0) hxr = 0;

    if (hyb < 0) hyb = 0;
    if (hyb > maxy) hyb = maxy;


    if (hyt > maxy) hyt = maxy;
    if (hyt < 0) hyt = 0;

    // inner hi-res zone
    for(i = hyb; i < hyt; i += innerStep)
    {
    DrawShadedStrip(i,hxl,hxr+1,innerStep);
    }

    //outer low-res zone
    lxl = hxl % outerStep;
    lyb = hyb % outerStep;

    // bottom
    for(i = lyb; i < hyb; i+= outerStep)
    {
        DrawShadedStrip(i,lxl,maxx,outerStep);
    }
    // top
    for(i = hyt; i < maxy-outerStep-1; i+= outerStep)
    {
        DrawShadedStrip(i,lxl,maxx,outerStep);
    }
    // left
    for(i = hyb; i < hyt; i+= outerStep)
    {
        DrawShadedStrip(i,hxr,maxx,outerStep);
    }
    // right
    for(i = hyb; i < hyt; i+= outerStep)
    {
        DrawShadedStrip(i,lxl,hxl+1,outerStep);
    }
    currentView = NULL;
}

/*---------------------------------------------------------------------------*/
void Gutm::DrawEverything(int eye, Window3D *win)
{
    int currentRow, last; //mdp 3/30/2000

#ifdef GENERATE_GDT
    if (!tempFile)
        tempFile = fopen("gutm.gdt", "w");
#elif defined(GENERATE_FULLGRID)
    if (!tempFile)
        tempFile = fopen("gutm.gutm", "w");
#endif

    //draw several rows at once (mdp 3/30/2000)
    last = drawrow + HIGH_RES_ROW_STEP;
    if (last > maxy - 1)
    {   last = maxy - 1;
    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
        doneFlag = 1;
#if defined(GENERATE_GDT) || defined(GENERATE_FULLGRID)
    if (tempFile && !fileDone)
    {
        fclose(tempFile);
        fileDone = 1;
    }
#endif
    }
    for (currentRow = drawrow; currentRow < last; currentRow++)
        DrawShadedStrip(currentRow,0,maxx,1);
    if (eye == dominantEye || eye == MONO_EYE) //update our idea of the current row unless we're on the first of two eyes.
    {
        drawrow = last;
        if (last >= maxy) //go ahead and set the doneFlag if this will be our last time through
            doneFlag = 1;
    }
}

/*---------------------------------------------------------------------------*/
int Gutm::DrawHighRes(int eye, Window3D *win)
{
    int start, last;
    float heading = 0.0;

    if (visible == 1 && !doneFlag && dem != NULL)
    {
        glShadeModel(GL_SMOOTH);

        currentView = win->getViewInfo();
        if (useHighResZone == GL_TRUE)
            DrawEverything(eye, win);
        else
        {
            heading = currentView->getPosable()->getOrientationAxis()[Yaw];
            if (heading < 0)
                heading = -heading;
            renderingBounds(0.0);
            if (!generateBoundingLines(OuterBounds))
            {
                doneFlag = 1;
                return doneFlag;
            }
            if (drawrow <= 0)
                drewBox = GL_FALSE;
            if (heading > 90 && heading < 270) //backwards
            {
                if (drawrow <= 0)
                {
                    if (maxyBound < maxy)
                        start = maxyBound;
                    else
                        start = maxy;
                }
                else
                    start = drawrow;
                start -= HIGH_RES_ROW_STEP;
            }
            else
            {
                if (drawrow <= 0 && minyBound > 0)
                    start = minyBound;
                else
                    start = drawrow;
            }
            last = start + HIGH_RES_ROW_STEP;
            renderWithinBounds(OuterBounds, 1, start, last);

            if (eye == dominantEye || eye == MONO_EYE) //update our idea of the current row unless we're on the first of two eyes.
            {
                if (heading > 90 && heading < 270) //backwards
                {
                    drawrow = start;
                    if (start <= 0)
                        doneFlag = 1;
                }
                else
                {
                    drawrow = last;
                    if (last >= maxy) //go ahead and set the doneFlag if this will be our last time through
                        doneFlag = 1;
                }
            }
            currentView = NULL;
        }
        if (rubberBand)
            rubberBand->DrawLowRes(0.0, win);
    }
    else
        doneFlag = 1;

    return doneFlag;
}

/*---------------------------------------------------------------------------*/
void Gutm::renderWithinBounds(int boundsType, int step, int earliestY, int latestY)
{
    int i;
    int startx, starty, stopx, stopy, leftStop, rightStop;
    int firstx1, lastx1, firstx2, lastx2, temp;
    int boxStartx = 0, boxStopx = 0, boxStarty = 0, boxStopy = 0;
    gz4d::Point<double> viewPosition, offset;
    int modifiedLeftStop, modifiedRightStop;

    if (latestY < 0 || latestY > maxy)
        latestY = maxy;

    if (earliestY < 0)
        earliestY = 0;

    //set up the bounds
    bounds = allBounds[boundsType];
    starty = (int) ((minyBound < earliestY) ? earliestY : minyBound);
    stopy = (int) ((maxyBound >= latestY) ? latestY - 1 : maxyBound);
    if (stopy < 0)
        stopy = 0;
    if (step > 1)
    {
        starty -= starty % step;
        stopy -= stopy % step;
    }
    leftStop = (int) bounds[leftBoundHighIndex][Y];
    rightStop = (int) bounds[rightBoundHighIndex][Y];
    if (leftInvSlope < 40 && leftInvSlope > -40) //watch out for near-horizontal lines
        firstx1 = (int) (starty * leftInvSlope + leftInvIntercept);
    else
        firstx1 = (int) bounds[leftBoundHighIndex][X];
    if (rightInvSlope < 40 && rightInvSlope > -40) //watch out for near-horizontal lines
        lastx1 = (int) (starty * rightInvSlope + rightInvIntercept) + step;
    else
        lastx1 = (int) bounds[rightBoundLowIndex][X] + step;
    startx = firstx1;
    if (startx < 0)
        startx = 0;
    stopx = lastx1;
    if (stopx > maxx)
        stopx = maxx;

    //ensure we always get the area around the center of workspace
    if (boundsType != BackBounds) //only do for FrontBounds and OuterBounds
    {
        viewPosition = currentView->getPosable()->getPosition();
        offset = pose->getPosition();
        boxStartx = (int) ((viewPosition[X] - offset[X]) / cellsize) - 3;
        boxStopx = boxStartx + 7;
        boxStarty = (int) ((viewPosition[Y] - offset[Y]) / cellsize) - 3;
        boxStopy = boxStarty + 6;
        if (boxStartx < 0)
            boxStartx = 0;
        if (boxStopx > maxx)
            boxStopx = maxx;
        if (boxStarty < 0)
            boxStarty = 0;
        if (boxStopy > maxy)
            boxStopy = maxy;

        //draw portions near center that are somehow not covered by our normal method
        if (!drewBox)
        {
            if (starty < boxStopy)
            {
                for(i = boxStarty; i < starty; i++)
                {
                    DrawShadedStrip(i, boxStartx, boxStopx, 1);
                }
                drewBox = GL_TRUE;
            }
            if (stopy > boxStarty)
            {
                for(i = stopy; i < boxStopy; i++)
                {
                    DrawShadedStrip(i, boxStartx, boxStopx, 1);
                }
                drewBox = GL_TRUE;
            }
            if (!drewBox)
            {
                for(i = boxStarty; i < boxStopy; i++)
                {
                    DrawShadedStrip(i, boxStartx, boxStopx, 1);
                }
                drewBox = GL_TRUE;
            }
        }
    }

    //draw within the bounds
    for(i = starty; i <= stopy; i += step)
    {
        modifiedLeftStop = modifiedRightStop = 0; //these are for adding an extra row to the changeover
        while (i > leftStop)
        {
            leftBoundHighIndex = leftBoundHighIndex + leftDirection;
            leftBoundLowIndex = leftBoundLowIndex + leftDirection;
            leftBoundHighIndex = NEAR_MOD(leftBoundHighIndex, 4);
            leftBoundLowIndex = NEAR_MOD(leftBoundLowIndex, 4);
            if (bounds[leftBoundHighIndex][Y] <= leftStop) //should never happen?
                break;
            calcBoundingLine(leftBoundLowIndex, leftBoundHighIndex, leftInvSlope, leftInvIntercept);
            leftStop = (int) bounds[leftBoundHighIndex][Y];
            modifiedLeftStop = 1;
        }

        while (i > rightStop)
        {
            if (rightBoundHighIndex == leftBoundHighIndex)
                break;
            rightBoundHighIndex = rightBoundHighIndex + rightDirection;
            rightBoundLowIndex = rightBoundLowIndex + rightDirection;
            rightBoundHighIndex = NEAR_MOD(rightBoundHighIndex, 4);
            rightBoundLowIndex = NEAR_MOD(rightBoundLowIndex, 4);
            if (bounds[rightBoundHighIndex][Y] <= rightStop) //should never happen?
                break;
            calcBoundingLine(rightBoundLowIndex, rightBoundHighIndex, rightInvSlope, rightInvIntercept);
            rightStop = (int) bounds[rightBoundHighIndex][Y];
            modifiedRightStop = 0;
        }

        firstx2 = firstx1;
        lastx2 = lastx1;
        if (leftInvSlope < 40 && leftInvSlope > -40) //watch out for near-horizontal lines
            firstx1 = (int) (i * leftInvSlope + leftInvIntercept) - modifiedLeftStop;
        else
            firstx1 = (int) bounds[leftBoundHighIndex][X];
        if (rightInvSlope < 40 && rightInvSlope > -40) //watch out for near-horizontal lines
            lastx1 = (int) (i * rightInvSlope + rightInvIntercept) + step + modifiedRightStop;
        else
        {
            lastx1 = (int)bounds[rightBoundLowIndex][X];
            if (lastx1 < bounds[rightBoundHighIndex][X])
                lastx1 = (int)bounds[rightBoundHighIndex][X];
            lastx1++;
        }
        startx = (firstx1 < firstx2) ? firstx1 : firstx2;
        stopx = ((lastx1 > lastx2) ? lastx1 : lastx2) + step;
        if (step > 1)
        {
            startx -= startx % step;
            stopx -= stopx % step;
        }
        if (startx > stopx) //I wish we didn't have to do this!
        {
            temp = startx;
            startx = stopx;
            stopx = temp;
        }
        startx = (int) ((startx < 0) ? 0 : startx);
        stopx = (int) ((stopx > maxx) ? maxx : stopx);
        if (boundsType != BackBounds) //only do for FrontBounds and OuterBounds
        {
            if (i >= boxStarty && i < boxStopy)
            {
                if (startx > boxStartx)
                    startx = boxStartx;
                if (stopx < boxStopx)
                    stopx = boxStopx;
            }
        }
        if (i > 0)
            DrawShadedStrip(i - step, startx, stopx, step); //draw back a line so we're sure to cover our lines!
    }
    if (i - step < maxy)
        DrawShadedStrip(i - step, startx, stopx, step); //draw back a line so we're sure to cover our lines!
    if (step == 1 && i < maxy)
        DrawShadedStrip(i, startx, stopx, step); //draw last line so we're sure to cover our lines!
}

/*---------------------------------------------------------------------------*/
/** Calculates the effective intersection of the viewing frustum in the
*   currentView with this GUTM.  Results are stored in the Bounds variables.
*/
void Gutm::renderingBounds(float degradation)
{
    GLfloat zi, zf, zG, ze, zRatio, xDelta, yDelta, maxz;
    gz4d::Point<double> p1, p2, q1, q2;
    GLfloat pt, qt, distance, factor, xval, yval;
    GLfloat pitch, absPitch, frontLength;
    gz4d::Point<double> pos;
    GLfloat scale;
    GLboolean dropFar = GL_FALSE;
    int i, b;


    bounds = allBounds[OuterBounds];
    dropNear = GL_TRUE;

    //---Determine p and q---
    zf = (float) currentView->getCorner(ViewManager::BACK_TOP_LEFT)[Z];
    zi = (float) currentView->getCorner(ViewManager::BACK_BOTTOM_LEFT)[Z];
    if (currentView->getEyePoint().IsValid())
        ze = (float) currentView->getEyePoint()[Z];
    else
    {
        std::cerr << "Failed eyepoint determination.\nScale might be zero!" << std::endl;
        return;
    }
    scale = pose->getScale();
    if (scale == 0.0f)
        scale = 1.0f;
    zG = (pose->getPosition()[Z] + bbox.b) * scale; //minheight(bbox.b) added by mdp 7/14/2004 to make it smarter about things way off the bottom
    pitch = currentView->getPosable()->getOrientationAxis()[Pitch];
    if (zG > zf) // ground plane cuts above backplane
    {
        if (zG > ze) //ground plane is above frustum!
            return;

        p1 = currentView->getCorner(ViewManager::FRONT_TOP_LEFT);
        p2 = currentView->getCorner(ViewManager::BACK_TOP_LEFT);
        q1 = currentView->getCorner(ViewManager::FRONT_TOP_RIGHT);
        q2 = currentView->getCorner(ViewManager::BACK_TOP_RIGHT);
    }
    else if (zG < zi) //ground plane cuts below backplane
    {
        maxz = zG + bbox.t * scale;
        if (maxz < ze && maxz < zi) //GUTM is below frustum
            return;

        if (zG < ze && zG < zi) //baseplane is below frustum
        {
            dropFar = GL_TRUE;
            p1 = currentView->getCorner(ViewManager::BACK_TOP_LEFT);
            q1 = currentView->getCorner(ViewManager::BACK_TOP_RIGHT);
        }
        else
        {
            p1 = currentView->getCorner(ViewManager::FRONT_BOTTOM_LEFT);
            q1 = currentView->getCorner(ViewManager::FRONT_BOTTOM_RIGHT);
        }
        p2 = currentView->getCorner(ViewManager::BACK_BOTTOM_LEFT);
        q2 = currentView->getCorner(ViewManager::BACK_BOTTOM_RIGHT);
    }
    else //ground plane cuts through backplane
    {
        p1 = currentView->getCorner(ViewManager::BACK_TOP_LEFT);
        p2 = currentView->getCorner(ViewManager::BACK_BOTTOM_LEFT);
        q1 = currentView->getCorner(ViewManager::BACK_TOP_RIGHT);
        q2 = currentView->getCorner(ViewManager::BACK_BOTTOM_RIGHT);
    }
    if (dropFar)
    {
        //drop the far clipping plane down to get coords.
        if (pitch > 0)
        {
            bounds[pBound][X] = (float) p1[X];
            bounds[pBound][Y] = (float) p1[Y];
            bounds[qBound][X] = (float) q1[X];
            bounds[qBound][Y] = (float) q1[Y];
        }
        else
        {
            bounds[pBound][X] = (float) p2[X];
            bounds[pBound][Y] = (float) p2[Y];
            bounds[qBound][X] = (float) q2[X];
            bounds[qBound][Y] = (float) q2[Y];
        }
    }
    else
    {
        //find the intersection of lines with GUTM base-plane
        pt = (float) ((zG - p1[Z]) / (p2[Z] - p1[Z]));
        qt = (float) ((zG - q1[Z]) / (q2[Z] - q1[Z]));
        bounds[pBound][X] = (float) ((p2[X] - p1[X]) * pt + p1[X]);
        bounds[pBound][Y] = (float) ((p2[Y] - p1[Y]) * pt + p1[Y]);
        bounds[qBound][X] = (float) ((q2[X] - q1[X]) * qt + q1[X]);
        bounds[qBound][Y] = (float) ((q2[Y] - q1[Y]) * qt + q1[Y]);
    }

    //---Determine r and s---
    if (pitch >= 0 && currentView->getZComponentOfUnderPlane() < 0)
    {
        p1 = currentView->getCorner(ViewManager::FRONT_BOTTOM_RIGHT);
        q1 = currentView->getCorner(ViewManager::FRONT_BOTTOM_LEFT);
        if (dropFar) //do a real drop of the near-plane without any fanciness.
        {
            bounds[rBound][X] = (float) p1[X];
            bounds[rBound][Y] = (float) p1[Y];
            bounds[sBound][X] = (float) q1[X];
            bounds[sBound][Y] = (float) q1[Y];
            dropNear = GL_FALSE;
        }
    }
    else if (pitch < 0 && currentView->getZComponentOfAbovePlane() > 0)
    {
        p1 = currentView->getCorner(ViewManager::FRONT_TOP_RIGHT);
        q1 = currentView->getCorner(ViewManager::FRONT_TOP_LEFT);
    }
    else
    {
        p2 = currentView->getCorner(ViewManager::BACK_BOTTOM_RIGHT);
        q2 = currentView->getCorner(ViewManager::BACK_BOTTOM_LEFT);
        if (dropFar) //drop the other part of the far-plane
        {
            bounds[rBound][X] = (float) p2[X];
            bounds[rBound][Y] = (float) p2[Y];
            bounds[sBound][X] = (float) q2[X];
            bounds[sBound][Y] = (float) q2[Y];
        }
        else
        {
            p1 = currentView->getCorner(ViewManager::FRONT_BOTTOM_RIGHT);
            q1 = currentView->getCorner(ViewManager::FRONT_BOTTOM_LEFT);
            pt = (float) ((zG - p1[Z]) / (p2[Z] - p1[Z]));
            qt = (float) ((zG - q1[Z]) / (q2[Z] - q1[Z]));
            bounds[rBound][X] = (float) ((p2[X] - p1[X]) * pt + p1[X]);
            bounds[rBound][Y] = (float) ((p2[Y] - p1[Y]) * pt + p1[Y]);
            bounds[sBound][X] = (float) ((q2[X] - q1[X]) * qt + q1[X]);
            bounds[sBound][Y] = (float) ((q2[Y] - q1[Y]) * qt + q1[Y]);
        }
        dropNear = GL_FALSE;
    }
    if (dropNear)
    {
        zRatio = (float) ((p1[Z] - zG) / (2 * (ze - p1[Z])));
        if (zRatio < 0.0)
            zRatio = 0.0;
        xDelta = (float) (p1[X] - q1[X]) * zRatio;
        yDelta = (float) (p1[Y] - q1[Y]) * zRatio;
        bounds[rBound][X] = (float) p1[X] + xDelta;
        bounds[rBound][Y] = (float) p1[Y] + yDelta;
        bounds[sBound][X] = (float) q1[X] - xDelta;
        bounds[sBound][Y] = (float) q1[Y] - yDelta;
    }

    //---Normalize p, q, r, and s to cell coordinates---
    pos = pose->getPosition();
    //glBegin(GL_LINE_STRIP);
    for (i = pBound; i <= sBound; i++)
    {
    //  glVertex3f(bounds[i][X], bounds[i][Y], pos[Z]);
        bounds[i][X] = (bounds[i][X] - pos[X]) / cellsize;
        bounds[i][Y] = (bounds[i][Y] - pos[Y]) / cellsize;
    }
    //glEnd();

    //---Create front/back bounds by going out roughly
    //   3x distance between eye and workspace center
    for (i = pBound; i <= sBound; i++)
    {
        if (i <= qBound)
            b = BackBounds;
        else
            b = FrontBounds;
        allBounds[b][i][X] = bounds[i][X];
        allBounds[b][i][Y] = bounds[i][Y];
    }
    if (pitch >= 0)
        absPitch = pitch;
    else
        absPitch = -pitch;
    frontLength = (float) (currentView->getCenterToEyepoint() * 2.5) / cellsize + 90 - absPitch;
    frontLength *= (1.0f - (degradation * (.25f + (float) Scene3D::interactivityHint * 0.5f)));
    if (frontLength < 20 + absPitch)
        frontLength = 20 + absPitch;
    if (absPitch < 70)
        frontLength *= frontLengthFactor;

    //right side
    xDelta = bounds[qBound][X] - bounds[rBound][X];
    yDelta = bounds[qBound][Y] - bounds[rBound][Y];
    distance = (float) sqrt(xDelta * xDelta + yDelta * yDelta);
    factor = (frontLength / distance);
    if (factor < 1.0 && (dropNear || dropFar))
    {
        xval = bounds[rBound][X] + factor * (xDelta);
        yval = bounds[rBound][Y] + factor * (yDelta);
    }
    else
    {
        xval = bounds[qBound][X];
        yval = bounds[qBound][Y];
    }
    allBounds[FrontBounds][qBound][X] = allBounds[BackBounds][rBound][X] = xval;
    allBounds[FrontBounds][qBound][Y] = allBounds[BackBounds][rBound][Y] = yval;

    //left side
    xDelta = bounds[pBound][X] - bounds[sBound][X];
    yDelta = bounds[pBound][Y] - bounds[sBound][Y];
    distance = (float) sqrt(xDelta * xDelta + yDelta * yDelta);
    factor = (frontLength / distance);
    if (factor < 1.0 && (dropNear || dropFar))
    {
        xval = bounds[sBound][X] + factor * (xDelta);
        yval = bounds[sBound][Y] + factor * (yDelta);
    }
    else
    {
        xval = bounds[pBound][X];
        yval = bounds[pBound][Y];
    }
    allBounds[FrontBounds][pBound][X] = allBounds[BackBounds][sBound][X] = xval;
    allBounds[FrontBounds][pBound][Y] = allBounds[BackBounds][sBound][Y] = yval;
}

/*---------------------------------------------------------------------------*/
/** Creates the bounding lines for where the frustum potentially picks up
*   cells in the GUTM.  Returns GL_TRUE if there is a potential for rendering,
*   and GL_FALSE if nothing from this GUTM should be rendered.
*/
GLboolean Gutm::generateBoundingLines(int boundsType)
{
    int i, leastBound, minxBound, maxxBound;
    GLfloat minx;

    bounds = allBounds[boundsType];

    //---Find *yBound, *xBound from p, q, r, and s (with minx deciding ties in y)---
    maxyBound = minyBound = (int) bounds[pBound][Y];
    minxBound = maxxBound = (int) bounds[pBound][X];
    minx = bounds[pBound][X];
    leastBound = pBound;
    for (i = qBound; i <= sBound; i++)
    {
        if (minyBound >= bounds[i][Y])
        {
            if (minyBound > bounds[i][Y] || minx > bounds[i][X])
            {
                minyBound = (int) bounds[i][Y];
                minx = bounds[i][X];
                leastBound = i;
            }
        }
        if (minxBound >= bounds[i][X])
            minxBound = (int) bounds[i][X];
        if (maxyBound < bounds[i][Y])
            maxyBound = (int) bounds[i][Y];
        if (maxxBound < bounds[i][X])
            maxxBound = (int) bounds[i][X];
    }
    if (minxBound > maxx || maxxBound < 0 || minyBound > maxy || maxyBound < 0)
        return GL_FALSE;

    leftBoundHighIndex = leastBound - 1;
    leftBoundHighIndex = NEAR_MOD(leftBoundHighIndex, 4);
    rightBoundHighIndex = leastBound + 1;
    rightBoundHighIndex = NEAR_MOD(rightBoundHighIndex, 4);
    leftDirection = -1;
    rightDirection = 1;
    if (bounds[leftBoundHighIndex][X] > bounds[rightBoundHighIndex][X])
    {
        i = rightBoundHighIndex;
        rightBoundHighIndex = leftBoundHighIndex;
        leftBoundHighIndex = i;
        leftDirection = 1;
        rightDirection = -1;
    }
    leftBoundLowIndex = rightBoundLowIndex = leastBound;

    if (bounds[leftBoundLowIndex][Y] == bounds[leftBoundHighIndex][Y])
    {
        leftBoundLowIndex = leftBoundLowIndex + leftDirection;
        leftBoundHighIndex = leftBoundHighIndex + leftDirection;
        leftBoundLowIndex = NEAR_MOD(leftBoundLowIndex, 4);
        leftBoundHighIndex = NEAR_MOD(leftBoundHighIndex, 4);
    }
    calcBoundingLine(leftBoundLowIndex, leftBoundHighIndex, leftInvSlope, leftInvIntercept);

    if (bounds[rightBoundLowIndex][Y] == bounds[rightBoundHighIndex][Y])
    {
        rightBoundLowIndex = rightBoundLowIndex + rightDirection;
        rightBoundHighIndex = rightBoundHighIndex + rightDirection;
        rightBoundLowIndex = NEAR_MOD(rightBoundLowIndex, 4);
        rightBoundHighIndex = NEAR_MOD(rightBoundHighIndex, 4);
    }
    calcBoundingLine(rightBoundLowIndex, rightBoundHighIndex, rightInvSlope, rightInvIntercept);
    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/** Calculates the slope and intercept of a line of the form x = m'y + b',
*    based on the startIdx and stopIdx into the bounds array.
*/
void Gutm::calcBoundingLine(int startIdx, int stopIdx, float &invSlope, float &invIntercept)
{
    float x1, y1, x2, y2, dy;

    x1 = bounds[startIdx][X];
    y1 = bounds[startIdx][Y];
    x2 = bounds[stopIdx][X];
    y2 = bounds[stopIdx][Y];
    dy = y2 - y1;

    invSlope = (x2 - x1) / dy;
    invIntercept = (x1 * y2 - x2 * y1) / dy;
}

/*---------------------------------------------------------------------------*/
void Gutm::Fuse(Object *other)  // only works if other is of type Gutm
{
    int otherMaxx, otherMaxy,ix,iy;
    int i,j, idx;
    int startx, starty, endx, endy, doingCloseFusion;
    float otherH, val;
    float otherCellSize, maxHolesInCoverage, cellsizeRatio;
    Gutm * Other;
    gz4d::Point<double> offset, otherOffset;
    
    if(other == this)
        return;

    offset = pose->getPosition();

    if(!other || other->getRegistrationEntry()->className != "Gutm")
        return;

    Other = ((Gutm *)other);

    int otherX, otherY;

    otherCellSize = Other->cellsize;
    
    if (cellsize > otherCellSize) // cut a hole
    {
//        cerr << "cut a hole internally \n";
//        cerr << otherMaxx << "  " << otherMaxy << "\n";
    otherMaxx = Other->maxx;
    otherMaxy = Other->maxy;
    otherOffset = Other->getPosable()->getPosition();
    cellsizeRatio = cellsize / otherCellSize;
    maxHolesInCoverage = int(cellsizeRatio * cellsizeRatio);
    doingCloseFusion = ((cellsizeRatio <= FUSION_CELLSIZE_RATIO_THRESHOLD) && cellsizeRatio == (int)cellsizeRatio); //added by mdp 4/29/2005 since non-integer fusion with the new technique seems spotty. :)

    //determine bounds of overlapping rectangle in terms of other's cells
    startx = (int)((offset[X] - otherOffset[X]) / otherCellSize - cellsizeRatio);
    endx = (int)((offset[X] + maxx * cellsize - otherOffset[X]) / otherCellSize + 1);
    starty = (int)((offset[Y] - otherOffset[Y]) / otherCellSize - cellsizeRatio);
    endy = (int)((offset[Y] + maxy * cellsize - otherOffset[Y]) / otherCellSize + 1);
    if (endx < 0 || startx > otherMaxx || endy < 0 || starty > otherMaxy)
        return;
    if (startx < 0)
        startx = 0;
    if (endx > otherMaxx)
        endx = otherMaxx;
    if (starty < 0)
        starty = 0;
    if (endy > otherMaxy)
        endy = otherMaxy;
    
    //create mask
    float *mask = NULL;
    char *holesInCoverage = NULL;

    if (doingCloseFusion)
    {
        mask = new float[maxy * maxx];
        holesInCoverage = new char[maxy * maxx];
        for (i = 0; i < maxy; i++)
        {
            for (j = 0; j < maxx; j++)
            {
                idx = i * maxx + j;
                mask[idx] = 0;
                holesInCoverage[idx] = (char)maxHolesInCoverage;
            }
        }
    }

    for (i = starty; i < endy; ++i)
    {
        for (j = startx; j < endx; ++j)
        {
            otherX = (int)(otherOffset[0]);
            otherX += (int)(float(j)*otherCellSize);
            otherY = (int)(otherOffset[1]);
            otherY += (int)(float(i)*otherCellSize);

            // find the corresponding local coordinate
            ix = int (0.5 + (otherX - offset[X])/cellsize);
            iy = int (0.5 + (otherY - offset[Y])/cellsize);
            // test for ranges
            if((ix <= -1) || (iy <= -1) || (ix >= maxx) || (iy >= maxy))
                continue;

            if (dem[iy][ix] >0.0)
            {
                otherH = Other->dem[i][j];
                idx = iy * maxx + ix;
                if (otherH > 0.0) // now we fill the mask
                {
                    if (doingCloseFusion)
                    {
                        if (mask[idx] == 0 || (mask[idx] > otherH))
                            mask[idx] = otherH;
                        holesInCoverage[idx]--;
                    }
                    else
                        dem[iy][ix] = -dem[iy][ix]; //cutting a hole, don't bother with mask
//                      cerr << ix << "  " << iy << "\n";
                    }
                }
            }
        }
        if (doingCloseFusion)
        {
            for (i = 0; i < maxy; i++)
            {
                for (j = 0; j < maxx; j++)
                {
                    idx = i * maxx + j;
                    if (mask[idx] > 0)
                    {
                        val = mask[idx] + otherOffset[Z] - offset[Z];
                        if (//holesInCoverage[idx] &&
                            (((i > 0)? (holesInCoverage[idx - maxx] > 0) : 0) ||
                            ((j > 0)? (holesInCoverage[idx - 1] > 0) : 0) ||
                            ((i < maxy - 1)? (holesInCoverage[idx + maxx] > 0) : 0) ||
                            ((j < maxx - 1)? (holesInCoverage[idx + 1] > 0) : 0)))
                        { //if there are any holes in coverage nearby, don't create a hole
                            //dem[i][j] = val; //this piece of code woud cause surface to depress to conform to high-res surface
                            continue;
                        }
                        else if (holesInCoverage[idx] <= 0 && dem[i][j] > 0.0)
                            dem[i][j] = -dem[i][j];
                    }
                }
            }
            delete []mask;
            delete []holesInCoverage;
        }
    }
}

/*---------------------------------------------------------------------------*/
void Gutm::RestoreFusionHoles()
{
    int i, j;

    for (i = 0; i < maxy; i++)
    {
        for (j = 0; j < maxx; j++)
        {
            if (dem[i][j] < 0.0)
                dem[i][j] = -dem[i][j];
        }
    }
    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
void Gutm::useNewHeightExaggeration(GLfloat heightEx)
{
    Object::useNewHeightExaggeration(heightEx);
    if (dem && !normalsOnTheFly)
        CalculateNormals(heightEx);
}

/*---------------------------------------------------------------------------*/
/** This function creates a table of times for use with NavigationPath
*   instances.  This is done so that much is pre-calculated so that when we
*   get to the actual drawing, tide calculations don't take so much time.
*/
void Gutm::updatePlannedPathTimes()
{
    /*
    const double *pos;
    int i, sz;
    
    if (!navPath)
        return;

    //create the storage if it doesn't already exist.
    if (!plannedPathTime)
    {
        sz = maxx * maxy;
        plannedPathTime = new float[sz];
        for (i = 0; i < sz; i++)
        {
            plannedPathTime[i] = NAN;
        }
    }

    pos = pose->getPosition();

    if (navPath != lastNavPath)
    {
        navPathUpdateCounter = -1;
        lastNavPath = navPath;
    }
    navPath->updateTimes(plannedPathTime, pos, cellsize, maxx, maxy, navPathUpdateCounter);
    */
}

/*---------------------------------------------------------------------------*/
/** This function creates a table of time offsets and multipliers for use with
*   DynamicTides instances.  This is done so that much is pre-calculated so
*   that when we get to the actual drawing, tide calculations don't take so
*   much time.
*/
void Gutm::createTideOffsetTable()
{
    int row, col, idx;
    gz4d::Point<double> pos;
    //float zOffset;
    gz4d::Point<double> p;
    
    if (!dTides)
        return;

    lastDynamicTides = dTides;

    pos = pose->getPosition();

    //create the storage if it doesn't already exist.
    if (!tideTimeOffsets)
    {
        tideTimeOffsets = new float[maxx * maxy];
        tideMultipliers = new float[maxx * maxy];
    }

    //fill up the tables.
    p[Z] = 0.0;
    for (row = 0; row < maxy; row++)
    {
        for (col = 0; col < maxx; col++)
        {
            p[X] = float(col) * cellsize + pos[X];
            p[Y] = float(row) + pos[Y];
            idx = row * maxx + col;
            if (dem[row][col] > 0.0)
                dTides->getTimeOffsetAndMultiplier(p[X], p[Y], tideTimeOffsets[idx], tideMultipliers[idx]);
            else
                tideMultipliers[idx] = 0.0;
        }
    }
}

/*---------------------------------------------------------------------------*/
void Gutm::DrawShadedStrip(int row, int start, int end, int step)
{

    // draw a single shaded triangle strip
    // need to change to row major order
    int i, loop, idx = 0;
    vNorms *row1;
    vNorms *row2;
    float *h1;
    float *h2;
    //vNorms *currentRow;
    const GLshort *currentRow;
    float currentH, currentY, waterHeight, landHeight;
    float y1,y2,x;
    int currentIdx;
    //int start,end;
    int drawing, last;
    int hasTides = 0;
    int hasNav = 0;
    std::shared_ptr<TextureMap> currentTexture;
    gz4d::Point<double> p;
    gz4d::Point<double> pos = gz4d::Point<double>::Invalid();
    Gutm *tideGutm;
    double currentGzTime = 0.0, pathTime = 0.0;

    if(*tide || dTides)
    {
        hasTides = 1;
        pos = pose->getPosition();
        if (dTides != lastDynamicTides)
            createTideOffsetTable();
    }

    if(*navPath)
    {
        hasNav = 1;
        currentGzTime = Scene3D::timeControl->getGZTime();
//      updatePlannedPathTimes();
    }

    glColor4fv(surfaceColor);

    //clamp the row and row+step quantities, then see if they match
    last = row+step;
    if (last >= maxy)
        last = maxy - 1;
    if (row < 0)
        row = 0;
    if (row == last)
        return;

    drawing = GL_FALSE;

    row1 = norms_array[row];
    row2 = norms_array[last];

    h1 = dem[row];
    h2 = dem[last];

    if(hasTides || hasAlternateData)
    {
        EnableTexture(TEXTURE_COORD_NONE);
        if (!customtextureptr)
            currentTexture = *defaulttextureptr;
        else
            currentTexture = customtextureptr;
    }
    else
    {
        EnableTexture();
    }

    y1 = float(row)*cellsize;
    y2 = float(last)*cellsize;

    for (i = start; i < end;)
    {
        if ((h1[i] > 0.0) || (h2[i] > 0.0)) // checking for holes
        {
            if(!drawing)
            {
                glBegin(GL_TRIANGLE_STRIP);
                drawing = GL_TRUE;
            }
            x = float(i)*cellsize;

            currentIdx = row;
            currentH = h1[i];
            //currentRow = &row1[i];
            currentRow = row1[i];
            currentY = y1;
            loop = 2;
            while (loop--)
            {
                if(currentH > 0.0)
                {
/*                    if (hasAlternateData)
                    {
                        glColor4ubv(*GetColorAtValue(alternateData[currentIdx][i]));
                        //gzlog << currentIdx << ", " << i << " altData: " << alternateData[currentIdx][i] << endl;
                    }*/
                    if (normalsOnTheFly)
                        //glNormal3f(currentRow->nx, currentRow->ny, currentRow->nz / (currentView->getHeightExaggeration() * shadeExaggeration));
                        glNormal3f(currentRow[X], currentRow[Y], currentRow[Z] / (currentView->getHeightExaggeration() * shadeExaggeration));
                    else
                        //glNormal3s(currentRow->nx, currentRow->ny, currentRow->nz);
                        glNormal3sv(currentRow);
                    if(hasTides)
                    {
                        if (dTides)
                            idx = currentIdx * maxx + i;
                        if (!dTides || hasNav)
                        {
                            p[Z] = 0.0;
                            p[X] = x + pos[X];
                            p[Y] = currentY + pos[Y];
                        }

                        if (hasNav)
                            pathTime = (*navPath)->getTimeAt(p);
                        if (dTides)
                        {
                            if (!tideMultipliers[idx])
                                waterHeight = 0.0;
                            else
                            {
                                if (hasNav && pathTime > currentGzTime)
                                    waterHeight = dTides->getTideOffsetGiven(tideTimeOffsets[idx], tideMultipliers[idx], pathTime);
                                else
                                    waterHeight = dTides->getTideOffsetGiven(tideTimeOffsets[idx], tideMultipliers[idx]);
                            }
                        }
                        else
                        {
                            if (hasNav && pathTime > currentGzTime)
                                tideGutm = (Gutm *)(*tide)->GetOffsetFromCurrent(pathTime);
//                              tideGutm = (Gutm *)(*tide)->GetOffsetFromCurrent((*navPath)->getTimeOffset(p));
                            else
                                tideGutm = (Gutm *)(*tide)->GetCurrent();
                            if(tideGutm)
                                waterHeight = tideGutm->GetValue(p[X], p[Y]);
                            else
                                waterHeight = 0.0;
                        }
                        landHeight = pos[Z] + currentH;
                        if (landHeight < waterHeight)
                            currentTexture->SetHeight(landHeight - waterHeight);
                        else
                            currentTexture->SetHeight(landHeight);
                    }
                    else if(hasAlternateData)
                    {
                        currentTexture->SetHeight(alternateData[currentIdx][i]);
                    }
                    glVertex3f(x, currentY, currentH);
                }
                if (loop)
                {
                    currentIdx = last;
                    currentH = h2[i];
                    //currentRow = &row2[i];
                    currentRow = row2[i];
                    currentY = y2;
                }
            }
        }
        else
        {
            glEnd();
            drawing = GL_FALSE;
        }
        
        if(i+step <end-1)
        {
            i+=step;
        }
        else if(i<end-1)
        {
            i=end-1;
        }
        else
        i=end;
    }

    if (drawing)
        glEnd();

    
    DisableTexture();
    
}

/*--------------------------------------------------------------------------*/
/**    This function resets this object to start over drawing high res the next
*    time DrawHighRes is called.
*/
void Gutm::ResetHighRes()
{
doneFlag = drawrow = 0;
}

/*---------------------------------------------------------------------------*/
/**    Display info about this object in the info window on the GUI.
*/
void Gutm::ShowInfo(GLuint *names, int numNames)
{
    char infoLine[512];
    Scene3D *scene;
    std::shared_ptr<TextureMap> texture;
    double area;

    scene = mainWindow->getScene3D();

    sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className.c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "Name:\t%s\n", GetLabel().c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "File:\t%s\n", (GetFilename().empty())? "<none>" : GetFilename().c_str());
    scene->appendInfo(infoLine);
    sprintf(infoLine, "GeoRef:\t%f, %f, %f\n", GetPosition(X), GetPosition(Y), GetPosition(Z));
    scene->appendInfo(infoLine);

    sprintf(infoLine, "Row x Col:  %i x %i\n", maxy, maxx);
    scene->appendInfo(infoLine);

    sprintf(infoLine, "Resolution: %.2f m\n", cellsize);
    scene->appendInfo(infoLine);

    area = ((maxy - 1) * (maxx - 1) * cellsize * cellsize) / 1000000.0;    // sq.km
    sprintf(infoLine, "Area:\t%.2f sq.km\n", area);
    scene->appendInfo(infoLine);

    texture = GetCustomTexture();
    sprintf(infoLine, "Texture:%s\n", (texture)? GetCustomTexture()->GetLabel().c_str() : "<default>");
    scene->appendInfo(infoLine);
    scene->appendInfo("--------------------\n");
}

/*---------------------------------------------------------------------------*/
int Gutm::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    const char *message = argv[0];
    char retval[128];

    if (argc >= 1)
    {
        if (!strcmp(message, "fuse"))
        {
            mainWindow->getScene3D()->Fuse(this);
                        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
            return TCL_OK;
        }
        if (!strcmp(message, "restore"))
        {
            RestoreFusionHoles();
            return TCL_OK;
        }
        if (!strcmp(message, "getat"))
        {
            if (argc < 3)
            {
                Tcl_AppendResult(interp, "usage: getat <x> <y>", NULL);
                return TCL_ERROR;
            }
            float x, y;
            x = (float)atof(argv[1]);
            y = (float)atof(argv[2]);
            if (argc == 4 && !strcmp(argv[3], "data")) //&&& hack for Shep!
            {
                Point3d coord;
                //find nearest xyz
                GetHeightAt(x, y, coord, GL_TRUE);
                //print it
                sprintf(retval, "%f %f %f", coord[X], coord[Y], coord[Z]);
            }
            else
                sprintf(retval, "%f", GetHeightAt(x, y));
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "highres"))
        {
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "usage: highres {on | <size> | off | hybrid <size> | extendZone <factor>", NULL);
                return TCL_ERROR;
            }
            if (!strcmp(argv[1], "off"))
                useHighResZone = GL_FALSE;
            else if (!strcmp(argv[1], "extendZone"))
            {
                if (argc >= 3)
                    frontLengthFactor = (float)atof(argv[2]);
                sprintf(retval, "%f", frontLengthFactor);
                Tcl_SetResult(interp, retval, TCL_VOLATILE);
            }
            else
            {
                if (strcmp(argv[1], "hybrid") == 0)
                {
                    if (argc >= 3)
                        highResSize = atoi(argv[2]);
                    useHighResZone = HYBRID_HIGHRES;
                }
                else
                {
                    if (strcmp(argv[1], "on") != 0)
                        highResSize = atoi(argv[1]);
                    useHighResZone = GL_TRUE;
                }
            }
            return TCL_OK;
        }
        if (!strcmp(message, "normals"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "precomputed"))
                {
                    normalsOnTheFly = GL_FALSE;
                    return TCL_OK;
                }
                if (!strcmp(argv[1], "onTheFly"))
                {
                    normalsOnTheFly = GL_TRUE;
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "Usage: normals {precomputed | onTheFly}", NULL);
            return TCL_ERROR;
        }
        if (!strcmp(message, "texture"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "ensureBuilt"))
                {
                    std::shared_ptr<TextureMap> currentTexture;

                    if (!customtextureptr)
                        currentTexture = *defaulttextureptr;
                    else
                        currentTexture = customtextureptr;
                    
                    currentTexture->EnsureBuilt();
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "Usage: texture ensureBuilt", NULL);
            return TCL_ERROR;
        }
        if (!strcmp(message, "tides"))
        {
            if (argc >= 2)
            {
                Addressable *address;
                Object *ob;
                int ret;

                if (!argv[1] || argv[1][0] == '\0' || (argv[1][0] == '{' && argv[1][0] == '}'))
                    dTides = NULL;
                else
                {
                    ret = gzCommandInterface::findObject(argv[1], address, ob);
                    if (ret != TCL_OK)
                        return ret;
                    DYNAMIC_CAST(dTides, DynamicTides *, ob);
                    if (!dTides)
                    {
                        Tcl_AppendResult(interp, argv[1], " is not of type DynamicTides.  Discarding.", NULL);
                        return TCL_ERROR;
                    }
                }
            }
            if (dTides)
                Tcl_SetResult(interp, (char *) dTides->getAddress()->getAddress().c_str(), TCL_VOLATILE);
            else
                Tcl_SetResult(interp, const_cast<char*>("{}"), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "shadeExaggeration"))
        {
            if (argc >= 2)
            {
                shadeExaggeration = (float)atof(argv[1]);
                if (!normalsOnTheFly)
                    CalculateNormals(lastHeightExaggeration);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            sprintf(retval, "%f", shadeExaggeration);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "copy"))
        {
            int startx = 0, starty = 0, endx = -1, endy = -1;
            Gutm *newg;

            if (argc >= 2)
                startx = atoi(argv[1]);
            if (argc >= 3)
                starty = atoi(argv[2]);
            if (argc >= 4)
                endx = atoi(argv[3]);
            if (argc >= 5)
                endy = atoi(argv[4]);
            newg = createCopy(startx, starty, endx, endy);
            if (newg)
                Tcl_SetResult(interp, (char *) newg->getPosable()->getAddress()->getAddress().c_str(), TCL_VOLATILE);
            else
                Tcl_SetResult(interp, const_cast<char*>("{}"), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "rubberband"))
        {
            gutmRubberband->attachToGutm(this);
            return TCL_OK;
        }
        if (!strcmp(message, "baseColor"))
        {
            if (argc >= 2)
                surfaceColor[0] = (float)atof(argv[1]);
            if (argc >= 3)
                surfaceColor[1] = (float)atof(argv[2]);
            if (argc >= 4)
                surfaceColor[2] = (float)atof(argv[3]);
            if (argc >= 5)
                surfaceColor[3] = (float)atof(argv[4]);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            sprintf(retval, "%f %f %f %f", surfaceColor[0], surfaceColor[1], surfaceColor[2], surfaceColor[3]);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
    }
    return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Gutm::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "fuse restore highres getat normals texture tides shadeExaggeration copy rubberband baseColor ", NULL);
    return Object::list(interp);
}

/*---------------------------------------------------------------------------*/
/* This function opens this object's file and reads the information in it.
*/
int Gutm::Load()
{
#ifdef USE_BAG_FILES
    if (isBag)
    {
        if (!LoadBag())
            return 1;

        registerLabel();
        return 0;
    }
#endif
    return Object::Load();
}

/*--------------------------------------------------------------------------*/
int Gutm::processField(int option, char **fields)
{
    int i = 1;
    int numArgs;

    switch(option)
    {
        case PROJECT:
            std::cerr << "PROJECT no longer supported, use SPATIAL_REFERENCE" << std::endl;
            return 1;
/*            numArgs = getNumFields();
            if (numArgs < 3)
            {
                cerr << "Wrong format for PROJECT line.  Should be\n"
                        << "PROJECT <projection> <conversion-method> <args...>\n";
                return 1;
            }
            retval = Object::processField(option, fields);
            if (retval > 1)
                return retval;
            i++; //skip projection argument
            if (strcmp(fields[i], "CENTER") == 0)
                i += 4; //skip CENTER clause.
            address = ConversionMethodNeighborhood->findItem(fields[i]);
            if (!address)
                address = rootAddress->findItem(fields[i]);
            if (!address)
            {
                cerr << "Cannot find conversion method " << fields[i] << endl;
                return 1;
            }
            DYNAMIC_CAST(converter, ConversionMethod*, address);
            i++; //skip conversion argument
            for (; i < numArgs; i++)
                converter->addArgument(fields[i]);
            //go ahead and read rest of file
            if (maxy > 0)
            {
                retval = AllocateGrid();
                if (retval)
                    return retval;
            }
            convGrid = new ConversionGrid;
            convGrid->ShareWith(this);
            retval = convGrid->Load();
            convGrid->BreakShare();
            if (retval)
                return retval;
            cerr << "Converting to GUTM..." << endl;
            convGrid->updateTitle("Projecting...");
            converter->convert(this, convGrid, proj);
            convGrid->killProgress();
            shape = FULLGRID;
            delete convGrid;
            return -1; //header done
            break;*/
        case COLS_ROWS:
            maxx = ConvertInt(i);
            maxy = ConvertInt(i);
            break;
        case CELLSIZE:
            cellsize = ConvertFloat(i);
            break;
        case SHADE_EXAGGERATION:
            shadeExaggeration = ConvertFloat(i);
            break;
        case SHAPE:
            shape = Lookup(fields[i++]);
            break;
        case USE_ALTERNATE_DATA:
            hasAlternateData = true;
            numArgs = getNumFields();
            if (numArgs >= 3)
            {
                if (Lookup(fields[i++]) == OFFSET)
                    alternateDataOffset = ConvertFloat(i);
            }
            break;
        case BINARY:
            binary = 1;
            break;
        default:
            return Object::processField(option, fields);
            break;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
// documented in Object
int Gutm::readFrom(FILE *infile)
{
    float height;
    int i,j, retval;
    float minheight = 0.0, maxheight = 0.0;
//  double s = 0.0;
//  double e = 0.0;

    //parse header
    if (Object::readFrom(infile))
        return 1;

    std::cout << "loading..." << endl;
    
    createProgress(); //don't put it up until we know we're going to get here.
    if (!norms_array) //must not have been CONVERTed
    {
        retval = AllocateGrid();
        if (retval)
            return retval;

        minheight = 100000000.0;
        maxheight = -100000000.0;

    // timing stuff
//  mainWindow->getScene3D()->timeControl.updateCurrentTime();
//  s=mainWindow->getScene3D()->timeControl.getSysTime();
//  cerr << "Start: " << s << endl;

        long filePos;

        if(binary)
        {
            filePos = ftell(in);
            fclose(in);
            in = fopen(GetFilename().c_str(), "rb");
            fseek(in,filePos,SEEK_SET);
        }

        
        if (shape == FULLGRID)
        {
            for (i = 0; i < maxy; i++)
            {
                if(binary)
                {
                    if(fread(dem[i],sizeof(float),maxx,in) == (unsigned int)(maxx))
                    {
                        for (j = 0; j < maxx; j++)
                        {
                            height = dem[i][j];
                            if (height < 0)
                                height = -height; //want any holes to be properly accounted for when later restored.
                            if (height > 0)
                            {
                                if (height > maxheight) maxheight = height;
                                if (height < minheight) minheight = height;
                            }
                        }
                    }
                }
                else
                {
                    for (j = 0; j < maxx; j++)
                    {
                        if (fscanf(in, "%f", &height) == 1)
                        {
                            dem[i][j] = height;
                            if (height < 0)
                                height = -height; //want any holes to be properly accounted for when later restored.
                            if (height > 0)
                            {
                                if (height > maxheight) maxheight = height;
                                if (height < minheight) minheight = height;
                            }
                        }
                    }
                }
                updateProgress(i*80/maxy);
            }
        }
        else
        {
            for (i = 0; i < maxy; i++)
            {
                if(binary)
                {
                    if(fread(&startline[i],sizeof(int),1,in) == 1 && fread(&endline[i],sizeof(int),1,in) == 1 && fread(&dem[i][startline[i]],sizeof(float),endline[i]-startline[i],in) == (unsigned int)(endline[i]-startline[i]))
                    {
                        for (j = startline[i]; j < endline[i]; j++)
                        {
                            height = dem[i][j];
                            if (height < 0)
                                height = -height; //want any holes to be properly accounted for when later restored.
                            if (height > 0)
                            {
                                if (height > maxheight) maxheight = height;
                                if (height < minheight) minheight = height;
                            }
                        }
                    }
                }
                else
                {
                    if(fscanf(in,"%d %d",&startline[i],&endline[i]) == 2)
                    {
                        for (j = startline[i]; j < endline[i]; j++)
                        {
                            if (j >= maxx || i >= maxy)
                            {
                                cerr << "Bad row or column value in file (" << j << ", " << i << ")" << endl;
                                continue;
                            }
                            if (hasAlternateData)
                            {
                                if(fscanf(in, "%f %f", &height, &(alternateData[i][j])) == 2)
                                    alternateData[i][j] += alternateDataOffset;
                            }
                            else
                                if(fscanf(in, "%f", &height) != 1) height = 0.0;
                            dem[i][j] = height ;
                            if (height < 0)
                                height = -height; //want any holes to be properly accounted for when later restored.
                            if (height > 0)
                            {
                                if (height > maxheight) maxheight = height;
                                if (height < minheight) minheight = height;
                            }
                        }
                    }
                }
            updateProgress(i*80/maxy);
            }
        }
        // done reading file
//      printf("\nDone loading.\n");
    }
    
    // timing stuff
//  mainWindow->getScene3D()->timeControl.updateCurrentTime();
//  e=mainWindow->getScene3D()->timeControl.getSysTime();
//  cerr << "End: " << e << endl;
//    cerr << "Elapsed: " << e-s << endl;

    bbox.w = pose->getPosition()[0];
    bbox.e = bbox.w + maxx*cellsize;
    bbox.s = pose->getPosition()[1];
    bbox.n = bbox.s + maxy*cellsize;
    if (minheight > 0.0)
        bbox.b = minheight;
    else
        bbox.b = 0.0;
    bbox.t = maxheight;

    //&&& part of the preferredOffset hack
    calculateCenter();

    //cout << "Bottom  :" << bbox.b  << "\n";
    //cerr << "Gutm::bbox e " << bbox.e << " w " << bbox.w << " n " << bbox.n << " s " << bbox.s << endl;

    cout.flush();

    // calculate normals

    std::cout << "Calculating normals..." << endl;
    updateTitle("Calculating normals...");

    if (normalsOnTheFly)
        CalculateNormals(1.0f);
    else
        CalculateNormals(lastHeightExaggeration);

    //printf("Done.\n");

    killProgress();
    
    
    // done
    return(0);
}

/*---------------------------------------------------------------------------*/

SDV_DataHandler * Gutm::GetHandler(int x, int y)
{
    return rtdata[y][x];
}

/*---------------------------------------------------------------------------*/
int Gutm::AllocateGrid(bool zero)
{
    int i, j;

    // allocate the vertex array
    norms_data = new vNorms[maxx*maxy];
    norms_array = new vNorms*[maxy];
    if (!norms_array || !norms_data)
    {
        cerr << "Can't allocate Normals array! Buy more ram!\n";
        return 1;
    }
    for (i = 0; i < maxy; i++)
    {
        norms_array[i] = &norms_data[i*maxx];
    }
    
    dem_data = new float[maxx*maxy];
    dem = new float*[maxy];
    if ( !dem || !dem_data)
    {
        cerr << "Can't allocate heights array! Buy more ram!\n";
        return 1;
    }

    startline = new int[maxy];
    endline = new int[maxy];

    for (i = 0; i < maxy; i++)
    {
        dem[i] = &dem_data[i*maxx];
    }

// real time
    if(rtflag)
    {
        rtdata = new SDV_DataHandler**[maxy];
        startValidTime = new double[maxy];
        endValidTime = new double[maxy];
        if ( rtdata == NULL)
        {
            cerr << "Can't allocate real time data array! Buy more ram!\n";
            return 1;
        }
        for (i = 0; i < maxy; i++)
        {
            startValidTime[i] = gz4d::Nan<double>();
            endValidTime[i] = gz4d::Nan<double>();
        
            rtdata[i] = new SDV_DataHandler*[maxx];
            if ( rtdata[i] == NULL)
            {
                cerr << "Can't allocate real time data array! Buy more ram!\n";
                return 1;
            }
        }
    }
    

    //allocate alternate data
    if (hasAlternateData)
    {
        alternateData = new float*[maxy];
        if ( alternateData == NULL)
        {
            cerr << "Can't allocate data array! Buy more ram!\n";
            return 1;
        }
        for (i = 0; i < maxy; i++)
        {
            alternateData[i] = new float[maxx];
            if ( alternateData[i] == NULL)
            {
                cerr << "Can't allocate data array! Buy more ram!\n";
                return 1;
            }
        }
    }

    // load the heights
    if (zero)
    {
        for (i = 0; i < maxy; i++)
        {
            startline[i] = 0;
            endline[i] = maxx;
    
            for (j = 0; j < maxx; j++)
            {
                dem[i][j] = 0.0 ;
                if(rtflag)
                    rtdata[i][j] = NULL;
                if(hasAlternateData)
                    alternateData[i][j] = 0.0;
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
int Gutm::writeTo(FILE *file)
{
    int i, j, start, end;

    fprintf(file, "%s %d %d\n", Keyword(COLS_ROWS), maxx, maxy);
    fprintf(file, "%s %f\n", Keyword(CELLSIZE), cellsize);
    fprintf(file, "%s %s\n", Keyword(SHAPE), Keyword(shape));
    if (hasAlternateData)
        fprintf(file, "%s %s %f\n", Keyword(USE_ALTERNATE_DATA), Keyword(OFFSET), alternateDataOffset);
    Object::writeTo(file);

    start = 0;
    end = maxx;
    for (i = 0; i < maxy; i++)
    {
        if (shape != FULLGRID)
        {
            fprintf(file, "%d %d\n", startline[i], endline[i]);
            start = startline[i];
            end = endline[i];
        }
        for (j = start; j < end; j++)
        {
            if (hasAlternateData)
                fprintf(file, "%f %f\n", dem[i][j], alternateData[i][j] - alternateDataOffset);
            else
                fprintf(file, "%f\n", dem[i][j]);
        }
    }

    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
void Gutm::CalculateNormals(float verticalScale, int progressStart)
{
/*  float x1,x2,y1,y2;
    int i, j;
    float dhx, dhy, dhz, l;

    for ( i = 1; i < maxy-1; i++)
    {
        for (j = 1; j < maxx-1; j++)
        {
            x1 = dem[i][j-1];
            x2 = dem[i][j+1];
            y1 = dem[i-1][j];
            y2 = dem[i+1][j];

            dhx = ((x1 - x2)/2.0f)*verticalScale;
            dhy = ((y1 - y2)/2.0f)*verticalScale;

            // the grid gap is cellsize
            l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

            dhx /= l;
            dhy /= l;
            dhz = cellsize/l;
            
            norms_array[i][j].nx = (GLshort)(32767*dhx);
            norms_array[i][j].ny = (GLshort)(32767*dhy);
            norms_array[i][j].nz = (GLshort)(32767*dhz);
        }
        norms_array[i][0].nx = norms_array[i][1].nx;
        norms_array[i][0].ny = norms_array[i][1].ny;
        norms_array[i][0].nz = norms_array[i][1].nz;

        norms_array[i][maxx-1].nx = norms_array[i][maxx-2].nx;
        norms_array[i][maxx-1].ny = norms_array[i][maxx-2].ny;
        norms_array[i][maxx-1].nz = norms_array[i][maxx-2].nz;
        updateProgress(i*20/maxy+80);
    }
    for (i = 0; i < maxx; i++) // make the last row the same
    {
        norms_array[0][i].nx = norms_array[1][i].nx;
        norms_array[0][i].ny = norms_array[1][i].ny;
        norms_array[0][i].nz = norms_array[1][i].nz;

        norms_array[maxy-1][i].nx = norms_array[maxy-2][i].nx;
        norms_array[maxy-1][i].ny = norms_array[maxy-2][i].ny;
        norms_array[maxy-1][i].nz = norms_array[maxy-2][i].nz;
    }*/

float v1,v2,v3;
int i=0, j=0;
float dhx, dhy, dhz, l;
int progressMult = 100 - progressStart;

// use forward differences to calculate normals

    for ( i = 0; i < maxy-1; i++)
    {
        for (j = 0; j < maxx-1; j++)
        {
            v1 = fabs(dem[i][j]);   // abs() added to calc proper
            v2 = fabs(dem[i+1][j]); // norms when gutm has holes (neg values)
            v3 = fabs(dem[i][j+1]); // rja 11/30/2005

            dhy = (v1 - v2)*verticalScale;
            dhx = (v1 - v3)*verticalScale;

            // the grid gap is cellsize
            l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

            dhx /= l;
            dhy /= l;
            dhz = cellsize/l;

/*          norms_array[i][j].nx = (GLshort)(32767*dhx); //HACK
            norms_array[i][j].ny = (GLshort)(32767*dhy);
            if (normalsOnTheFly)
                norms_array[i][j].nz = (GLshort)(32767*dhz);
            else
                norms_array[i][j].nz = (GLshort)(32767*dhz/shadeExaggeration);*/
            norms_array[i][j][X] = (GLshort)(32767*dhx); //HACK
            norms_array[i][j][Y] = (GLshort)(32767*dhy);
            if (normalsOnTheFly)
                norms_array[i][j][Z] = (GLshort)(32767*dhz);
            else
                norms_array[i][j][Z] = (GLshort)(32767*dhz/shadeExaggeration);
        }
        if(east)
        {
            v1 = fabs(dem[i][maxx-1]);
            v2 = fabs(dem[i+1][maxx-1]);
            v3 = fabs(east->GetValue(i,0));

            dhy = (v1 - v2)*verticalScale;
            dhx = (v1 - v3)*verticalScale;

            // the grid gap is cellsize
            l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

            dhx /= l;
            dhy /= l;
            dhz = cellsize/l;

/*          norms_array[i][maxx-1].nx = (GLshort)(32767*dhx);
            norms_array[i][maxx-1].ny = (GLshort)(32767*dhy);
            if (normalsOnTheFly)
                norms_array[i][j].nz = (GLshort)(32767*dhz);
            else
                norms_array[i][j].nz = (GLshort)(32767*dhz/shadeExaggeration);*/
            norms_array[i][maxx-1][X] = (GLshort)(32767*dhx);
            norms_array[i][maxx-1][Y] = (GLshort)(32767*dhy);
            if (normalsOnTheFly)
                norms_array[i][j][Z] = (GLshort)(32767*dhz);
            else
                norms_array[i][j][Z] = (GLshort)(32767*dhz/shadeExaggeration);
        }
        else
        {
/*          norms_array[i][maxx-1].nx = norms_array[i][maxx-2].nx;
            norms_array[i][maxx-1].ny = norms_array[i][maxx-2].ny;
            norms_array[i][maxx-1].nz = norms_array[i][maxx-2].nz;*/
            norms_array[i][maxx-1][X] = norms_array[i][maxx-2][X];
            norms_array[i][maxx-1][Y] = norms_array[i][maxx-2][Y];
            norms_array[i][maxx-1][Z] = norms_array[i][maxx-2][Z];
        }
        updateProgress(i*progressMult/maxy+progressStart);
    }
    for (i = 0; i < maxx; i++) // make the last row the same
    {
        if(north && i < maxx-1)
        {
            v1 = fabs(dem[maxy-1][i]);
            v2 = fabs(north->GetValue(0,i));
            v3 = fabs(dem[maxy-1][i+1]);

            dhy = (v1 - v2)*verticalScale;
            dhx = (v1 - v3)*verticalScale;

            // the grid gap is cellsize
            l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

            dhx /= l;
            dhy /= l;
            dhz = cellsize/l;

/*          norms_array[maxy-1][i].nx = (GLshort)(32767*dhx);
            norms_array[maxy-1][i].ny = (GLshort)(32767*dhy);
            if (normalsOnTheFly)
                norms_array[i][j].nz = (GLshort)(32767*dhz);
            else
                norms_array[i][j].nz = (GLshort)(32767*dhz/shadeExaggeration);*/
            norms_array[maxy-1][i][X] = (GLshort)(32767*dhx);
            norms_array[maxy-1][i][Y] = (GLshort)(32767*dhy);
            if (normalsOnTheFly)
                norms_array[i][j][Z] = (GLshort)(32767*dhz);
            else
                norms_array[i][j][Z] = (GLshort)(32767*dhz/shadeExaggeration);
        }
        else
        {
/*          norms_array[maxy-1][i].nx = norms_array[maxy-2][i].nx;
            norms_array[maxy-1][i].ny = norms_array[maxy-2][i].ny;
            norms_array[maxy-1][i].nz = norms_array[maxy-2][i].nz;*/
            norms_array[maxy-1][i][X] = norms_array[maxy-2][i][X];
            norms_array[maxy-1][i][Y] = norms_array[maxy-2][i][Y];
            norms_array[maxy-1][i][Z] = norms_array[maxy-2][i][Z];
        }
    }
}

/*--------------------------------------------------------------------------*/
/** This function returns the height at the passed location.  Returns invalidHeight
*   if a height is not found at this location.  If a coordinate array
*   is passed it is filled with the result for an x and y it knows more
*   exactly.  The "secondary" variable is part of a hack for gettig at
*   alternateData from scripting.  It is only used when coord is non-NULL.
*/
double Gutm::GetHeightAt(double x, double y, Point3d coord, GLboolean secondary)
{
    gz4d::Point<double> offset = pose->getPosition();
    GLfloat scale, dx, _dx, dy, diagy, edgey, edge, diagonal;
    int ix, iy, ix2, iy2;

    scale = pose->getScale();
    if (scale == 0.0f)
    scale = 1.0f;

    // test for ranges
    if (coord)
    {
        ix = (int)(0.5 + (x - offset[X])/cellsize);
        iy = (int)(0.5 + (y - offset[Y])/cellsize);

        if((ix >= 0) && (iy >= 0) && (ix < maxx) && (iy < maxy))
        {
            if (dem[iy][ix] >0.0)
            {
                coord[X] = (ix * cellsize) + offset[X]; // + 0.5f;  //I'm not sure why these 0.5's were here... (mdp)
                coord[Y] = (iy * cellsize) + offset[Y]; // + 0.5f;
                if (secondary)
                    coord[Z] = alternateData[iy][ix] - alternateDataOffset;
                else
                    coord[Z] = (dem[iy][ix] + offset[Z]) * scale;
                return coord[Z];
            }
        }
        return invalidHeight;
    }

    dx = (float)((x - offset[X])/cellsize);
    dy = (float)((y - offset[Y])/cellsize);
    ix = int (dx);
    iy = int (dy);
    dx -= ix;
    dy -= iy;
    if ((ix == maxx - 1) && dx < .00001)
        ix2 = ix;
    else
        ix2 = ix + 1;
    if ((iy == maxy - 1) && dy < .00001)
        iy2 = iy;
    else
        iy2 = iy + 1;

    _dx = 1.0f - dx;
    if((ix >= 0) && (iy >= 0) && (ix2 < maxx) && (iy2 < maxy))
    {
        if (dem[iy][ix] <= 0.0 || dem[iy][ix2] <= 0.0 ||
            dem[iy2][ix] <= 0.0 || dem[iy2][ix2] <= 0.0)
            return invalidHeight;
        //interpolate
        diagonal = (_dx * dem[iy2][ix]) + (dx * dem[iy][ix2]);
        if (dy >= _dx)
        {
            edge = (_dx * dem[iy2][ix]) + (dx * dem[iy2][ix2]);
            diagy = (1 - dy) / dx;
            edgey = 1 - diagy;
        }
        else
        {
            edge = (_dx * dem[iy][ix]) + (dx * dem[iy][ix2]);
            diagy = dy / _dx;
            edgey = 1 - diagy;
        }
        return ((edgey * edge) + (diagy * diagonal) + offset[Z]) * scale ;
    }
    return invalidHeight;
}

/*--------------------------------------------------------------------------*/
/** This function returns a height summary of the specified area surrounding
*   the passed location.  Defaults to the maximum height (Object::height_max)
*   but can also do Object::height_min and Object::height_average.  Returns
*   invalidHeight if a height is not found near this location or if the range
*   doesn't cross any cells in the GUTM.  The area is always a box with the
*   length of twice the rangeMeters parameter on each side.  Right now, the
*   function is very crude, in that it works only with cell values and does
*   not do any interpolation.
*   Function created by mdp 1/3/2006 to support vessel shadow functionality.
*/
double Gutm::GetHeightNear(double x, double y, float rangeMeters, int heightType)
{
    gz4d::Point<double> offset = pose->getPosition();
    GLfloat scale, dxLeft, dxRight, dyBottom, dyTop;
    int ixLeft, ixRight, iyBottom, iyTop;
    int ix, iy, count = 0;
    float val, dval;
    val = invalidHeight;

    scale = pose->getScale();
    if (scale == 0.0f)
    scale = 1.0f;

    //work out the boundaries in dem coordinates.
    dxLeft = (float)((x - offset[X] - rangeMeters)/cellsize);
    ixLeft = int (dxLeft);
//    dxLeft -= ixLeft;
    if (ixLeft < 0)
    {
        ixLeft = 0;
//      dxLeft = 0.0;
    }

    dxRight = (float)((x - offset[X] + rangeMeters)/cellsize);
    ixRight = int (dxRight);
//    dxRight -= ixRight;
    if (ixRight > maxx - 1)
    {
        ixRight = maxx - 1;
//      dxRight = 0.0;
    }

    dyBottom = (float)((y - offset[Y] - rangeMeters)/cellsize);
    iyBottom = int (dyBottom);
//    dyBottom -= iyBottom;
    if (iyBottom < 0)
    {
        iyBottom = 0;
//      dyBottom = 0.0;
    }

    dyTop = (float)((y - offset[Y] + rangeMeters)/cellsize);
    iyTop = int (dyTop);
//    dyTop -= iyTop;
    if (iyTop > maxy - 1)
    {
        iyTop = maxy - 1;
//      dyTop = 0.0;
    }

    //see if we're just completely out of range
    if (iyTop <= iyBottom || ixRight <= ixLeft)
        return invalidHeight;

    //do the actual work now
    for (ix = ixLeft; ix <= ixRight; ix++)
    {
        for (iy = iyBottom; iy <= iyTop; iy++)
        {
            dval = dem[iy][ix];
            if (dval <= 0.0)
                continue; //skip over holes
            if (!count++)
            {
                val = dval;
                continue;
            }
            switch(heightType)
            {
                case height_max:
                default:
                    if (dval > val)
                        val = dval;
                    break;
                case height_min:
                    if (dval < val)
                        val = dval;
                    break;
                case height_average:
                    val += dval;
                    break;
            }
        }
    }

    //return the answer based on heightType
    val = (val + offset[Z]) * scale;
    if (count && (heightType == height_average))
        return val / count;
    else
        return val;
}

/*--------------------------------------------------------------------------*/
gz4d::Point<double> Gutm::getPreferredOffset()
{
    return centerOfGutm;
}

/*--------------------------------------------------------------------------*/
void Gutm::SetValue(int x, int y, double z, bool allowZero)
{
    float v1,v2,v3;
    float dhx, dhy, dhz, l;
    float verticalScale;

    if (normalsOnTheFly)
        verticalScale = 1.0;
    else
        verticalScale = lastHeightExaggeration;

//  cerr << "Gutm: " << GetLabel() << " cell: " << x << "," << y << " set to: " << z << endl;

    if(z==0)
    {
        if(allowZero)
            dem[y][x] = 0;
        return;
    }
    
    if(dem[y][x] == z - GetPosition(2))
        return;

// the easy part, setting the value
    dem[y][x] = (float) (z - GetPosition(2));

    if(dem[y][x] > bbox.t)
        bbox.t = dem[y][x];
    if(dem[y][x] < bbox.b)
        bbox.b = dem[y][x];

    // the trickier part, updating affected normals
    if(y>0 && x<maxx-1)
    {
        v1 = dem[y-1][x];
        v2 = dem[y][x];
        v3 = dem[y][x+1];

        dhy = (v1 - v2)*verticalScale;
        dhx = (v1 - v3)*verticalScale;

        // the grid gap is cellsize
        l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

        dhx /= l;
        dhy /= l;
        dhz = cellsize/l;

/*      norms_array[y-1][x].nx = (GLshort)(32767*dhx);
        norms_array[y-1][x].ny = (GLshort)(32767*dhy);
        norms_array[y-1][x].nz = (GLshort)(32767*dhz);*/
        norms_array[y-1][x][X] = (GLshort)(32767*dhx);
        norms_array[y-1][x][Y] = (GLshort)(32767*dhy);
        norms_array[y-1][x][Z] = (GLshort)(32767*dhz);
    }
    if(x>0 && y<maxy-1)
    {
        v1 = dem[y][x-1];
        v2 = dem[y+1][x-1];
        v3 = dem[y][x];

        dhy = (v1 - v2)*verticalScale;
        dhx = (v1 - v3)*verticalScale;

        // the grid gap is cellsize
        l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

        dhx /= l;
        dhy /= l;
        dhz = cellsize/l;

/*      norms_array[y][x-1].nx = (GLshort)(32767*dhx);
        norms_array[y][x-1].ny = (GLshort)(32767*dhy);
        norms_array[y][x-1].nz = (GLshort)(32767*dhz);*/
        norms_array[y][x-1][X] = (GLshort)(32767*dhx);
        norms_array[y][x-1][Y] = (GLshort)(32767*dhy);
        norms_array[y][x-1][Z] = (GLshort)(32767*dhz);
    }
    if(y<maxy-1 && x<maxx-1)
    {
        v1 = dem[y][x];
        v2 = dem[y+1][x];
        v3 = dem[y][x+1];

        dhy = (v1 - v2)*verticalScale;
        dhx = (v1 - v3)*verticalScale;

        // the grid gap is cellsize
        l = (float)sqrt(dhx*dhx + dhy*dhy + cellsize*cellsize);

        dhx /= l;
        dhy /= l;
        dhz = cellsize/l;

/*      norms_array[y][x].nx = (GLshort)(32767*dhx);
        norms_array[y][x].ny = (GLshort)(32767*dhy);
        norms_array[y][x].nz = (GLshort)(32767*dhz);*/
        norms_array[y][x][X] = (GLshort)(32767*dhx);
        norms_array[y][x][Y] = (GLshort)(32767*dhy);
        norms_array[y][x][Z] = (GLshort)(32767*dhz);
    }
    
    if(y==maxy-1 && x==maxx-1)
    {
        if(north)
        {
/*          norms_array[y][x].nx = north->GetNormX(x,0);
            norms_array[y][x].ny = north->GetNormY(x,0);
            norms_array[y][x].nz = north->GetNormZ(x,0);*/
            norms_array[y][x][X] = north->GetNormX(x,0);
            norms_array[y][x][Y] = north->GetNormY(x,0);
            norms_array[y][x][Z] = north->GetNormZ(x,0);
        }
        else if(east)
        {
/*          norms_array[y][x].nx = east->GetNormX(0,y);
            norms_array[y][x].ny = east->GetNormY(0,y);
            norms_array[y][x].nz = east->GetNormZ(0,y);*/
            norms_array[y][x][X] = east->GetNormX(0,y);
            norms_array[y][x][Y] = east->GetNormY(0,y);
            norms_array[y][x][Z] = east->GetNormZ(0,y);
        }
        else
        {
/*          norms_array[y][x].nx = norms_array[y-1][x].nx;
            norms_array[y][x].ny = norms_array[y-1][x].ny;
            norms_array[y][x].nz = norms_array[y-1][x].nz;*/
            norms_array[y][x][X] = norms_array[y-1][x][X];
            norms_array[y][x][Y] = norms_array[y-1][x][Y];
            norms_array[y][x][Z] = norms_array[y-1][x][Z];
        }
    }
    else
    {
        if(y==maxy-1)
        {
            if(north)
            {
/*              norms_array[y][x].nx = north->GetNormX(x,0);
                norms_array[y][x].ny = north->GetNormY(x,0);
                norms_array[y][x].nz = north->GetNormZ(x,0);*/
                norms_array[y][x][X] = north->GetNormX(x,0);
                norms_array[y][x][Y] = north->GetNormY(x,0);
                norms_array[y][x][Z] = north->GetNormZ(x,0);
            }
            else
            {
/*              norms_array[y][x].nx = norms_array[y-1][x].nx;
                norms_array[y][x].ny = norms_array[y-1][x].ny;
                norms_array[y][x].nz = norms_array[y-1][x].nz;*/
                norms_array[y][x][X] = norms_array[y-1][x][X];
                norms_array[y][x][Y] = norms_array[y-1][x][Y];
                norms_array[y][x][Z] = norms_array[y-1][x][Z];
            }
        }
        if(x==maxx-1)
        {
            if(east)
            {
/*              norms_array[y][x].nx = east->GetNormX(0,y);
                norms_array[y][x].ny = east->GetNormY(0,y);
                norms_array[y][x].nz = east->GetNormZ(0,y);*/
                norms_array[y][x][X] = east->GetNormX(0,y);
                norms_array[y][x][Y] = east->GetNormY(0,y);
                norms_array[y][x][Z] = east->GetNormZ(0,y);
            }
            else
            {
/*              norms_array[y][x].nx = norms_array[y][x-1].nx;
                norms_array[y][x].ny = norms_array[y][x-1].ny;
                norms_array[y][x].nz = norms_array[y][x-1].nz;*/
                norms_array[y][x][X] = norms_array[y][x-1][X];
                norms_array[y][x][Y] = norms_array[y][x-1][Y];
                norms_array[y][x][Z] = norms_array[y][x-1][Z];
            }
        }
    }
}

/*--------------------------------------------------------------------------*/
float Gutm::GetValue(int x, int y)
{
    return dem[y][x];
}

void Gutm::SetAltValue(int x, int y, float v)
{
    if (hasAlternateData)
        alternateData[y][x] = v;
}

/*--------------------------------------------------------------------------*/
GLshort Gutm::GetNormX(int x, int y)
{
    //return norms_array[y][x].nx;
    return norms_array[y][x][X];
}

/*--------------------------------------------------------------------------*/
GLshort Gutm::GetNormY(int x, int y)
{
//  return norms_array[y][x].ny;
    return norms_array[y][x][Y];
}

/*--------------------------------------------------------------------------*/
GLshort Gutm::GetNormZ(int x, int y)
{
//  return norms_array[y][x].nz;
    return norms_array[y][x][Z];
}

/*--------------------------------------------------------------------------*/
double Gutm::GetValue(double x,double y)
{
    int xi,yi;

x -= pose->getPosition()[X];
y -= pose->getPosition()[Y];

    
    x /= cellsize;
    y /= cellsize;

    
xi = (int)x;
yi = (int)y;

    if(xi<0 || xi >= maxx || yi < 0 || yi>= maxy)
    {
        return 0.0;
    }
    else
    {
        if(dem[yi][xi] > 0.0)
        {
            return (GetPosition(2)+dem[yi][xi]);
        }
        else
        {
            return 0.0;
        }
    }
}

/*--------------------------------------------------------------------------*/
int Gutm::HasValue(double x,double y)
{
    int xi,yi;

    x -= GetPosition(X);
    y -= GetPosition(Y);
    x /= cellsize;
    y /= cellsize;

xi = (int)x;
yi = (int)y;

    if(xi<0 || xi >= maxx || yi < 0 || yi>= maxy)
        return 0;
    else
    {
        cerr << xi << " " << yi << " " << dem[yi][xi] << endl;
    if(dem[yi][xi]>0.0)
        return 1;
    else
        return 0;
    }
}

/*--------------------------------------------------------------------------*/
int Gutm::HasValueInRange(double x1,double y1, double x2, double y2)
{
    int xi1,yi1,xi2,yi2;
    int i,j;

    x1 -= GetPosition(X);
    y1 -= GetPosition(Y);
    x1 /= cellsize;
    y1 /= cellsize;

xi1 = (int)x1;
yi1 = (int)y1;

    x2 -= GetPosition(X);
    y2 -= GetPosition(Y);
    x2 /= cellsize;
    y2 /= cellsize;

xi2 = (int)x2;
yi2 = (int)y2;

    if(xi1 > xi2)
    {
        i=xi1;
        xi1 = xi2;
        xi2 = i;
    }

    if(yi1 > yi2)
    {
        i=yi1;
        yi1 = yi2;
        yi2 = i;
    }

    if( (xi1<0 && xi2 <0) || (xi1 >= maxx && xi2 >= maxx) || (yi1 < 0 && yi2 < 0) || (yi1>= maxy && yi2 >= maxy))
        return 0;

    if(xi1<0) xi1 = 0;
    if(xi2<0) xi2 = 0;
    if(yi1<0) yi1 = 0;
    if(yi2<0) yi2 = 0;

    if(xi1>=maxx) xi1 = maxx-1;
    if(xi2>=maxx) xi2 = maxx-1;
    if(yi1>=maxy) yi1 = maxy-1;
    if(yi2>=maxy) yi2 = maxy-1;
    
            
    for(i=xi1;i<=xi2;i++)
        for(j=yi1;j<=yi2;j++)
        {
        if(dem[j][i]>0.0)
            return 1;
        }       
    return 0;
}

/*--------------------------------------------------------------------------*/
int Gutm::loadFromNetCDF(gz4d::netcdf::File ncfile, string var, string normsvar, int timestep)
{
    Point3d gr;
    float height, minheight, maxheight;
    minheight = 100000000.0;
    maxheight = -100000000.0;


    int i,j;

    shape = FULLGRID;

    gz4d::netcdf::Att ncatt;
    
    for(i=0;i<ncfile.NumAtts();i++)
    {
        ncatt = ncfile.GetAtt(i);
        if (ncatt.Name() == string("localx"))
            gr[X] = ncatt.As<double>(0);
        if (ncatt.Name() == string("localy"))
            gr[Y] = ncatt.As<double>(0);
        if (ncatt.Name() == string("localz"))
            gr[Z] = ncatt.As<double>(0);
        if (ncatt.Name() == string("cellsize"))
            cellsize = ncatt.As<float>(0);
    }

    maxx = ncfile.GetDim("col").Size();
    maxy = ncfile.GetDim("row").Size();

    AllocateGrid();

    gz4d::netcdf::Var v = ncfile.GetVar(var);

    for(i=0;i<v.NumAtts();i++)
    {
        ncatt = v.GetAtt(i);
        if (ncatt.Name() == string("localx"))
            gr[X] = ncatt.As<double>(0);
        if (ncatt.Name() == string("localy"))
            gr[Y] = ncatt.As<double>(0);
        if (ncatt.Name() == string("localz"))
            gr[Z] = ncatt.As<double>(0);
        if (ncatt.Name() == string("cellsize"))
            cellsize = ncatt.As<float>(0);
    }


    SetPosition(gr);

    if(timestep == -1)
    {
        v.SetCur(0,0);
        v.Get(dem_data,maxy,maxx);
    }
    else
    {
        v.SetCur(timestep,0,0);
        v.Get(dem_data,1,maxy,maxx);
    }

    bbox.w = gr[X];
    bbox.e = bbox.w + maxx*cellsize;
    bbox.s = gr[Y];
    bbox.n = bbox.s + maxy*cellsize;

    for(i=0;i<maxy;i++)
        for(j=0;j<maxx;j++)
        {
            height = dem[i][j];
            if (height < 0)
                height = -height; //want any holes to be properly accounted for when later restored.
            if (height > 0)
            {
                if (height > maxheight) maxheight = height;
                if (height < minheight) minheight = height;
            }
        }
    
    bbox.t = gr[Z] + maxheight;
    bbox.b = gr[Z] + minheight;
    
    if(!normsvar.empty())
    {
        gz4d::netcdf::Var nv = ncfile.GetVar(normsvar);
        if(timestep == -1)
        {
            nv.SetCur(0,0,0);
            nv.Get((short *)norms_data,maxy,maxx,3);
        }
        else
        {
            nv.SetCur(timestep,0,0,0);
            nv.Get((short *)norms_data,1,maxy,maxx,3);
        }
        
    }
    else
    {
        printf("Calculating normals...");
        
        if (normalsOnTheFly)
                CalculateNormals(1.0f);
        else
                CalculateNormals(lastHeightExaggeration);
        
        printf("Done.\n");
    }   

    calculateCenter();

    return 0;
}

/*--------------------------------------------------------------------------*/
Object * Gutm::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml,path,ncfile);

    boost::optional<std::string> ncvar = xml.get_optional<std::string>("<xmlattr>.ncvar");
    if(ncvar)
    {
        std::string norms;
        norms = xml.get("<xmlattr>.ncnorms",norms);
        if(!loadFromNetCDF(ncfile, *ncvar, norms, timeIndex))
            return this;
    }
    return NULL;
}

/*--------------------------------------------------------------------------*/
GLboolean Gutm::LoadBag(const char *path)
{
#ifdef USE_BAG_FILES
    bagHandle hnd;
    bagError stat;
    bagData *data;
    int i, j;
    Point3d gr = {0.0, 0.0, 0.0};
    float offset;
    float minheight, maxheight;
    
    if (path)
    {
        SetFilename(Filename(path));
    }
    else
        path = GetFilename().string();

    //open the BAG file
    stat = bagFileOpen(&hnd, BAG_OPEN_READONLY, (const unsigned char *)path);
    if (stat != BAG_SUCCESS)
        return GL_FALSE;
    data = bagGetDataPointer(hnd);

    //create the internal representation of the grid
    maxx = data->def.ncols;
    maxy = data->def.nrows;
    cellsize = data->def.nodeSpacingX; //assume they're the same, assume UTM for now...&&&
    if (cellsize <= 0)
        cellsize = 1.0;
    minheight = data->min_elevation;
    maxheight = data->max_elevation;
    offset = -minheight + 1.0;
    gr[X] = data->def.swCornerX;
    gr[Y] = data->def.swCornerY;
    gr[Z] = -offset;
    maxheight += offset;
    minheight += offset;
    bbox.w = gr[X];
    bbox.e = bbox.w + maxx*cellsize;
    bbox.s = gr[Y];
    bbox.n = bbox.s + maxy*cellsize;
    bbox.b = minheight;
    bbox.t = maxheight;
    SetPosition(gr);

    hasAlternateData = true;
    AllocateGrid();

    stat = bagReadXMLStream(hnd);
    stat = bagInitDefinitionFromBag(hnd);
    //read in the rows
    createProgress();
    for (i = 0; i < maxy; i++)
    {
        bagReadRow(hnd, i, 0, maxx - 1, Elevation, dem[i]);
        bagReadRow(hnd, i, 0, maxx - 1, Uncertainty, alternateData[i]);
        for (j = 0; j < maxx; j++)
        {
            if (dem[i][j] >= FLT_MAX || alternateData[i][j] >= FLT_MAX)
            {
                dem[i][j] = 0.0;
                alternateData[i][j] = -1.0;
            }
            else
                dem[i][j] += offset;
        }
        updateProgress(i*50/maxy);
    }
    updateTitle("Calculating normals...");
    if (normalsOnTheFly)
        CalculateNormals(1.0f, 50);
    else
        CalculateNormals(lastHeightExaggeration, 50);

    calculateCenter();

    //close the file
    stat = bagFileClose(hnd);
    killProgress();

    return GL_TRUE;
#else
    return GL_FALSE;
#endif
}

void Gutm::calculateCenter()
{
    centerOfGutm[X] = bbox.w + ((bbox.e -bbox.w)/ 2.0);
    centerOfGutm[Y] = bbox.s + ((bbox.n -bbox.s)/ 2.0);
    centerOfGutm[Z] = GetHeightAt(centerOfGutm[X], centerOfGutm[Y]); //dem[maxy / 2][maxx / 2];
    if (centerOfGutm[Z] <= invalidHeight)
        centerOfGutm[Z] = bbox.b;
}


/*--------------------------------------------------------------------------*/
void Gutm::print()
{
    int i,j;
    for(i=0;i<maxy;i++)
    {
        for(j=0;j<maxx;j++)
        {
            cerr << dem[i][j] << " , ";
        }
        cerr << endl;
    }
}

/*--------------------------------------------------------------------------*/
void Gutm::Initialize(int r, int c,double csize, bool altData)
{
    cerr << "Initializing GUTM with " << r << " rows and " << c << " columns at " << GetPosition(X) << ", " << GetPosition(Y) << endl;
    maxx = c;
    maxy = r;
    shape = FULLGRID;
    cellsize = (float) csize;
    hasAlternateData = altData;
    AllocateGrid();
    bbox.w = GetPosition(X);
    bbox.e = bbox.w + maxx*cellsize;
    bbox.s = GetPosition(Y);
    bbox.n = bbox.s + maxy*cellsize;
    bbox.b = bbox.t = 0.0;
    if (normalsOnTheFly)
        CalculateNormals(1.0f);
    else
        CalculateNormals(lastHeightExaggeration);
    if(parent)
        parent->setBBoxStale();
}

/*--------------------------------------------------------------------------*/
void Gutm::ApplyNormal(int x, int y, float he)
{
/*  if (normalsOnTheFly)
        glNormal3f(norms_array[y][x].nx, norms_array[y][x].ny, norms_array[y][x].nz / he);
    else
        glNormal3s(norms_array[y][x].nx, norms_array[y][x].ny, norms_array[y][x].nz);*/
    if (normalsOnTheFly)
        glNormal3f(norms_array[y][x][X], norms_array[y][x][Y], norms_array[y][x][Z] / he);
    else
        glNormal3sv(norms_array[y][x]);
}

/*--------------------------------------------------------------------------*/
Gutm *Gutm::createCopy(int startx, int starty, int endx, int endy)
{
    Gutm *newg;
    int newMaxx, newMaxy, retval, i, j, idx;
    Point3d gr;
    float minheight, maxheight, height;
    char newname[256];

    //set the bounds up right.  When called, end indicates last cell to
    //copy, but from here on, it will be a loop bound (end + 1);
    if (endx < 0 || endx > maxx)
        endx = maxx;
    else
        endx++;
    if (endy < 0 || endy > maxy)
        endy = maxy;
    else
        endy++;

    //set values we'll be making heavy use of locally
    newMaxx = endx - startx;
    newMaxy = endy - starty;

    //create the new Gutm, fill in the basics
    newg = new Gutm();
    newg->maxx = newMaxx;
    newg->maxy = newMaxy;
    newg->shape = FULLGRID;
    newg->cellsize = cellsize;
    newg->shadeExaggeration = shadeExaggeration;
    newg->hasAlternateData = false; //for now, we won't copy it... maybe later.
    GetPosition(gr);
    gr[X] += startx * cellsize;
    gr[Y] += starty * cellsize;
    newg->SetPosition(gr);

    //create the new Gutm's grid
    retval = newg->AllocateGrid();
    if (retval)
    {
        delete newg;
        return NULL;
    }
    minheight = 100000000.0;
    maxheight = -100000000.0;

    for (i = 0; i < newMaxy; i++)
    {
        idx = starty + i;
        for (j = 0; j < newMaxx; j++)
        {
            height = dem[idx][startx + j];
            newg->dem[i][j] = height;
            if (height < 0)
                height = -height; //want any holes to be properly accounted for when later restored.
            if (height > 0)
            {
                if (height > maxheight) maxheight = height;
                if (height < minheight) minheight = height;
            }
        }
    }

    newg->bbox.w = gr[X];
    newg->bbox.e = gr[X] + newMaxx * cellsize;
    newg->bbox.s = gr[Y];
    newg->bbox.n = gr[Y] + newMaxy * cellsize;
    if (minheight > 0.0)
        newg->bbox.b = minheight;
    else
        newg->bbox.b = 0.0;
    newg->bbox.t = maxheight;

    if (normalsOnTheFly)
        newg->CalculateNormals(1.0f);
    else
        newg->CalculateNormals(lastHeightExaggeration);

    sprintf(newname, "%s_copy_%d_%d", getAddress()->getName().c_str(), startx, starty);
    newg->SetLabel(newname);

    mainWindow->getScene3D()->AddObject(newg, mainWindow->interp);
    newg->SetCustomTexture(mainWindow->getScene3D()->GetTexturePtr("default"));

    return newg;
}

/*--------------------------------------------------------------------------*/
void Gutm::setRubberband(RubberBandObject *r)
{
    float maxPos[3];

    if (r == rubberBand)
        return;

    rubberBand = r;
    if (!rubberBand)
        return;

    gz4d::Point<double> basePos = pose->getPosition();
    maxPos[X] = basePos[X] + cellsize * maxx;
    maxPos[Y] = basePos[Y] + cellsize * maxy;

    rubberBand->setCorner(0, basePos[X], basePos[Y], bbox.b + basePos[Z] + cellsize);
    rubberBand->setCorner(1, maxPos[X], maxPos[Y], bbox.t + basePos[Z]);
    rubberBand->setValidRange(basePos[X], maxPos[X], basePos[Y], maxPos[Y], cellsize);
    rubberBand->setKeepPointsOnTop();
    rubberBand->SetVisible();
    setSelectedObject(rubberBand);
}

/*--------------------------------------------------------------------------*/
GLboolean Gutm::getRubberbandIndexes(int &startx, int &starty, int &endx, int &endy)
{
    gz4d::Point<double> point1, point2, pos;
    int temp;
        
    if (!rubberBand)
        return GL_FALSE;

    pos = pose->getPosition();
    point1 = rubberBand->getCorner(0);
    point2 = rubberBand->getCorner(1);

    startx = (int)((point1[X] - pos[X]) / cellsize);
    endx = (int)((point2[X] - pos[X]) / cellsize);
    if (startx > endx)
    {
        temp = startx;
        startx = endx;
        endx = temp;
    }

    starty = (int)((point1[Y] - pos[Y]) / cellsize);
    endy = (int)((point2[Y] - pos[Y]) / cellsize);
    if (starty > endy)
    {
        temp = starty;
        starty = endy;
        endy = temp;
    }
    return GL_TRUE;
}

/*--------------------------------------------------------------------------*/
void Gutm::rubberbandCopy()
{
    int startx, starty, endx, endy;
        
    if (!getRubberbandIndexes(startx, starty, endx, endy))
        return;

    createCopy(startx, starty, endx, endy);
}

/*--------------------------------------------------------------------------*/
void Gutm::rubberbandHole()
{
    int startx, starty, endx, endy, i, j;
    float height;
        
    if (!getRubberbandIndexes(startx, starty, endx, endy))
        return;

    for (i = starty; i < endy; i++)
    {
        for (j = startx; j < endx; j++)
        {
            height = dem[i][j];
            if (height > 0.0)
                dem[i][j] = -height;
        }
    }
    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*--------------------------------------------------------------------------*/
void Gutm::rubberbandRestore()
{
    int startx, starty, endx, endy, i, j;
    float height;
        
    if (!getRubberbandIndexes(startx, starty, endx, endy))
        return;

    for (i = starty; i < endy; i++)
    {
        for (j = startx; j < endx; j++)
        {
            height = dem[i][j];
            if (height < 0.0)
                dem[i][j] = -height;
        }
    }
    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

void Gutm::Idle(double t)
{
    int i,j;
    
    Object::Idle(t);
    
    if(rtflag)
    {
        for (i = 0; i < maxy; i++)
        {
            for (j = 0; j < maxx; j++)
            {
                if(rtdata[i][j])
                {
                    float a = rtdata[i][j]->GetAverage(t);
                    if (gz4d::IsNan(a))
                        SetValue(j,i,0.0,true);
                    else
                        SetValue(j,i,a,true);
                    if(hasAlternateData)
                    {
                        a = rtdata[i][j]->GetAlternateData(t);
                        if (gz4d::IsNan(a))
                            SetAltValue(j,i,0.0);
                        else
                            SetAltValue(j,i,a);
                    }
                }
            }
        }
    }
    return;

    
    if(rtflag)
    {
        cerr << "\nt= " << t << endl;
        for (i = 0; i < maxy; i++)
        {
            cerr << "i= " << i << endl;
            
            if(!gz4d::IsNan(startValidTime[i])&&!gz4d::IsNan(endValidTime[i]))  // does this row have a valid time range?
            {
                cerr << "validRange: " << startValidTime[i] << " to " << endValidTime[i] << endl;
                if(t<startValidTime[i]||t>endValidTime[i]) // if so, are we out of that range?
                {
                    cerr << "reset\n";
                    startValidTime[i] = gz4d::Nan<double>();  // if so, reset the range
                    endValidTime[i] = gz4d::Nan<double>();
                }
            }
            if(gz4d::IsNan(startValidTime[i])) // no valid range, or out of previous valid range (for current row)
            {
                cerr << "process row\n";
                for (j = 0; j < maxx; j++)
                {
                    if(rtdata[i][j])
                    {
                        cerr << "j= " << j << " value: " << rtdata[i][j]->GetAverage(t) << endl;
                        SetValue(j,i,rtdata[i][j]->GetAverage(t),1);
                        if(gz4d::IsNan(startValidTime[i]))
                            startValidTime[i] = rtdata[i][j]->GetStartValidTime();
                        else
                        {
                            if(rtdata[i][j]->GetStartValidTime() > startValidTime[i])
                                startValidTime[i] = rtdata[i][j]->GetStartValidTime();
                        }
                            
                        if(gz4d::IsNan(endValidTime[i]))
                            endValidTime[i] = rtdata[i][j]->GetEndValidTime();
                        else
                        {
                            if(rtdata[i][j]->GetEndValidTime() < endValidTime[i])
                                endValidTime[i] = rtdata[i][j]->GetEndValidTime();
                        }
                    }
                }
                cerr << "new valid range: "  << startValidTime[i] << " to " << endValidTime[i] << endl;
            }
        }
    }
}

