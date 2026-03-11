// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"CurrentLayer.h"
#include"Current.h"
#include "engine/InputMediator.h"
#include "interactors/DoubleClickInteractor.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "GZincludes.h"
#include "utilities/SmartValue.h"
#include "engine/TextureMaps/ImageMap.h"
#include "utilities/gauss.h"
#include "engine/PosableStateBase.h"
// #ifdef WIN32
// #include <time.h>
// #endif

#define PATHLEN 32
#define MAXAGE  24
#define STRLEN 8

#define NUM_FLOATERS 100 // must be perfect square!!!!


REGISTER_OBJECT_WITH_MENU(CurrentLayer, "currentlayer") = "{"
DEFAULT_OBJECT_MENU_ITEMS
"{Add Floater} addFloater "
"{Add Many Floaters} addManyFloaters "
"{Add Group of Floaters} addGroupOfFloaters "
"{Add Grid of Floaters} addGridOfFloaters "
"}";

extern MainWindow * mainWindow;
extern int dominantEye;

static const char *CurrentLayerAttributeNames[] = {OBJECT_ATTRIBUTES,"floaterTailLength"};
int CurrentLayer::commonMipmapFactor = 2;

CurrentLayer::CurrentLayer(Current * c)
{
    currentTimestep = 0;
    pendingFloater = NULL;
    pendingFloaters = NULL;
    globalInputMediator->PushInteractor(this);

    sigmaIndex = 0;
    sigma = 0.0;
    current = c;
    addingManyFloaters = 0;
    displayType = display_arrows;

    headColor[0] = headColor[1] = headColor[2] = 1.0;
    tailColor[0] = tailColor[1] = tailColor[2] = 1.0;
    spacing = 1;
    
    floaterTailLength = 120;
    floaterGrid = 0;

    numAttributes = NumCurrentLayerAttributes;
    attributeNames = CurrentLayerAttributeNames;

    spacing = 1.0;
    skipCells = 5;
    lineWidth = 2.0;

    transparencyLevel = 0;
    vectorMipmap = NULL;
    numMipmapLevels = 0;
}

/*---------------------------------------------------------------------------*/
CurrentLayer::~CurrentLayer()
{
    destroyVectorMipmap();
}

/*---------------------------------------------------------------------------*/
int CurrentLayer::DrawHighRes(int eye, Window3D * win)
{
    int ret1,ret2;

    ret1 = Object::DrawHighRes(eye,win);
    ret2 = ObjectCollection::DrawHighRes(eye,win);
    return(ret1&&ret2);
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::DrawLowRes(float degradation, Window3D *win)
{
    float lz;
    int i;
    double scale;
    GLint df;

    glPushAttrib(GL_POINT_BIT);

    switch(displayType)
    {
    case display_arrows:
        DrawBoringArrows();
        //DrawMipmappedArrows(degradation, win);
        //DrawMipmappedArrowsOnTheFly(degradation, win);
        break;
    case display_streaks:
        glLineWidth(lineWidth);
        glNormal3f(0.0,0.0,1.0);
        drawStreaks();
        glLineWidth(1.0);
    case display_tex_arrows:
        break;
    }

    
    if(pendingFloater)
    {
        lz = getZ((float)(cursorLocation[1]),(float)(cursorLocation[0]),currentTimestep, GL_FALSE);
        glPointSize(10);
        glColor3f(1.0,0.0,0.0);
        glBegin(GL_POINTS);
            glVertex3f(cursorLocation[0],cursorLocation[1],lz);
        glEnd();
        glGetIntegerv(GL_DEPTH_FUNC,&df);
        glDepthFunc(GL_LEQUAL);
        glPointSize(5);
        glColor3f(1.0,1.0,0.0);
        glBegin(GL_POINTS);
            glVertex3f(cursorLocation[0],cursorLocation[1],lz);
        glEnd();
        glDepthFunc(df);
    }
    if(pendingFloaters)
    {
        scale=mainWindow->getPoseInfo()->getScale();
        lz = getZ((float)(cursorLocation[1]),(float)(cursorLocation[0]),currentTimestep, GL_FALSE);
        glPointSize(10);
        glColor3f(1.0,0.0,0.0);
        glBegin(GL_POINTS);
        for(i=0;i<NUM_FLOATERS;i++)
            glVertex3f(cursorLocation[0]+pendingFloaters[i].x/scale,cursorLocation[1]+pendingFloaters[i].y/scale,lz);
        glEnd();
        glGetIntegerv(GL_DEPTH_FUNC,&df);
        glDepthFunc(GL_LEQUAL);
        glPointSize(5);
        glColor3f(1.0,1.0,0.0);
        glBegin(GL_POINTS);
        for(i=0;i<NUM_FLOATERS;i++)
            glVertex3f(cursorLocation[0]+pendingFloaters[i].x/scale,cursorLocation[1]+pendingFloaters[i].y/scale,lz);
        glEnd();
        glDepthFunc(df);
    }

    glPopAttrib();

    ObjectCollection::DrawLowRes(degradation,win);
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::DrawBoringArrows()
{
    int i,j;
    float lz;
    float p;
    int t2;
    GLint df;

    glLineWidth(3.0);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    t2 = currentTimestep+1;
    if (t2>=timesteps)
        t2--;
    if(t2!=currentTimestep)
        p = (lastIdleTime-times[currentTimestep])/(times[t2]-times[currentTimestep]);
    else
        p = 1.0;
    glNormal3f(0.0,0.0,1.0);
    glGetIntegerv(GL_DEPTH_FUNC,&df);
    glDepthFunc(GL_LEQUAL);
    glBegin(GL_LINES);

    for(i=0;i<rows;i++)
        for(j=0;j<cols;j++)

        {
            if( (u[currentTimestep][i][j]!=0) ||(v[currentTimestep][i][j]!=0))
            {
                lz = getZ(i,j,currentTimestep);
                glColor3fv(tailColor);
                glVertex3d(j*cellsize,i*cellsize,lz-GetPosition(Z));
                glColor3fv(headColor);

                glVertex3d(j*cellsize+( (u[currentTimestep][i][j]*(1-p)+u[t2][i][j]*(p)) *scale),
                           i*cellsize+( (v[currentTimestep][i][j]*(1-p)+v[t2][i][j]*(p)) *scale),lz-GetPosition(Z));

            }
        }
    glEnd();

    glLineWidth(1.0);

    glPointSize(4.0);
    glBegin(GL_POINTS);
    glColor3fv(headColor);
    for(i=0;i<rows;i++)
        for(j=0;j<cols;j++)
        {
            if( (u[currentTimestep][i][j]!=-0) ||(v[currentTimestep][i][j]!=0))
            {
                lz = getZ(i,j,currentTimestep);
                glVertex3d(j*cellsize+( (u[currentTimestep][i][j]*(1-p)+u[t2][i][j]*(p)) *scale),
                    i*cellsize+( (v[currentTimestep][i][j]*(1-p)+v[t2][i][j]*(p)) *scale),lz-GetPosition(Z));
            }
        }
    glEnd();
    glDepthFunc(df);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::DrawMipmappedArrows(float degradation, Window3D *win)
{
/*
    int i, j, k, numPoints, maxDim, stepSize, t2;
    const float CELL_FACTOR = 0.08f;
    float winScale, lz, p, levelCellSize, density;
    Point2f *arrowPoints;
    ViewManager *view;
    PosableStateBase *winPose;
    VectorMipmapLevel *mipmap;
    VectorMipmapCell **cell;

    if (!win)
        return;

    view = win->getViewInfo();
    winPose = view->getPosable();
    winScale = winPose->getScale();

    density = current->getCurrentDensity();
    //find proper mipmap level
    if (cellsize > CELL_FACTOR / (winScale * density))
    {
        DrawBoringArrows();
        return;
    }
    for (i = 0; i < numMipmapLevels; i++)
    {
        mipmap = &vectorMipmap[currentTimestep][i];
        if (mipmap->cellSize <= CELL_FACTOR / (winScale * density))
            break;
    }

    //determine how to interpolate
    t2 = currentTimestep+1;
    if (t2>=timesteps)
        t2--;
    if(t2!=currentTimestep)
        p = (lastIdleTime-times[currentTimestep])/(times[t2]-times[currentTimestep]);
    else
        p = 1.0;

    //draw the arrows for this level
    maxDim = mipmap->maxDim;
    cell = mipmap->cell;
    stepSize = mipmap->stepSize;
    levelCellSize = mipmap->cellSize;
    glLineWidth(3.0);
    for (i = 0; i < maxDim; i++)
    {
        for (j = 0; j < maxDim; j++)
        {
            if (cell[i][j].numPoints <= 0)
                continue;

            glColor3fv(tailColor);
//glLineWidth(3.0);
            glBegin(GL_LINES);
            numPoints = cell[i][j].numPoints;
            arrowPoints = cell[i][j].arrowPoints;
            //lz = getZ(i * stepSize, j * stepSize,currentTimestep) - geoRef[Z];
            lz = getZ(arrowPoints[numPoints/2][X], arrowPoints[numPoints/2][Y], currentTimestep) - geoRef[Z];
            for (k = 0; k < numPoints; k++)
                glVertex3f(arrowPoints[k][X], arrowPoints[k][Y], lz);
            glEnd();

            //draw box

glLineWidth(1.0);
            glBegin(GL_LINE_STRIP);
                glVertex3f(j * levelCellSize, i * levelCellSize, lz);
                glVertex3f((j + 1) * levelCellSize, i * levelCellSize, lz);
                glVertex3f((j + 1) * levelCellSize, (i + 1) * levelCellSize, lz);
                glVertex3f(j * levelCellSize, (i + 1) * levelCellSize, lz);
                glVertex3f(j * levelCellSize, i * levelCellSize, lz);
            glEnd();

            glColor3fv(headColor);
            glBegin(GL_TRIANGLES);
            for (k = 0; k < 3; k++)
                glVertex3f(cell[i][j].arrowHead[k][X], cell[i][j].arrowHead[k][Y], lz);
            glEnd();
        }
    }
*/
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::DrawMipmappedArrowsOnTheFly(float degradation, Window3D *win)
{
    int i, j, k, maxX, maxY, stepSize, t1, t2, middle, mipmapLevel=0, startx = 0, starty = 0;
    const float CELL_FACTOR = 0.08f;
    float winScale, lz, p, levelCellSize, density, tempf, textz;
    float au[ARROW_BOUNDARIES], av[ARROW_BOUNDARIES], ax[ARROW_BOUNDARIES], ay[ARROW_BOUNDARIES];
    float xOffset, yOffset, cu, cv, length, lengthFactor, widthFactor;
    ViewManager *view;
    PosableStateBase *winPose;
    VectorMipmapLevel *mipmap1 = NULL, *mipmap2 = NULL;
    VectorMipmapCell **cellArray = NULL, *cell = NULL, *cell2 = NULL;
    gz4d::Point<double> worldPoint;
    double depth;
    gz4d::Point<double> position, winPosition;
    int ltype;
    char vectorStrength[16];

    static ViewIntersection *vi = NULL;

    if (!win)
        return;

    view = win->getViewInfo();
    winPose = view->getPosable();
    winScale = winPose->getScale();
    vi = view->getIntersectionAtPlaneZ(0.0, 0.001, vi);

    if (!vi)
        return; //?

    density = current->getCurrentDensity();
    ltype = current->getCurrentLengthType();

    //find proper mipmap level
    if (cellsize > CELL_FACTOR / (winScale * density))
    {
        mipmap1 = NULL;
        mipmapLevel = numMipmapLevels;
    }
    else
    {
        for (i = 0; i < numMipmapLevels; i++)
        {
            mipmapLevel = i;
            mipmap1 = &vectorMipmap[currentTimestep][mipmapLevel];
            if (mipmap1->cellSize <= CELL_FACTOR / (winScale * density))
                break;
        }
    }

    //determine how to interpolate
    t1 = currentTimestep;
    t2 = t1 + 1;
    if (t2>=timesteps)
        t2--;
    if(t2!=currentTimestep)
        p = (lastIdleTime-times[currentTimestep])/(times[t2]-times[currentTimestep]);
    else
        p = 1.0;

    //draw the arrows for this level
    if (mipmap1)
    {
        mipmap2 = &vectorMipmap[t2][mipmapLevel];
        maxX = maxY = mipmap1->maxDim;
        cellArray = mipmap1->cell;
        stepSize = mipmap1->stepSize;
        levelCellSize = mipmap1->cellSize;
    }
    else
    {
        maxX = cols;
        maxY = rows;
        cellArray = NULL;
        cell = NULL;
        stepSize = 1;
        levelCellSize = cellsize;
    }
    position = pose->getPosition();
    tempf = (vi->xMin - position[X]) / levelCellSize - 1;
    if (tempf > startx)
        startx = (int)tempf;
    tempf = (vi->yMin - position[Y]) / levelCellSize - 1;
    if (tempf > starty)
        starty = (int)tempf;
    tempf = (vi->xMax - position[X]) / levelCellSize + 1;
    if (tempf < maxX)
        maxX = (int)tempf;
    tempf = (vi->yMax - position[Y]) / levelCellSize + 1;
    if (tempf < maxY)
        maxY = (int)tempf;

    for (i = starty; i < maxY; i++)
    {
        for (j = startx; j < maxX; j++)
        {
            if (cellArray)
            {
                cell = &cellArray[i][j];
                cell2 = &mipmap2->cell[i][j];
//              if (cell->numPoints <= 0 || cell2->numPoints <= 0)
//                  continue;
            }

            //find center of cell
            xOffset = levelCellSize * (j + (stepSize - 1) / (2.0 * stepSize));
            yOffset = levelCellSize * (i + (stepSize - 1) / (2.0 * stepSize));

            //is it on the screen?
            worldPoint[X] = xOffset + position[X];
            worldPoint[Y] = yOffset + position[Y];
            worldPoint[Z] = position[Z];

            //get strength of cell
            if (cell)
            {
                cu = cell->u * (1 - p) + cell2->u * p;
                cv = cell->v * (1 - p) + cell2->v * p;
            }
            else
            {
                cu = u[t1][i][j] * (1 - p) + u[t2][i][j] * p;
                cv = v[t1][i][j] * (1 - p) + v[t2][i][j] * p;
            }
            length = sqrt(cu * cu + cv * cv);
            if (length < 0.06)
                continue;

            if (!view->pointIsOnScreen(worldPoint, 50, &depth) || depth > 0.9)
                continue;

            if (ltype == Current::As_Sqrt)
                lengthFactor = (0.5 + sqrt(length)) * (levelCellSize / ARROW_BOUNDARIES) * density;
            else
                lengthFactor = length * (levelCellSize / ARROW_BOUNDARIES) * density;
            if (length < 0.25 && depth < 0.6) //when too short but not too far away, draw some simple lines another layer down.
            {
                DrawMipmapVectors(i * commonMipmapFactor, j * commonMipmapFactor, mipmapLevel + 1, t1, t2, p, commonMipmapFactor, lengthFactor * ARROW_BOUNDARIES);
                if (length < 0.12f)
                    continue;
            }

            //create first panel
            //middle = ARROW_BOUNDARIES / 2;
            middle = 0;
            ax[middle] = xOffset - cu * 0.5f * lengthFactor;
            ay[middle] = yOffset - cv * 0.5f * lengthFactor;
            ax[middle+1] = xOffset + cu * 0.5f * lengthFactor;
            ay[middle+1] = yOffset + cv * 0.5f * lengthFactor;
            calcUV(au[middle], av[middle], ax[middle], ay[middle]);
            calcUV(au[middle + 1], av[middle + 1], ax[middle + 1], ay[middle + 1]);

            //create prior panels
            for (k = middle - 1; k >= 0; k--)
            {
                ax[k] = ax[k + 1] - au[k + 1] * lengthFactor;
                ay[k] = ay[k + 1] - av[k + 1] * lengthFactor;
                calcUV(au[k], av[k], ax[k], ay[k]);
            }

            //create later panels
            for (k = middle + 2; k < ARROW_BOUNDARIES; k++)
            {
                ax[k] = ax[k - 1] + au[k - 1] * lengthFactor;
                ay[k] = ay[k - 1] + av[k - 1] * lengthFactor;
                calcUV(au[k], av[k], ax[k], ay[k]);
            }

            //draw them
            glBegin(GL_QUAD_STRIP);
            for (k = 0; k < ARROW_BOUNDARIES; k++)
            {
                glColor3fv(arrowColor[k]);
                lz = getZ(ay[k], ax[k], t1, GL_FALSE) * (1.0 - p) + getZ(ay[k], ax[k], t2, GL_FALSE) * p - GetPosition(Z);
                //widthFactor = lengthFactor * (0.02 * k + 0.5f); // log(0.5 * k + 1.5f);
                //widthFactor = 0.2 * levelCellSize * (0.02 * k + 0.3f); // log(0.5 * k + 1.5f);
                widthFactor = 0.1f * levelCellSize * (0.02f * k + 0.3f);
                //swap u and -v to get perpendicular
                glVertex3f(ax[k] + av[k] * widthFactor, ay[k] - au[k] * widthFactor, lz);
                glVertex3f(ax[k] - av[k] * widthFactor, ay[k] + au[k] * widthFactor, lz);
            }
            glEnd();
            if (current->getShowLabels() && (length > 0.06))
            {
                winPosition = winPose->getPosition();
                if ((fabs(ax[0] - winPosition[X] + position[X]) < 0.5 / winScale) && (fabs(ay[0] - winPosition[Y] + position[Y]) < 0.5 / winScale))
                {
                    textz = lz + 0.005f / winScale;
                    glColor3f(0.0, 0.0, 0.0);
                    glBegin(GL_LINES);
                        glVertex3f(ax[0], ay[0], lz);
                        glVertex3f(ax[0], ay[0], textz);
                    glEnd();
                    glColor3f(1.0, 1.0, 1.0);
                    sprintf(vectorStrength, "%.2f", length * 1.944f); //convert to knots, assuming was m/s
                    win->printAt(ax[0], ay[0], textz, vectorStrength, 2, GLUT_BITMAP_HELVETICA_10, -8, -8);
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*/
/** Draws simple vectors from the mipmap (if it is a valid level or is
 *  numMipmapLevels, which indicates the base data should be used).  Nothing
 *  is drawn if an invalid mipmapLevel is given.
 */
void CurrentLayer::DrawMipmapVectors(int starti, int startj, int mipmapLevel, int t1, int t2, float p, int count, float size)
{
    float u1, v1, u2, v2, lu, lv, x, y, levelCellSize, lz;
    int endi, endj, max, i, j, stepSize;
    VectorMipmapLevel *mipmap1 = NULL, *mipmap2 = NULL;
    float _p = 1.0 - p;

    if (mipmapLevel > numMipmapLevels || mipmapLevel < 0)
        return;

    endi = starti + count;
    endj = startj + count;

    if (mipmapLevel == numMipmapLevels) //use base data
    {
        if (starti >= rows)
            starti = rows;
        if (endi >= rows)
            endi = rows;
        if (startj >= cols)
            startj = cols;
        if (endj >= cols)
            endj = cols;
        levelCellSize = cellsize;
        stepSize = 1;
    }
    else
    {
        mipmap1 = &vectorMipmap[t1][mipmapLevel];
        mipmap2 = &vectorMipmap[t2][mipmapLevel];
        max = mipmap1->maxDim;
        if (starti >= max)
            starti = max;
        if (endi >= max)
            endi = max;
        if (startj >= max)
            startj = max;
        if (endj >= max)
            endj = max;
        levelCellSize = mipmap1->cellSize;
        stepSize = mipmap1->stepSize;
    }

    for (i = starti; i < endi; i++)
    {
        for (j = startj; j < endj; j++)
        {
            //find center of cell
            x = levelCellSize * (j + (stepSize - 1) / (2.0 * stepSize));
            y = levelCellSize * (i + (stepSize - 1) / (2.0 * stepSize));

            //Calculate vector
            if (mipmapLevel == numMipmapLevels) //use base data
            {
                u1 = u[t1][i][j];
                u2 = u[t2][i][j];
                v1 = v[t1][i][j];
                v2 = v[t2][i][j];
            }
            else
            {
                u1 = mipmap1->cell[i][j].u;
                u2 = mipmap2->cell[i][j].u;
                v1 = mipmap1->cell[i][j].v;
                v2 = mipmap2->cell[i][j].v;
            }
            lu = u1 * _p + u2 * p;
            lv = v1 * _p + v2 * p;

            //calculate height
            lz = getZ(y, x, t1, GL_FALSE) * _p + getZ(y, x, t2, GL_FALSE) * p - GetPosition(Z);

            //draw the vector
            glBegin(GL_LINES);
            glColor3fv(tailColor);
            glVertex3f(x, y, lz);
            glColor3fv(headColor);
            glVertex3f(x + lu * size, y + lv * size, lz);
            glEnd();
        }
    }
}
/*---------------------------------------------------------------------------*/
void CurrentLayer::Idle(double time)
{
    int i,j;
    float p;
    int t2;

    currentTimestep = getTimestep(time);

    ObjectCollection::Idle(time);
    if(displayType == display_tex_arrows)
  {
        glDrawBuffer(GL_FRONT);
//      glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLineWidth(1.0);
        t2 = currentTimestep+1;
        if (t2>=timesteps)
            t2--;
        if(t2!=currentTimestep)
            p = (lastIdleTime-times[currentTimestep])/(times[t2]-times[currentTimestep]);
        else
            p = 1.0;
        glColor4d(0.0,0.0,0.0,1.0);
        glBegin(GL_LINES);
        for(i=0;i<rows;i++)
            for(j=0;j<cols;j++)
            {
                if( (u[currentTimestep][i][j]!=0) ||(v[currentTimestep][i][j]!=0))
                {
                    glColor3fv(tailColor);
                    glVertex2d(j,i);
                    glColor3fv(headColor);
                    glVertex2d(j+((u[currentTimestep][i][j]*(1-p)+u[t2][i][j]*(p)) *scale),
                               i+((v[currentTimestep][i][j]*(1-p)+v[t2][i][j]*(p)) *scale));
                }
            }
        glEnd();

//      glPointSize(2.0);
//      glBegin(GL_POINTS);
//      glColor3fv(headColor);
//      for(i=0;i<rows;i++)
//          for(j=0;j<cols;j++)
//          {
//              if( (u[currentTimestep][i][j]!=-0) ||(v[currentTimestep][i][j]!=0))
//              {
//                  glVertex2d(j+( (u[currentTimestep][i][j]*(1-p)+u[t2][i][j]*(p)) *scale),
//                             i+( (v[currentTimestep][i][j]*(1-p)+v[t2][i][j]*(p)) *scale));
//              }
//          }
//      glEnd();

        tex->Bind();
        glReadBuffer(GL_FRONT);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0,0,1024, 1024);
//      pb->bind();


    }

    
    // trigger redraw always
    globalInputMediator->Trigger(Scene3D::objectChangedEvent());

}

/*---------------------------------------------------------------------------*/
int CurrentLayer::perform(int argc, const char * * argv, Tcl_Interp * interp)
{
    if (argc >= 1)
    {
        if(strcmp(argv[0],"addFloater")==0)
        {
            pendingFloater = new Floater;
            addingManyFloaters = 0;
            return TCL_OK;
        }
        if(strcmp(argv[0],"addManyFloaters")==0)
        {
            pendingFloater = new Floater;
            addingManyFloaters = 1;
            return TCL_OK;
        }
        if(strcmp(argv[0],"addGroupOfFloaters")==0)
        {
            floaterGrid = 0;
            pendingFloaters = newFloaters();
            addingManyFloaters = 1;
            return TCL_OK;
        }
        if(strcmp(argv[0],"addGridOfFloaters")==0)
        {
            floaterGrid = 1;
            pendingFloaters = newFloaters();
            addingManyFloaters = 1;
            return TCL_OK;
        }
    }
    return ObjectCollection::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int CurrentLayer::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "addFloater addManyFloaters addGroupOfFloaters addGridOfFloaters ", NULL);
    return ObjectCollection::list(interp);
}

CurrentLayer::FloaterNodeType * CurrentLayer::newFloaters()
{
    int i,j;
    int sr; //square root
    
    FloaterNodeType * ret = new FloaterNodeType[NUM_FLOATERS];
    if(floaterGrid)
    {
        sr = (int)sqrt((float)NUM_FLOATERS);
        for(i=0;i<sr;i++)
            for(j=0;j<sr;j++)
            {
                ret[i*sr+j].floater = new Floater;
                ret[i*sr+j].x = -0.05+(i/(float)sr)*.10;
                ret[i*sr+j].y = -0.05+(j/(float)sr)*.10;
                
            }
    }
    else
    {
        for(i=0;i<NUM_FLOATERS;i++)
        {
            ret[i].floater = new Floater;
            ret[i].x = gauss(0.0,.01,time(NULL));
            ret[i].y = gauss(0.0,.01,0);
        }
    }
    return ret;
}

/*---------------------------------------------------------------------------*/
int CurrentLayer::HandleInput(unsigned int inputType, InputInfo * info)
{
    Window3D *window;
    gz4d::Point<double> mousePoint;
    gz4d::Point<double> position;
    ViewManager *currentView;
    gz4d::Point<double> eyePoint;
    double t;
    gz4d::Point<double> p;
    char l[15];
    int i;
    double scale;
    scale=mainWindow->getPoseInfo()->getScale();

    if (pendingFloater)
    {
        if (inputType & EnumeratedAction)
        {
            if (inputType == CLICK_EVENT && (((InputInfo*)info->val.p)->type & MouseButton1))
            {
                p[0] = GetPosition(0)+cursorLocation[0];
                p[1] = GetPosition(1)+cursorLocation[1];
                p[2] = 0.0;
                pendingFloater->SetPosition(p);
                sprintf(l,"Floater_%i",pendingFloater->GetId());
                pendingFloater->SetLabel(l);
                pendingFloater->setCurrent(this,lastIdleTime);
                pendingFloater->setTailLength(floaterTailLength);
                pendingFloater->respondTo(PICK_CLICK1 | PICK_PRESS3);
                AddObject(pendingFloater);
                mainWindow->getScene3D()->PostAddObject(pendingFloater);
                if(addingManyFloaters)
                {
                    pendingFloater = new Floater;
                }
                else
                {
                    pendingFloater = NULL;
                }
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                return SINK_EVENT;
            }
        }
        if ((inputType & MouseButtonDown)&&(inputType & MouseButton3))
        {
            delete pendingFloater;
            pendingFloater = NULL;
            return SINK_EVENT;
        }

        if (inputType & MouseMotion)
        {
            position[X] = (double)info->x.i;
            position[Y] = (double)info->y.i;
            position[Z] = 0.0;

            window = Window3D::cast(info->window);
            currentView = window->getViewInfo();

            if (currentView->get_ssstereo_flag() == 1)
                position[X] = position[X] + currentView->getViewport()[2]/2;

            currentView->getWorldPoint(position, mousePoint);

            eyePoint = currentView->getEyePoint();

            // extend the eye to mouse ray until the z is 0
            t = (-eyePoint[Z])/(mousePoint[Z]-eyePoint[Z]);
            cursorLocation[X] = (t*(mousePoint[X]-eyePoint[X]))+eyePoint[X];
            cursorLocation[Y] = (t*(mousePoint[Y]-eyePoint[Y]))+eyePoint[Y];
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        }
    }
    if (pendingFloaters)
    {
        if (inputType & EnumeratedAction)
        {
            if (inputType == CLICK_EVENT && (((InputInfo*)info->val.p)->type & MouseButton1))
            {
                p[0] = GetPosition(0)+cursorLocation[0];
                p[1] = GetPosition(1)+cursorLocation[1];
                p[2] = 0.0;
                for(i=0;i<NUM_FLOATERS;i++)
                {
                    p[0] = GetPosition(0)+cursorLocation[0]+pendingFloaters[i].x/scale;
                    p[1] = GetPosition(1)+cursorLocation[1]+pendingFloaters[i].y/scale;
                    pendingFloaters[i].floater->SetPosition(p);
                    sprintf(l,"Floater_%i",pendingFloaters[i].floater->GetId());
                    pendingFloaters[i].floater->SetLabel(l);
                    pendingFloaters[i].floater->setCurrent(this,lastIdleTime);
                    pendingFloaters[i].floater->setTailLength(floaterTailLength);
                                        pendingFloaters[i].floater->respondTo(PICK_CLICK1 | PICK_PRESS3);
                    AddObject(pendingFloaters[i].floater);
                    mainWindow->getScene3D()->PostAddObject(pendingFloaters[i].floater);
                }
                delete[] pendingFloaters;
                if(addingManyFloaters)
                {
                    pendingFloaters = newFloaters();
                }
                else
                {
                    pendingFloaters = NULL;
                }
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                return SINK_EVENT;
            }
        }
        if ((inputType & MouseButtonDown)&&(inputType & MouseButton3))
        {
            for(i=0;i<NUM_FLOATERS;i++)
            {
                delete pendingFloaters[i].floater;
            }
            delete[] pendingFloaters;
            pendingFloaters = NULL;
            return SINK_EVENT;
        }

        if (inputType & MouseMotion)
        {
            position[X] = (double)info->x.i;
            position[Y] = (double)info->y.i;
            position[Z] = 0.0;

            window = Window3D::cast(info->window);
            currentView = window->getViewInfo();

            if (currentView->get_ssstereo_flag() == 1)
                position[X] = position[X] + currentView->getViewport()[2]/2;

            currentView->getWorldPoint(position, mousePoint);

            eyePoint = currentView->getEyePoint();

            // extend the eye to mouse ray until the z is 0
            t = (-eyePoint[Z])/(mousePoint[Z]-eyePoint[Z]);
            cursorLocation[X] = (t*(mousePoint[X]-eyePoint[X]))+eyePoint[X];
            cursorLocation[Y] = (t*(mousePoint[Y]-eyePoint[Y]))+eyePoint[Y];
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        }
    }
    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
GLboolean CurrentLayer::getInterpolatedCurrent(float x, float y, double time, float &u, float &v)
{
    u = 0.0;
    v = 0.0;
    int t,w,s, tp, wp, sp;
    float tbias, tcomp;
    float xbias, xcomp;
    float ybias, ycomp;

    if(time > bbox.end || time < bbox.start)
    {
        return GL_FALSE;
    }

    for(t=0;(t<(timesteps-1))&&(times[t]<time);t++);
    if (t > 0)
        t--; //we went one too far! (added by mdp 5/24/2004)

    w =(int)floor((x-GetPosition(0))/cellsize);
    s =(int)floor((y-GetPosition(1))/cellsize);

    tp = t + 1;
    sp = s + 1;
    wp = w + 1;

    if((w < 0) || (w >= cols - 1) || (s < 0) || (s >= rows - 1))
        return GL_FALSE;

    tbias = (time-times[t])/(times[t+1]-times[t]);
    xbias = ((x-GetPosition(0))/cellsize)-floor((x-GetPosition(0))/cellsize);
    ybias = ((y-GetPosition(1))/cellsize)-floor((y-GetPosition(1))/cellsize);
    tcomp = 1 - tbias;
    xcomp = 1 - xbias;
    ycomp = 1 - ybias;

    u =  (CurrentLayer::u[tp][sp][wp]* tbias
        + CurrentLayer::u[t][sp][wp] * tcomp) * xbias * ybias
        +(CurrentLayer::u[tp][sp][w] * tbias
        + CurrentLayer::u[t][sp][w]  * tcomp) * xcomp * ybias
        +(CurrentLayer::u[tp][s][wp] * tbias
        + CurrentLayer::u[t][s][wp]  * tcomp) * xbias * ycomp
        +(CurrentLayer::u[tp][s][w]  * tbias
        + CurrentLayer::u[t][s][w]   * tcomp) * xcomp * ycomp;

    v =  (CurrentLayer::v[tp][sp][wp]* tbias
        + CurrentLayer::v[t][sp][wp] * tcomp) * xbias * ybias
        +(CurrentLayer::v[tp][sp][w] * tbias
        + CurrentLayer::v[t][sp][w]  * tcomp) * xcomp * ybias
        +(CurrentLayer::v[tp][s][wp] * tbias
        + CurrentLayer::v[t][s][wp]  * tcomp) * xbias * ycomp 
        +(CurrentLayer::v[tp][s][w]  * tbias
        + CurrentLayer::v[t][s][w]   * tcomp) * xcomp * ycomp;

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
Object * CurrentLayer::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    char sigmastr[100];
    std::string text;
    sigmaIndex = -1;
    int i;
    float p1, p2;

    Object::LoadXML(xml,path,ncfile);

    scale = xml.get("<xmlattr>.scale",current->getScale());
    sigmaIndex = xml.get("<xmlattr>.index",sigmaIndex);

    boost::optional<std::string> attribute;
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.headColor"))
    {
        std::stringstream ss(*attribute);
        ss >> headColor[0];
        ss >> headColor[1];
        ss >> headColor[2];
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.tailColor"))
    {
        std::stringstream ss(*attribute);
        ss >> tailColor[0];
        ss >> tailColor[1];
        ss >> tailColor[2];
    }
    
    spacing = xml.get("<xmlattr>.spacing",spacing);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.displayType"))
    {
        if(*attribute == "streaks")
            displayType = display_streaks;
        if(*attribute == "tex_arrows")
            displayType = display_tex_arrows;
    }

    lineWidth = xml.get("<xmlattr>.lineWidth",lineWidth);

    if(sigmaIndex >= 0)
    {   
        sigma = current->getSigma(sigmaIndex);
        sprintf(sigmastr,"Sigma_%f",sigma);
        SetLabel(sigmastr);
    }
    else
    {
        SetLabel("Layer");
    }
    

    for (i = 0; i < ARROW_BOUNDARIES; i++)
    {
        p1 = i / (float) ARROW_SEGMENTS;
        p2 = 1 - p1;
        arrowColor[i][0] = tailColor[0] * p2 + headColor[0] * p1;
        arrowColor[i][1] = tailColor[1] * p2 + headColor[1] * p1;
        arrowColor[i][2] = tailColor[2] * p2 + headColor[2] * p1;
    }

    if(loadFromNetCDF(ncfile))
    {
        return NULL;
    }
    else
    {
        createVectorMipmap(commonMipmapFactor);
        return this;
    }

}

/*---------------------------------------------------------------------------*/
int CurrentLayer::loadFromNetCDF(gz4d::netcdf::File ncfile)
{
    int i,j,k;
    Point3d gr;

    timesteps = ncfile.GetDim("time").Size();


    times = new double[timesteps];

    ncfile.GetVar("time").Get(times,timesteps);

    u = new float**[timesteps];
    v = new float**[timesteps];

    gr[0] = ncfile.GetAtt("localx").As<double>(0);
    gr[1] = ncfile.GetAtt("localy").As<double>(0);
    gr[2] = ncfile.GetAtt("localz").As<double>(0);

      // variable attribs may override global attribs
    gz4d::netcdf::Att localAttrib;


    for(i=0;i< ncfile.GetVar("u").NumAtts();i++)
    {
        localAttrib =ncfile.GetVar("u").GetAtt(i);
        if(localAttrib.Name() == "localx")
        {
            gr[0] = localAttrib.As<double>(0);
        }
        if(localAttrib.Name() == "localy")
        {
            gr[1] = localAttrib.As<double>(0);
        }
        if(localAttrib.Name() == "localz")
        {
            gr[2] = localAttrib.As<double>(0);
        }
        SetPosition(gr);
    }


    cellsize = ncfile.GetAtt("cellsize").As<float>(0);



    cols = ncfile.GetDim("col").Size();
    rows = ncfile.GetDim("row").Size();

    for(i=0;i<timesteps;i++)
    {
        u[i] = new float*[rows];
        v[i] = new float*[rows];
        for(j=0;j<rows;j++)
        {
            u[i][j] = new float[cols];
            v[i][j] = new float[cols];

            if(sigmaIndex >= 0)
            {
                ncfile.GetVar("u").SetCur(i,sigmaIndex,j,0);
                ncfile.GetVar("u").Get(u[i][j],1,1,1,cols);
                ncfile.GetVar("v").SetCur(i,sigmaIndex,j,0);
                ncfile.GetVar("v").Get(v[i][j],1,1,1,cols);
            }
            else
            {
                ncfile.GetVar("u").SetCur(i,j,0);
                ncfile.GetVar("u").Get(u[i][j],1,1,cols);
                ncfile.GetVar("v").SetCur(i,j,0);
                ncfile.GetVar("v").Get(v[i][j],1,1,cols);
            }
            for(k=0;k<cols;k++)
            {
                if(u[i][j][k] == -999.0)
                    u[i][j][k] = 0.0;
                if(v[i][j][k] == -999.0)
                    v[i][j][k] = 0.0;
            }
        }
    }


    if(!localBBox)
        localBBox = new BBox;

    localBBox->start = times[0];
    localBBox->end = times[timesteps-1];

    localBBox->w = 0;
    localBBox->e = cols*cellsize;
    localBBox->s = 0;
    localBBox->n = rows*cellsize;
    if(parent)
        parent->setBBoxStale();

  // create start points for streamlines on jittered grid.

  if(displayType == display_streaks)
  {
      if (spacing > cellsize)
            skipCells = (int)(spacing/cellsize);

      genPts(skipCells);

      deltaTime = (int)(times[1] - times[0]);

        for(i=0;i<nTraces;++i)
        {
            trace(i,(int)times[0]);
        }
    }

    // setup texture if necessary
    
    if(displayType == display_tex_arrows)
    {
        //TODO use frame buffer objects
        std::cerr << "Layer::LoadXML - configuring texture arrows\n";

        glViewport(0,0,1024,1024);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,cols,0,rows,-1,1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClearColor(1.0,1.0,1.0,0.5);

        tex = std::shared_ptr<ImageMap>(new ImageMap);
        tex->SetRef(GetPosition(0),GetPosition(1),(cellsize*cols)/1024.0 ,(cellsize*rows)/1024.0);
        tex->Build(); //added by mdp 3/20/2009 so I could take it out of SetRef
        tex->SetLabel("CurrentLayerGenerated");
        std::shared_ptr<TextureMap> t = tex;
        mainWindow->getScene3D()->AddTexture(t);
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::createVectorMipmap(int mipmapFactor)
{
    float cellsWide;
    VectorMipmapLevel *mipmapLevel;
    VectorMipmapCell *currentCell, **previousCells = NULL;
    float **uArray, **vArray, uTotal, vTotal;
    int t, level, x, y, maxDim, levelMaxDim, cellFactor;
    int xOffset, yOffset, i, j, xIndex, yIndex, previousMaxDim = 0;

    if (mipmapFactor < 2)
        return; //can't do less than 2!

    if (vectorMipmap)
        destroyVectorMipmap();

    maxDim = rows;
    if (cols > rows)
        maxDim = cols;
    cellsWide = (float)maxDim;
    if (cellsWide <= 1.0)
        return;

    for (numMipmapLevels = 0; cellsWide > 1.0; cellsWide /= mipmapFactor)
        numMipmapLevels++;

    //allocate mipmap
    vectorMipmap = new VectorMipmapLevel*[timesteps];
    for (t = 0; t < timesteps; t++)
    {
        cellFactor = 1;
        cellsWide = (float)maxDim;
        vectorMipmap[t] = new VectorMipmapLevel[numMipmapLevels];
        uArray = u[t];
        vArray = v[t];
        //perform averages on way up the hierarchy
        for (level = numMipmapLevels - 1; level >= 0; level--)
        {
            cellsWide /= mipmapFactor;
            cellFactor *= mipmapFactor;
            mipmapLevel = &vectorMipmap[t][level];
            mipmapLevel->cellSize = cellsize * cellFactor;
            mipmapLevel->maxDim = levelMaxDim = (int)ceil(cellsWide);
            mipmapLevel->cell = new VectorMipmapCell*[levelMaxDim];
            mipmapLevel->stepSize = cellFactor;
            for (y = 0; y < levelMaxDim; y++)
            {
                mipmapLevel->cell[y] = new VectorMipmapCell[levelMaxDim];
                for (x = 0; x < levelMaxDim; x++)
                {
                    currentCell = &mipmapLevel->cell[y][x];
                    xOffset = x * mipmapFactor;
                    yOffset = y * mipmapFactor;
                    uTotal = vTotal = 0.0f;
                    for (i = 0; i < mipmapFactor; i++)
                    {
                        yIndex = yOffset + i;
                        for (j = 0; j < mipmapFactor; j++)
                        {
                            xIndex = xOffset + j;
                            if (level == numMipmapLevels - 1) //get from original source
                            {
                                if (xIndex < cols && yIndex < rows)
                                {
                                    uTotal += uArray[yIndex][xIndex];
                                    vTotal += vArray[yIndex][xIndex];
                                }
                            }
                            else //get from previous mipmap level, don't need to worry about boundaries
                            {
                                if (xIndex < previousMaxDim && yIndex < previousMaxDim)
                                {
                                    uTotal += previousCells[yIndex][xIndex].u;
                                    vTotal += previousCells[yIndex][xIndex].v;
                                }
                            }
                        }
                    }
                    currentCell->u = uTotal / (mipmapFactor * mipmapFactor);
                    currentCell->v = vTotal / (mipmapFactor * mipmapFactor);
                    //currentCell->numPoints = 0;
                    //currentCell->arrowPoints = NULL;
                }
            }
            previousCells = mipmapLevel->cell;
            previousMaxDim = levelMaxDim;
        }

        //Create arrows on the way down the hierarchy
        //generateSimpleArrowMipmap(t, mipmapFactor);
        //generateMagnitudeBiasedMipmap(t, mipmapFactor);
    }
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::destroyVectorMipmap()
{
    int t, level, i, maxDim;
    //int j;
    VectorMipmapCell **cell;

    if (!vectorMipmap)
        return;

    for (t = 0; t < timesteps; t++)
    {
        for (level = 0; level < numMipmapLevels; level++)
        {
            cell = vectorMipmap[t][level].cell;
            maxDim = vectorMipmap[t][level].maxDim;
            for (i = 0; i < maxDim; i++)
            {
                //for (j = 0; j < maxDim; j++)
                //  delete [] cell[i][j].arrowPoints;
                delete [] cell[i];
            }
            delete [] cell;
        }
        delete [] vectorMipmap[t];
    }
    delete [] vectorMipmap;

    vectorMipmap = NULL;
}

/*---------------------------------------------------------------------------*/
/* Create simple arrows for the mipmap, one per cell
 */
void CurrentLayer::generateSimpleArrowMipmap(int t, int mipmapFactor)
{
/*
    float levelCellSize;
    VectorMipmapLevel *mipmapLevel;
    VectorMipmapCell *currentCell;//, **previousCells = NULL;
    float cu, cv, lengthFactor;
    int level, x, y, levelMaxDim;
    int xOffset, yOffset;//, previousMaxDim = 0;

    for (level = 0; level < numMipmapLevels; level++)
    {
        mipmapLevel = &vectorMipmap[t][level];
        levelMaxDim = mipmapLevel->maxDim;          
        levelCellSize = mipmapLevel->cellSize;
        for (y = 0; y < levelMaxDim; y++)
        {
            for (x = 0; x < levelMaxDim; x++)
            {
                currentCell = &mipmapLevel->cell[y][x];
                cu = currentCell->u;
                cv = currentCell->v;
                lengthFactor = sqrt(cu*cu + cv * cv);
                if (lengthFactor < 0.0001) //don't draw arrows when they are too small
                    continue;
                currentCell->numPoints = 2;
                currentCell->arrowPoints = new Point2f[2];
                xOffset = (int)(levelCellSize * (x + (mipmapFactor - 1) / (2.0 * mipmapFactor))); //this crazy little formula makes sure the start is at the center
                yOffset = (int)(levelCellSize * (y + (mipmapFactor - 1) / (2.0 * mipmapFactor))); //of the cell over which we did the averaging (n - 1)/(2 * n).
                cu *= levelCellSize; // * scale?
                cv *= levelCellSize; // * scale?
                currentCell->arrowPoints[0][X] = xOffset;
                currentCell->arrowPoints[0][Y] = yOffset;
                currentCell->arrowPoints[1][X] = xOffset + cu;
                currentCell->arrowPoints[1][Y] = yOffset + cv;
                currentCell->arrowHead[0][X] = xOffset + .73 * cu + .1 * cv;
                currentCell->arrowHead[0][Y] = yOffset - .1 * cu + .73 * cv;
                currentCell->arrowHead[1][X] = xOffset + cu;
                currentCell->arrowHead[1][Y] = yOffset + cv;
                currentCell->arrowHead[2][X] = xOffset + .73 * cu - .1 * cv;
                currentCell->arrowHead[2][Y] = yOffset + .1 * cu + .73 * cv;
            }
        }
    }
*/
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::generateMagnitudeBiasedMipmap(int t, int mipmapFactor)
{
    generateArrowFor(t, mipmapFactor, 0, 0, 0);
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::generateArrowFor(int t, int mipmapFactor, int level, int cellx, int celly)
{
/*
    VectorMipmapLevel *mipmapLevel = &vectorMipmap[t][level], *nextLevel = NULL;
    int i, j, xMax = 0, yMax = 0, nextx, nexty, nPoints, ix, iy, lineStep, maxRows, maxCols;
    float cu, cv, strength, maxStrength, dx, dy, length, levelCellSize;
    Point2f *array;
    float **uArray = u[t], **vArray = v[t];
    VectorMipmapCell *currentCell = &mipmapLevel->cell[celly][cellx], *childCell;
    GLboolean bottomLevel = GL_TRUE;
    GLboolean childCovered[50]; //this forces mipmapFactor <= 7
    struct pointNode {
        Point2f p, q;
        pointNode *next;
    } *nodeList, *lastNode, *tempNode;
    Point2f *points;

    //strategy: first, find strongest subcell, then make arrows for it, then make
    //arrows for me, then extend vector to any neighboring subcells while making
    //arrows for those subcells, then make arrows for any untouched subcells.
    nextx = cellx * mipmapFactor;
    nexty = celly * mipmapFactor;
    maxStrength = 0.0;
    if (level < numMipmapLevels - 1) //we're at a topmost or intermediate level
    {
        bottomLevel = GL_FALSE;
        nextLevel = &vectorMipmap[t][level + 1];
        maxRows = maxCols = nextLevel->maxDim;
    }
    else
    {
        maxRows = rows;
        maxCols = cols;
    }
    levelCellSize = mipmapLevel->cellSize;

    //Find strongest subcell
    for (i = 0; i < mipmapFactor; i++)
    {
        if (nexty + i >= maxRows)
            break;

        for (j = 0; j < mipmapFactor; j++)
        {
            if (nextx + j >= maxCols)
                break;

            childCovered[i * mipmapFactor + j] = GL_FALSE;
            if (bottomLevel)
            {
                cu = uArray[nexty + i][nextx + j];
                cv = vArray[nexty + i][nextx + j];
            }
            else
            {
                childCell = &nextLevel->cell[nexty + i][nextx + j];
                cu = childCell->u;
                cv = childCell->v;
            }
            strength = sqrt(cu * cu + cv * cv);
            if (strength > maxStrength)
            {
                maxStrength = strength;
                xMax = j;
                yMax = i;
            }
        }
    }

    if (bottomLevel)
    {
        if (maxStrength < 0.00000001) //don't do anything if they'd be too small!
            return;
    }
    else
    {
        if (maxStrength / nextLevel->cellSize < 0.00000001) //don't do anything if they'd be too small!
            return;
    }

    //Make arrows for it if it isn't a bottomLevel
    ix = nextx + xMax;
    iy = nexty + yMax;
    if (!bottomLevel)
        generateArrowFor(t, mipmapFactor, level + 1, ix, iy);
    childCovered[yMax * mipmapFactor + xMax] = GL_TRUE;

    //Make arrows for me
    nPoints = 0;
    lastNode = nodeList = new pointNode;
    nodeList->next = NULL;
    lineStep = mipmapFactor * 2;
    if (bottomLevel)
    {
        cu = cellsize * (ix);
        cv = cellsize * (iy);
        dx = uArray[iy][ix] * 0.5;
        dy = vArray[iy][ix] * 0.5;
        nodeList->p[X] = cu - dx * levelCellSize;
        nodeList->p[Y] = cv - dy * levelCellSize;
        nodeList->q[X] = cu + dx * levelCellSize;
        nodeList->q[Y] = cv + dy * levelCellSize;
        nPoints += 2;
    }
    else
    {
        childCell = &nextLevel->cell[iy][ix];
        points = childCell->arrowPoints;
        if (!points) //oops!
        {
            delete nodeList;
            return;
        }
        cu = nodeList->p[X] = points[0][X];
        cv = nodeList->p[Y] = points[0][Y];
        nPoints++;
        for (i = lineStep; i < childCell->numPoints; i += lineStep)
        {
            dx = (points[i][X] - cu);
            dy = (points[i][Y] - cv);
            length = levelCellSize * maxStrength / sqrt(dx * dx + dy * dy); //normalize to proper length
            lastNode->q[X] = cu + dx * length;
            lastNode->q[Y] = cv + dy * length;
            lastNode = lastNode->next = new pointNode;
            cu = lastNode->p[X] = points[i][X];
            cv = lastNode->p[Y] = points[i][Y];
            nPoints += 2;
        }
        // Finish last vector
        i = childCell->numPoints - 1;
        dx = (points[i][X] - cu);
        dy = (points[i][Y] - cv);
        length = levelCellSize * maxStrength / sqrt(dx * dx + dy * dy); //normalize to proper length
        lastNode->q[X] = cu + dx * length;
        lastNode->q[Y] = cv + dy * length;
        nPoints++;
    }

    //Extend my arrows to neighboring subcells to strongest subcell
    //extend accordig to cell size?

    //Pour list into array
    currentCell->numPoints = nPoints;
    array = currentCell->arrowPoints = new Point2f[nPoints];
    i = 0;
    while (nodeList)
    {
        array[i][X] = nodeList->p[X];
        array[i++][Y] = nodeList->p[Y];
        array[i][X] = nodeList->q[X];
        array[i++][Y] = nodeList->q[Y];
        tempNode = nodeList;
        nodeList = nodeList->next;
        delete tempNode;
    }

    //Make arrows for strongest subcell
    if (!bottomLevel)
    {
        for (i = 0; i < mipmapFactor; i++)
        {
            for (j = 0; j < mipmapFactor; j++)
            {
                if (childCovered[i * mipmapFactor + j])
                    continue;
                generateArrowFor(t, mipmapFactor, level + 1, nextx + j, nexty + i);
            }
        }
    }
*/
}

/*---------------------------------------------------------------------------*/
float CurrentLayer::getZ(int r, int c, int t)
{
    float depth;
    float tide;
    
    if(sigmaIndex < 0)
    {
        return current->getVSurfaceHeight(pose->getPosition()[X]+(c*cellsize),pose->getPosition()[Y]+(r*cellsize));
    }
    else
    {
        depth=current->getDepth(r,c);
        tide=current->getTide(r,c,t);
        return sigma*(depth-tide)+tide;
    }
}

/*---------------------------------------------------------------------------*/
float CurrentLayer::getZ(float i, float j, int t, GLboolean includesGeoRef)
{
    float lz = 0.0;
    int r;
    int c;

    if (includesGeoRef)
    {
        r = (int)((i-GetPosition(Y))/cellsize);
        c = (int)((j-GetPosition(X))/cellsize);
    }
    else
    {
        r = (int)(i / cellsize);
        c = (int)(j / cellsize);
    }

    if( r >= 0 && r < rows && c >= 0 && c < cols)
        if( (u[t][r][c]!=-999) &&(v[t][r][c]!=-999))
        {
            lz = getZ(r,c,t);
        }
    return lz;
}

/*---------------------------------------------------------------------------*/
float CurrentLayer::getZ(float i, float j, double t)
{
    
    return getZ(i,j,getTimestep(t));
}

/*---------------------------------------------------------------------------*/
int CurrentLayer::getTimestep(double t)
{
    int i;
    int currentTimestep = 0;
    for(i=0;i<timesteps;i++)
    {
        if (times[i] <= t)
        currentTimestep = i;
    }
    return currentTimestep;
}



/////////////////////
// The purpose of this method is to create a list of jittered start points over the area of the map.
// We start with every third cell.
void CurrentLayer::genPts(int sample)  // sample density
{
    int r,c,i;
    int count;

    count = 0;
    std::cerr << "COUNT   :  " << count << "\n";

    for (r=0;r<rows;r =r+sample)
        for (c=0;c<cols; c = c+sample)
        {
            if (u[0][r][c] !=-999) ++count;
        }

    nTraces = count;

    std::cerr << "NUMBER of Traces :  " << nTraces << "\n";


    XstartPos = new float[count];
    YstartPos = new float[count];
    ZstartPos = new float[count];


    // initialize the starte positions

    count = 0;
    for (r=0;r<rows;r =r+sample)
        for (c=0;c<cols; c = c+sample)
        {
            if (u[0][r][c] !=-999) 
            {
                
                // we jitter 80% of cellsize*sample to reduce aliasing
                XstartPos[count] = cellsize*(c + sample*(0.8*float(rand()%1000)/999.0 - 0.4));
                YstartPos[count] = cellsize*(r + sample*(0.8*float(rand()%1000)/999.0 - 0.4));
                
                if (XstartPos[count] < 0.0) XstartPos[count] = 0.0;
                if (YstartPos[count] < 0.0) YstartPos[count] = 0.0;
                 ++ count;
            }

        }

        
    // create the trace storage
    Xpaths = new float*[nTraces];
    Ypaths = new float*[nTraces];
    Zpaths = new float*[nTraces];
    
    for(i=0;i<nTraces;++i)
    {
        Xpaths[i] = new float[PATHLEN];
        Ypaths[i] = new float[PATHLEN];
        Zpaths[i] = new float[PATHLEN];
        
    }
    // initialize particles with random ages;
    age = new int[nTraces];
    for(i=0;i<nTraces;++i)
        age[i] = rand()%MAXAGE;


}

void CurrentLayer::trace(int s,int t)
{  // we compute a new path
    // s is the index of this particle
    // t is the time in seconds
    // we use interpolation in time, but not in space.
    int i, ix,iy;
    float xp, yp;
    float dx;
    float p;
    float dx1,dy1,dx2,dy2;
    int t1,t2, t0, ts;
    float interval;  // this is the interval used for advection distance
    // to get the trace lenght we multiply this by the streak length STRLEN
    float Zstart, Zend, dz;

    int tracetime;


    xp = XstartPos[s];
    yp = YstartPos[s];

    interval = 240;

    t0 = (int)times[0];

    // we set the start of the trace back in time

    tracetime = t - t0 - (int)interval*STRLEN;
    if (tracetime < 0) tracetime = 0;

    ts = tracetime/deltaTime;


    Zstart = getZ(int(yp/cellsize),int(xp/cellsize),ts);


    for (i = 0;i< PATHLEN;++i)   // note this can be optimized
    {   
        t1 = tracetime/deltaTime;

        if (t1 < timesteps -1) t2 = t1+1;
        else t2 = t1 = timesteps - 1;
    
                
        Xpaths[s][i] = xp;
        Ypaths[s][i] = yp;

    

        p = float(tracetime%deltaTime)/deltaTime;


        ix = int(xp/cellsize);
        iy = int(yp/cellsize);

        if (ix <= 0) ix = 0;
        if (iy <= 0) iy = 0;
        if (ix >= cols) ix = cols-1;
        if (iy >= rows) iy = rows-1;

        dx = u[t1][iy][ix];
        if (dx != -999)
        {
            dx1 = u[t1][iy][ix]*interval;  //* distance in interval seconds
            dy1 = v[t1][iy][ix]*interval;   

            dx2 = u[t2][iy][ix]*interval;  
            dy2 = v[t2][iy][ix]*interval;   
        }
        else
        {
            dx1 = 0;
            dy1 = 0;
            dx2 = 0;
            dy2 = 0;
        }


        xp = xp + (dx1*(1-p) + dx2*p)*100000000000.0;
        yp = yp + (dy1*(1-p) + dy2*p)*100000000000.0;
    

        tracetime += int(interval);  // advance 4 min

    
    }
    /*      // now we do linear interpolation to approximate the depths
            // this is done for speed, a better solution would be to get the interventing points
    */
    Zend = getZ(iy,ix,ts);
    

    dz = (Zend - Zstart)/float(PATHLEN);
    for (i = 0;i< PATHLEN;++i) 
    {
        Zpaths[s][i] = Zstart +dz*float(i) +1.0;  // note the +1 is a hack to lift up the points
    }



}

void CurrentLayer::drawStreaks()   // a complete set of traces
{
    int i,j,k;
    float pc;
    int a;
    float *tx, *ty, *tz;
    float re[STRLEN], gr[STRLEN], bl[STRLEN];

    // this is intended to be highly optimized
    //age = V.age;

    for(i=0;i<STRLEN;++i)
    {
        pc = float(i)/float(STRLEN-1);

        re[i] = tailColor[0]*(1.0-pc) + headColor[0]*pc;
        gr[i] = tailColor[1]*(1.0-pc) + headColor[1]*pc;
        bl[i] = tailColor[2]*(1.0-pc) + headColor[2]*pc;
    }

    for(i=0;i<nTraces;++i)
    {

        tx = Xpaths[i];
        ty = Ypaths[i];
        
        tz = Zpaths[i];


        glBegin(GL_LINE_STRIP);

//      float pc;

        a = age[i];
    //  f = fade[a];
        for (j = a;j< a + STRLEN;++j)   // note this can be optimized
        {   
            
            k = j-a;
            glColor3f(re[k],gr[k], bl[k]);
            glVertex3f(tx[j] , ty[j] ,tz[j]);
        }
        glEnd();

        ++age[i];

        if (age[i] > MAXAGE)  // we restart the particle
        {
            
            age[i] = 2;

            //lastIdleTime - this is the absolute time in seconds
              trace(i,(int)lastIdleTime);  // generates a path.
        }
        
    }

    //cerr << "width   " << lineWidth << "\n";
}



/*---------------------------------------------------------------------------*/
int CurrentLayer::getAttribute(SmartValue &val, int attributeIndex)
{
    switch (attributeIndex)
    {
        case Attr_FloaterTailLength:
            val.set(floaterTailLength);
            break;
        default:
            return Object::getAttribute(val,attributeIndex);
            break;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
int CurrentLayer::setAttribute(SmartValue &val, int attributeIndex)
{

    switch (attributeIndex)
    {
        case Attr_FloaterTailLength:
            floaterTailLength = val.asInteger();
            setChildrenFloaterTailLength();
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            break;
        default:
            return Object::setAttribute(val,attributeIndex);
            break;
    }
    return TCL_OK;
}

/*---------------------------------------------------------------------------*/
void CurrentLayer::setChildrenFloaterTailLength()
{
    Node * cursor = ObjectCollection::nodeList;

    while(cursor != NULL)
    {
        if(cursor->object->getRegistrationEntry()->className == "Floater")
        {
            ((Floater *)(cursor->object))->setTailLength(floaterTailLength);
        }
        cursor = cursor->next;
    }
    
}

GLboolean CurrentLayer::calcUV(float &newU, float &newV, float x, float y)
{
    return getInterpolatedCurrent(x + GetPosition(0), y + GetPosition(1), lastIdleTime, newU, newV);
}
