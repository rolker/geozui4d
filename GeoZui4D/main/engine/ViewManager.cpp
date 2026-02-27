/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ViewManager.cpp
// 
//  This class implements a viewpoint object for use in rendering GL scenes.
//  A ViewManager object encapsulates position, rotation, and scaling
//  information, relative to a particular reference point.
//
//  Created by mdp 7/20/2000
///////////////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include <math.h>
#include "engine/WindowPoseState.h"
#include "engine/ViewManager.h"
#include "widgets/Sv_Standards.h"
#include "widgets/Sv_Matrix4.h"
#include "engine/Window3D.h"
#include "interactors/WindowAnchor.h"
#include "PosableStateRelationships/WindowLock.h"
#include "engine/Scene3D.h"
#include "engine/Object.h"
#include "utilities/DottedAddress.h"
extern MainWindow *mainWindow;

#include "utilities/Quaternion.h"

#define CENTER_TO_NEAR_PLANE 0.25 
#define NEAR_PLANE 0.5
#define FAR_PLANE_DISTANCE 60000.0
#define MIN_FRUSTUM_SIZE 1.0

#define STEREO_OFFSET 0.0325 // m

GLboolean ViewManager::viewMovesWithWindow = GL_FALSE;
const EventType *ViewManager::viewChangedInputType =
                Interactor::newEventType("viewChanged", Interactor::DataAsLong);
const EventType *ViewManager::viewStabilizedInputType =
                Interactor::newEventType("viewStabilized");
const EventType *ViewManager::spaceWarpedInputType =
                Interactor::newEventType("spaceWarped");
GLfloat ViewManager::heightExaggeration = 1.0;

const GLdouble ViewManager::radiusOfWorkspaceCenter = CENTER_TO_NEAR_PLANE;

/*---------------------------------------------------------------------------*/
/** Constructor.  Performs a reset to initialize reference viewpoint to zero,
 *  initializes view parameters, and attaches to the width and height
 *  parameters of the managed window (the one whose view is encapsulated here).
 *
 *  @param  widthPtr    Pointer to the width variable of the managed window.
 *  @param  heightPtr
 *  @param  parentWin
 *  @param  ptr_ftVR_flag
 *  @param  ptr_ht_flag
 *  @param  ptr_s_flag
 *  @param  ptr_sss_flag
 */
ViewManager::ViewManager(const int *widthPtr, const int *heightPtr, Window3D *parentWin,
                         int * ptr_ftVR_flag, int * ptr_ht_flag, int * ptr_s_flag, int * ptr_sss_flag)
{
    Window3D *parentsParent;

    centerToNearPlaneOffset = CENTER_TO_NEAR_PLANE;
    nearPlane = NEAR_PLANE;
    distanceToFarPlane = FAR_PLANE_DISTANCE;
    minimumFrustumSize = MIN_FRUSTUM_SIZE;
    stereoOffset = STEREO_OFFSET;
    localHeightExaggeration = 1.0;
    usingLocalExaggeration = GL_FALSE;

    isotrack = NULL;

    perspectiveWidthRatio = 1.0f;
    perspectiveHeightRatio = 1.0f;
    perspectiveFlag = GL_TRUE;
    centerToEyepoint = nearPlane + centerToNearPlaneOffset;
    centerToScreen = centerToEyepoint - nearPlane;

    windowWidth = widthPtr;
    windowHeight = heightPtr;
    offX = offY = 0;
    parent = parentWin;

    ptr_fishtankVR_flag = ptr_ftVR_flag;
    ptr_headtracking_flag = ptr_ht_flag;
    ptr_stereo_flag = ptr_s_flag;
    ptr_ssstereo_flag = ptr_sss_flag;

    screenWidth_pix = NULL;
    screenHeight_pix = NULL;
    screenLeft = NULL;
    screenRight =  NULL;
    screenTop = NULL;
    screenBottom = NULL;

    tempVi = NULL;

    mainRight = NULL;

    maxOrientationChange = -1.0;
    oldHeading = 0.0f;
    
    pose = new WindowPoseState(this);
    perspectivePosable = new WindowPoseState(this, "perspective");
    perspectivePosable->setScale(1.5);
    parentsParent = parent->GetParent();
    if (parentsParent)
    {
        lockToParent = new WindowLock(this, pose);
        lockToParent->publishName("lockToParent");
        lockToParent->lock(PosableStateLock::LockBoth, parentsParent->getPoseInfo());
    }
    else
        lockToParent = NULL;
    lockToObject = new PosableStateLock(this);
    lockToObject->publishName("lockToObject");
    lockToObject->setDeleteThreshold(1);

    //and now the anchor representation for the parent window.
    anchor = new WindowAnchor(this, parentWin);
    referenceIsCurrent = false;

    reset();
}

/*---------------------------------------------------------------------------*/
/** Reset reference point and home position to 0's (scale to 1).
 */
void ViewManager::reset()
{
    pose->unsetPoseInfo();

    viewMatriciesValid = GL_FALSE;

    whichEye = GL_BACK;
    whichBuffer = GL_FRONT;

    if (lockToParent)
    {
        lockToParent->reset();
        lockToParent->lock(PosableStateLock::LockBoth);
    }
    unlockFromObject();
}

/*---------------------------------------------------------------------------*/
/** Destructor.
 */
ViewManager::~ViewManager()
{
    delete isotrack;
    delete perspectivePosable;

    /* Delete our representation */
    if (anchor)
        delete anchor;
    //lockToParent is deleted by virtue of our deleting our pose, which
    //happens due to being a PosableStateUser.
    if (lockToObject)
        delete lockToObject;
}

/*---------------------------------------------------------------------------*/
void ViewManager::PoseReplaced(PosableStateBase *oldPose,
                               PosableStateBase *newPose)
{
    parent->updatePosableState();
    anchor->UpdatePose();
}

/*----------------------------Basic Mutators---------------------------------*/
/*---------------------------------------------------------------------------*/
/** Copies the information from the right-hand ViewManager to this one.
 *  The home position for the new view manager becomes the current position.
 *
 *  @param  mgr         Right-hand ViewManager.
 */
ViewManager &ViewManager::operator =(const ViewManager &mgr)
{
    if (&mgr == this)
        return *this;

    pose->setPosition(mgr.pose->getPosition());
    pose->setOrientation(*mgr.pose->getOrientation());
    pose->setScale(mgr.pose->getScale());

    whichEye = GL_BACK;
    whichBuffer = GL_FRONT;

    return *this;
}

/*------------------------Viewing Frustum Operations-------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  Sets the monocular viewpoint.
 */
void ViewManager::applyFrustum(GLboolean fromScratch)
{
    setEye(whichEye, fromScratch);
}

/*---------------------------------------------------------------------------*/
void ViewManager::setDistanceToFarPlane(float distance)
{
    distanceToFarPlane = distance;
    globalInputMediator->Trigger(Window3D::windowChangedEvent());
}

/*---------------------------------------------------------------------------*/
void ViewManager::setPerspective(int pflag)
{
    perspectiveFlag = pflag;
    globalInputMediator->Trigger(Window3D::windowChangedEvent());
}

/*---------------------------------------------------------------------------*/
void ViewManager::setPerspectiveWidthRatio(float r)
{
    perspectiveWidthRatio = r;
    globalInputMediator->Trigger(Window3D::windowChangedEvent());
}

/*---------------------------------------------------------------------------*/
void ViewManager::setPerspectiveHeightRatio(float r)
{
    perspectiveHeightRatio = r;
    globalInputMediator->Trigger(Window3D::windowChangedEvent());
}


/*---------------------------------------------------------------------------*/
/**
 *  Sets the viewpoint for the passed side.
 */
void ViewManager::setEye(long gl_side, GLboolean fromScratch)
{
    GLdouble x;

    GLdouble y, z, q1, q2, q3, q4, temp;
    Quaternion quat, tempquat;
    Quaternion eye;

    if (*ptr_headtracking_flag == 1 )
    {
        //isotrack->GetPosition(2,&x,&y,&z,&q1,&q2,&q3,&q4); // reads sensor 2 (currently Fastrack)
        isotrack->GetPosition(1,&x,&y,&z,&q1,&q2,&q3,&q4); // reads sensor 1
        temp = z;
        z = x;
        //x = temp-34.5; // Portable FTVR stand, haptics
        x = temp-24.5; // Fixed FTVR table, haptics
        y = -y;

        //cm to m
        x/=100.0;
        y/=100.0;
        z/=100.0;

//      cout << "setEye, x: " << x << ", y: " << y << ", z:" << z << endl;

        quat.set(q1,q2,q3,q4);
            
        tempquat.setDegreeAngleVector(90,0.0,-1.0,0.0);
        //tempquat.setDegreeAngleVector(180,0.0,-1.0,0.0);
        quat = tempquat * quat;

        quat.y = -quat.y;
    }
    else
    {
        x = *eyeX + 0.10;
        y = *eyeY + 0.0;
        z = *eyeZ + 0.5;
        quat.set(1.0,0.0,0.0,0.0); // identity quat
    }

    if (whichEye != gl_side)
    {
        viewInfoCurrent = false;
        extraViewInfoCurrent = false;
        cornerInfoCurrent = GL_FALSE;
    }

    switch (whichEye = gl_side)
    {   
        case GL_FRONT_LEFT: case GL_BACK_LEFT:
            whichBuffer = GL_FRONT_LEFT;
            eye.set(0.0, -0.1-stereoOffset, 0.0, 0.0);
            if(*ptr_ssstereo_flag == 1)
            {
                glViewport(0, 0, *windowWidth/2, *windowHeight);
            }
            else
            {
                glViewport(0, 0, *windowWidth, *windowHeight);
            }
            break;
        case GL_FRONT_RIGHT: case GL_BACK_RIGHT:
            whichBuffer = GL_FRONT_RIGHT;
            eye.set(0.0, -0.1+stereoOffset, 0.0, 0.0);
            if(*ptr_ssstereo_flag == 1)
            {
                glViewport(*windowWidth/2, 0, *windowWidth/2, *windowHeight);
            }
            else
            {
                glViewport(0, 0, *windowWidth, *windowHeight);
            }
            break;
        case GL_COLOR_ATTACHMENT0_EXT:
            whichBuffer = GL_COLOR_ATTACHMENT0_EXT;
            eye.set(0.0, -0.1, 0.0, 0.0);
            glViewport(offX, offY, *windowWidth, *windowHeight);
            break;
        case GL_BACK: case GL_FRONT:
        default:
            whichBuffer = GL_FRONT;
            eye.set(0.0, -0.1, 0.0, 0.0);
            break;
    }

    //eye = eye.Rotate(quat);
    eye = quat.Rotate(eye);

    x += eye.x;
    y += eye.y;
    z += eye.z;

    setViewpoint(x, y, z, fromScratch);
}

/*---------------------------------------------------------------------------*/
/**
 *  This function resets the projection matrix, using the passed coordinate
 *  for the focus of the viewing space.
 */
void ViewManager::setViewpoint(GLdouble x, GLdouble y, GLdouble z, GLboolean fromScratch)
{
//  LogState ls(&gzlog,"ViewManager::setViewpoint",TRACE);
//    LogState ls(&gzlog,"ViewManager::setViewpoint",BOMBASTIC);
    //gzlog taken out of this method by mdp 4/17/2008 because the *windowWidth use caused a crash in GeoCoastPilot for some unknown reason.

    double l,r,b,t,n,f, hsw, hsh, widthRatio, heightRatio, screenWidth;
    float mainWidth, mainHeight;
    float xOffset, yOffset;
    double scale;//, screenOffset;
    float perspectiveScale;
    float ratio;
    mainWidth = mainHeight = xOffset = yOffset = 0;

    if (viewMovesWithWindow)
    {
        mainWidth = (float)*(mainWindow->getViewInfo()->windowWidth);
        mainHeight = (float)*(mainWindow->getViewInfo()->windowHeight);
        xOffset = parent->getX(GL_TRUE) + ((*windowWidth - mainWidth) / 2.0f);
        yOffset = parent->getY(GL_TRUE) + ((*windowHeight - mainHeight) / 2.0f);
    }

    if (*ptr_ssstereo_flag == 1 && *ptr_stereo_flag == 1)
    {
        ratio = (*windowWidth/(float)*windowHeight)/2.0f;
    }
    else
    {
        ratio = *windowWidth/(float)*windowHeight;
    }

    screenWidth = fabs(*screenRight - *screenLeft);

    widthRatio = (double)*windowWidth / (double)*screenWidth_pix;
    heightRatio = (double)*windowHeight / (double)*screenHeight_pix;
    
//    gzlog << "windowWidth: " << *windowWidth << endl;
//    gzlog << "windowHeight: " << *windowHeight << endl;

    hsw = (screenWidth / widthRatio) / (2.0);
    hsh = ((*screenTop-*screenBottom) / heightRatio) / (2.0);

    if (viewMovesWithWindow)
    {
        xOffset *= (float)(mainWidth / hsw);
        yOffset *= (float)(mainHeight/hsh);
        x -= xOffset;
        y -= yOffset;

        if (ratio > 1.0)
            z *= minimumFrustumSize / *windowHeight;
        else
            z *= minimumFrustumSize / *windowWidth;
    }

    if (*ptr_fishtankVR_flag == 1 )
    {
        l = hsw - x;    // left and right are switched for the mirror
        r = - hsw - x;
        b = - hsh - y;
        t = hsh - y;
        n = z;
        f = distanceToFarPlane + z;

        glMatrixMode(GL_PROJECTION);
        if (fromScratch)
            glLoadIdentity();

        if (perspectiveFlag)
            glFrustum(l, r, b, t, n, f);
        else
            glOrtho(l, r, b, t, n, f);
        glTranslated(-x, -y, -z - centerToNearPlaneOffset); // move origin to screen center
        glRotated(45.0,1.0,0.0,0.0);        // Rotate world to coincide with PHANToM's
        glMatrixMode(GL_MODELVIEW);
    }
    else
    {
        l = *screenLeft - x;
        r = *screenRight - x;
        t = *screenTop - y;
        b = *screenBottom - y;
        scale = widthRatio / heightRatio; //I'm not sure what's going on here, but at least it's more terse now(mdp)
        if (scale > 1.0)
        {
            l *= scale;
            r *= scale;
        }
        else
        {
            t /= scale;
            b /= scale;
        }

        n = z;
        f = distanceToFarPlane + z;

        glMatrixMode(GL_PROJECTION);
        if (fromScratch)
            glLoadIdentity();

        if (perspectiveFlag)
        {
            glFrustum(l * perspectiveWidthRatio, r * perspectiveWidthRatio, b * perspectiveHeightRatio, t * perspectiveHeightRatio, n, f);
//            gzlog << "Frustum: " << l * perspectiveWidthRatio  << ", " << r * perspectiveWidthRatio << ", " << b * perspectiveHeightRatio << ", " << t * perspectiveHeightRatio << endl;
            perspectiveScale = perspectivePosable->getScale();
            glScaled(perspectiveScale, perspectiveScale, perspectiveScale);
        }
        else
            glOrtho(l * perspectiveWidthRatio, r * perspectiveWidthRatio, b * perspectiveHeightRatio, t * perspectiveHeightRatio, n, f);
        glTranslated(-x, y, -z - centerToNearPlaneOffset);
        glMatrixMode(GL_MODELVIEW);
    }
};

/*---------------------------------------------------------------------------*/
gz4d::Point<double> const &ViewManager::getCorner(int corner)
{
    if (!cornerInfoCurrent)
        updateCornerInfo();

    return corners[corner];
}

/*---------------------------------------------------------------------------*/
void ViewManager::updateCornerInfo(void)
{
    gz4d::Point<double> screenPoint;
    int i, j, k, n, dim;
    double width, height, c1, c2, c3, val;
    gz4d::Point<double> vh, vd, vi, va, vf, vb;

    //project the corners of the frustum into the scene
    width = (double)getViewport()[2];
    height = (double)getViewport()[3];
    n = 0;
    for (i = LEFT_CORNERS; i <= RIGHT_CORNERS; i += RIGHT_CORNERS)
    {
        screenPoint[X] = (i ? width : 0.0);
        if(*ptr_ssstereo_flag == 1 && whichBuffer == GL_FRONT_RIGHT)
            screenPoint[X] += width;
        for (j = BOTTOM_CORNERS; j <= TOP_CORNERS; j += TOP_CORNERS)
        {
            screenPoint[Y] = (j ? 0.0 : height);
            for (k = FRONT_CORNERS; k <= BACK_CORNERS; k += BACK_CORNERS)
            {
                screenPoint[Z] = (k ? 1.0 : 0.0);
                getWorldPoint(screenPoint, corners[n]);
                if (n == 0)
                {
                    cornerMin = cornerMax = corners[n];
                }
                else
                {
                    for (dim = X; dim <= Z; dim++)
                    {
                        val = corners[n][dim];
                        if (val > cornerMax[dim])
                            cornerMax[dim] = val;
                        else if (val < cornerMin[dim])
                            cornerMin[dim] = val;
                    }
                }
                n++;
            }
        }
    }

    //calculate the z-component of the normal to the bottom plane
    //of the viewing frustum (used for clipping calculations)
    //The following calculation is taken from a more general
    //cross product between the bottom-back and bottom-left
    //edge of the frustum (pointing out of the frustum).
    //This is a simplification possible because we only need
    //the z-component.
    vi = corners[BACK_BOTTOM_LEFT];
    vh = corners[BACK_BOTTOM_RIGHT];
    vd = corners[FRONT_BOTTOM_LEFT];
    c1 = vh[Y] - vd[Y];
    c2 = vd[Y] - vi[Y];
    c3 = vi[Y] - vh[Y];
    zComponentOfUnderPlane = vi[X]*c1 + vh[X]*c2 + vd[X]*c3;

    //calculate the z-component of the normal to the top plane
    //of the viewing frustum in a similar manner
    vf = corners[BACK_TOP_LEFT];
    va = corners[FRONT_TOP_LEFT];
    vb = corners[FRONT_TOP_RIGHT];
    c1 = vb[Y] - vf[Y];
    c2 = va[Y] - vb[Y];
    c3 = vf[Y] - va[Y];
    zComponentOfAbovePlane = va[X]*c1 + vf[X]*c2 + vb[X]*c3;

    cornerInfoCurrent = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
/** If the plane at zBottom intersects the viewing frustum, fills the passed
 *  ViewIntersection (or creates a new one) with information regarding this
 *  intersection.  If they do not intersect, NULL is returned.
 */
ViewIntersection *ViewManager::getIntersectionAtPlaneZ(double zBottom, double tolerance, ViewIntersection *vi)
{
    return getIntersectionAtPlaneZ(zBottom, zBottom, tolerance, vi);
}

/*---------------------------------------------------------------------------*/
/** If any plane between zBottom and zTop intersects the viewing frustum, fills
 *  the passed ViewIntersection (or creates a new one) with information
 *  regarding this intersection.  If there is no intersection, NULL is returned.
 */
ViewIntersection *ViewManager::getIntersectionAtPlaneZ(double zBottom, double zTop, double tolerance, ViewIntersection *vi)
{
    GLboolean usingBothZs = GL_TRUE;
    int i, corner, bit, edge;
    GLdouble currentZ, t, minY, maxY, maybeY, minX, maxX, maybeX, zdiff;
    gz4d::Point<double> currentPoint, point1, point2;

    if (!vi)
    {
        if (!tempVi)
            tempVi = new ViewIntersection;
        vi = tempVi;
    }

    if (!cornerInfoCurrent)
        updateCornerInfo();

    //see if we're in the ballpark (assumes a pitch between 90 and -90, no roll)
    if (zBottom > cornerMax[Z] || zTop < cornerMin[Z])
        return NULL;

    tempVi = NULL; //make sure we don't reuse a vi that we're returning
    if (zBottom == zTop)
        usingBothZs = GL_FALSE;
    else
        vi->zTop = vi->zTop;

    //fill in the easy part
    vi->perspectiveFlag = perspectiveFlag;
    vi->usingBothZs = usingBothZs;
    currentZ = vi->zBottom = zBottom;

    //now see if we're fully enclosed between the two planes to save extra operations
    if (usingBothZs && zBottom < cornerMin[Z] && zTop > cornerMax[Z])
    {
        vi->yMin = cornerMin[Y];
        vi->yMax = cornerMax[Y];
        vi->xMin = cornerMin[X];
        vi->xMax = cornerMax[X];
        vi->fullyEnclosed = GL_TRUE;
        return vi;
    }

    vi->fullyEnclosed = GL_FALSE;

    //initialize some things;
    minY = maxY = gz4d::Nan<double>();
    minX = maxX = gz4d::Nan<double>();

    //calculate the plane intersections
    for (i = 0; i <= usingBothZs; i++)
    {
        edge = 0;
        //see the comments on the enum containing BOTTOM_LEFT
        //to understand how this loop system works.
        for (corner = FRONT_BOTTOM_LEFT; corner < BACK_TOP_RIGHT; corner++) //don't need <= because it's alread all ones.
        {
            for (bit = 1; bit <= 4; bit <<= 1)
            {
                if (corner & bit)
                    continue;
                point1 = corners[corner];
                point2 = corners[corner | bit];
                currentPoint = vi->edgeIntersect[i][edge];
                //find intersection of edge with z-plane
                zdiff = point2[Z] - point1[Z];
                if (zdiff < tolerance && zdiff > -tolerance)
                {
                    //just call it invalid.
                    vi->edgeValid[i][edge] = GL_FALSE;
                    continue;
                }
                else
                    t = (currentZ - point1[Z]) / zdiff;
                if (t < 0 || t > 1) //might really want (t < -tolerance / zdiff || t > 1 + tolerance / zdiff) to account for above bit of code
                {
                    vi->edgeValid[i][edge] = GL_FALSE;
                    continue;
                }
                currentPoint[X] = maybeX = (point2[X] - point1[X]) * t + point1[X];
                if (minX > maybeX || gz4d::IsNan(minX))
                    minX = maybeX;
                if (maxX < maybeX || gz4d::IsNan(maxX))
                    maxX = maybeX;
                currentPoint[Y] = maybeY = (point2[Y] - point1[Y]) * t + point1[Y];
                if (minY > maybeY || gz4d::IsNan(minY))
                    minY = maybeY;
                if (maxY < maybeY || gz4d::IsNan(maxY))
                    maxY = maybeY;
                vi->edgeValid[i][edge] = GL_TRUE;
                edge++;
            }
        }
        currentZ = zTop;
    }

    //now see if any corners between the two planes would change anything
    if (usingBothZs)
    {
        for (corner = FRONT_BOTTOM_LEFT; corner <= BACK_TOP_RIGHT; corner ++)
        {
            currentPoint = corners[corner];
            if (currentPoint[Z] >= zBottom && currentPoint[Z] <= zTop)
            {
                if (currentPoint[X] < minX)
                    minX = currentPoint[X];
                else if (currentPoint[X] > maxX)
                    maxX = currentPoint[X];
                if (currentPoint[Y] < minY)
                    minY = currentPoint[Y];
                else if (currentPoint[Y] > maxY)
                    maxY = currentPoint[Y];
            }
        }
    }

    //fill in the min/max's
    vi->yMin = minY;
    vi->yMax = maxY;
    vi->xMin = minX;
    vi->xMax = maxX;

    return vi;
}

/*---------------------------------------------------------------------------*/
GLboolean ViewManager::getIntersectionAtLineY(ViewIntersection *vi, double yLess, double yMore)
{
/*
    GLboolean usingBothYs = GL_TRUE, usingBothZs;
    double x1[2], ydiff, *point1, *point2, tolerance, t, currentY, minx, maxx;
    int i, j, p1, p2, xidx = 0;
    GLboolean retval = GL_FALSE;
*/
    return GL_FALSE;
/*
    if (isnan(yMore))
    {
        yMore = yLess;
        usingBothYs = GL_FALSE;
    }
    else
        vi->yMore = yMore;

    vi->usingBothYs = usingBothYs;
    vi->yLess = yLess;

    if (yLess > vi->yMax || yMore < vi->yMin)
        return GL_FALSE; //we already know we don't intersect there!
    if (vi->fullyEnclosed) //info is already correct!
        return GL_TRUE;
    
    usingBothZs = vi->usingBothZs;
    tolerance = vi->tolerance;

    //calculate the plane intersections
    for (i = 0; i <= usingBothZs; i++)
    {
        currentY = yLess;
        for (j = 0; j <= usingBothYs; j++)
        {
            for (p1 = BOTTOM_LEFT; p1 < TOP_RIGHT; p1++)
            {
                if (vi->edgeValid[i][p1] == GL_FALSE)
                    continue;
                point1 = vi->edgeIntersect[i][p1];
                for (p2 = p1 + 1; p2 <= TOP_RIGHT; p2++)
                {
                    if (vi->edgeValid[i][p2] == GL_FALSE)
                        continue;
                    point2 = vi->edgeIntersect[i][p2];
                    ydiff = point2[Y] - point1[Y];
                    if (ydiff < tolerance && ydiff > -tolerance)
                        continue; //assume no intersection
                    t = (currentY - point1[Y]) / ydiff;
                    x1[xidx++] = (point2[X] - point1[X]) * t + point1[X];
                    ...?
                }
            }
            currentY = yMore;
        }
    }
    
    return retval;
*/
}

/*---------------------------------------------------------------------------*/
double ViewManager::getZComponentOfUnderPlane()
{
    if (!cornerInfoCurrent)
        updateCornerInfo();

    return zComponentOfUnderPlane;
}

/*---------------------------------------------------------------------------*/
double ViewManager::getZComponentOfAbovePlane()
{
    if (!cornerInfoCurrent)
        updateCornerInfo();

    return zComponentOfAbovePlane;
}

/*---------------------------GL Transformations------------------------------*/
/*---------------------------------------------------------------------------*/
/** Applies the translation, rotation and scale information to the current
 *  modelview matrix (assumes tha modelview is current).  An initial flipping
 *  of the coordinate system is done, as well.
 */
void ViewManager::applyTransforms(GLboolean fromScratch)
{
    glMatrixMode(GL_PROJECTION);
    pose->applyTransforms();
    glScalef(1.0, 1.0, (usingLocalExaggeration? localHeightExaggeration : heightExaggeration));
    glMatrixMode(GL_MODELVIEW);
    if (fromScratch)
    {
        viewMatriciesValid = GL_TRUE;
        glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);
    }
    else
        viewMatriciesValid = GL_FALSE;

}


/*---------------------------------------------------------------------------*/
/** Pushes the frustum and modelview matrices onto the appropriate GL stacks.
 */
void ViewManager::pushView(GLboolean fromScratch)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    if (fromScratch)
        glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    applyFrustum(fromScratch);
    applyTransforms(fromScratch);
}

/*---------------------------------------------------------------------------*/
/** Pops the frustum and modelview matrices from the appropriate GL stacks.
 */
void ViewManager::popView()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    viewMatriciesValid = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** Sets the height exaggeration factor used by this view manager
 *  (and at this point, all view managers).
 */
void ViewManager::setHeightExaggeration(GLfloat heightEx)
{
    char command[48];

    if (usingLocalExaggeration)
    {
        setLocalHeightExaggeration(heightEx);
        return;
    }
    heightExaggeration = heightEx;
    sprintf(command, "set heightExaggeration %f", heightEx);
    parent->Tcl_Eval(command);
    globalInputMediator->Trigger(spaceWarpedInputType->value);
    globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
}

/*---------------------------------------------------------------------------*/
/** Sets the height exaggeration factor used by this view manager only.
 */
void ViewManager::setLocalHeightExaggeration(GLfloat heightEx)
{
    if (heightEx == 0.0f)
        usingLocalExaggeration = GL_FALSE;
    else
    {
        localHeightExaggeration = heightEx;
        usingLocalExaggeration = GL_TRUE;
    }
    viewUpdated();
    globalInputMediator->Trigger(Window3D::windowChangedEvent());
}

/*---------------------------------------------------------------------------*/
/** GetWindowPoint takes as input a screen position (x, y, z-bufferVal), and
 *  returns through winPosL the corresponding world-coordinate position relative
 *  to the position of the window.  If the z-bufferVal is outside of the 0 to 1
 *  range, then the actual value
 *  stored in the z-buffer at the coordinates (x, y) are used, and this value
 *  is passed back through the z coordinate of screenPos.  Returns GL_TRUE
 *  if successful, and GL_FALSE if unsuccessful.
 *
 *  @param  screenPos   Screen position (input)
 *  @param  winPosL Window-relative position (output)
 *  @return             GL_TRUE / GL_FALSE
 */
int ViewManager::getWindowPoint(gz4d::Point<double> &screenPos, gz4d::Point<float> &winPosL)
{
    GLfloat depth;
    GLdouble x, y;
    enum {X, Y, Z};
    int retval;
    double wx, wy, wz;

    if (!viewInfoCurrent)
        updateViewInfo();

    x = screenPos[X];
    y = viewport[3] - screenPos[Y];
    depth = (float)screenPos[Z];

    if (depth < 0.0 || depth > 1.0)
    {   
        pushView();
        glReadPixels((int)x, (int) y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        popView();

        //we will want to unproject offset by 0.5 since the depth buffer is in the center of the pixel
        x = ((int)x) + 0.5;
        y = ((int)y) + 0.5;

        if (depth >= 1.0)
            return GL_FALSE;

        screenPos[Z] = depth;
    }

    retval = gluUnProject(x, y, depth, modelMatrix, projectionMatrix, viewport, &wx, &wy, &wz);
    winPosL[X] = wx;
    winPosL[Y] = wy;
    winPosL[Z] = wz;
    return retval;
}

/*---------------------------------------------------------------------------*/
/** GetWorldPoint takes as input a screen position (x, y, z-bufferVal), and
 *  returns through worldPosL the corresponding world-coordinate position.  If
 *  the z-bufferVal is outside of the 0 to 1 range, then the actual value
 *  stored in the z-buffer at the coordinates (x, y) are used, and this value
 *  is passed back through the z coordinate of screenPos.  Returns GL_TRUE
 *  if successful, and GL_FALSE if unsuccessful.
 *
 *  @param  screenPos   Screen position (input)
 *  @param  worldPosL   World position (output)
 *  @return             GL_TRUE / GL_FALSE
 */
int ViewManager::getWorldPoint(gz4d::Point<double> &screenPos, gz4d::Point<double> &worldPosL)
{
    gz4d::Point<float> windowPos;

    if (getWindowPoint(screenPos, windowPos))
    {
        worldPosL = gz4d::Point<double>(windowPos) + pose->getPosition();
        return GL_TRUE;
    }
    return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
/** PlotWorldPoint takes as input a world position (x, y, z), and returns the
 *  corresponding screen position (x, y, z-bufferVal).  Returns GL_TRUE on
 *  success and GL_FALSE on failure.
 *
 *  @param  worldPos    World position (input)
 *  @param  screenPos   Screen position (output)
 *  @return             GL_TRUE / GL_FALSE
 */
int ViewManager::plotWorldPoint(gz4d::Point<double> const &worldPos, gz4d::Point<double> &screenPos)
{
    enum {X, Y, Z};
    int retval;
    Point3d p;

    if (!viewInfoCurrent)
        updateViewInfo();

    DIFF_POINT(worldPos,pose->getPosition(),p);

    retval = gluProject(p[X], p[Y], p[Z],
                        modelMatrix, projectionMatrix, viewport,
                        &(screenPos[X]), &(screenPos[Y]), &(screenPos[Z]));
    screenPos[Y] = viewport[3] - screenPos[Y];//transform y to 0-at-top.
    return retval;
}

/*---------------------------------------------------------------------------*/
/** pointIsOnScreen takes as input a world position (x, y, z), and returns
 *  GL_TRUE if the point is within bufferPixels pixels of being on the screen
 *  (returns GL_FALSE otherwise).
 */
GLboolean ViewManager::pointIsOnScreen(gz4d::Point<double> const &worldPos, int bufferPixels, GLdouble *depth)
{
    gz4d::Point<double> screenPos;
    int retval;

    retval = plotWorldPoint(worldPos, screenPos);
    if (retval)
    {
        if (screenPos[X] + bufferPixels < 0 || screenPos[X] - bufferPixels >= *windowWidth)
            return GL_FALSE;
        if (screenPos[Y] + bufferPixels < 0 || screenPos[Y] - bufferPixels >= *windowHeight)
            return GL_FALSE;
        if (depth)
            *depth = screenPos[Z];
    }
    return retval;
}

/*--------------------------Viewpoint Information----------------------------*/
/*---------------------------------------------------------------------------*/
/** This function updates the internal notion of the current projection,
 *  modelview, and viewport matrices, and updates the easily derived
 *  quantities of distanceToScreen and pixelSize.
 */
void ViewManager::updateViewInfo()
{
    int minDimension = *windowWidth;
    double scale = pose->getScale();
    GLdouble totalScale = scale;
    GLint tempViewport[4];
    double maxPixelSize;

    glGetIntegerv(GL_VIEWPORT, tempViewport);

    glViewport(0, 0, *windowWidth, *windowHeight);

    glReadBuffer(whichBuffer);
    if (!viewMatriciesValid)
    {
        pushView(); //make sure there is no junk.
        popView();  //return us to the previous state.
    }

    glViewport(tempViewport[0], tempViewport[1], tempViewport[2], tempViewport[3]);

    if (minDimension > *windowHeight)
        minDimension = *windowHeight;

    maxPixelSize = (fabs(*screenRight-*screenLeft))/(double)(*windowWidth);
    if(maxPixelSize < (*screenTop-*screenBottom)/(double)(*windowHeight))
        maxPixelSize = (*screenTop-*screenBottom)/(double)(*windowHeight);

    if (scale != 0.0)
    {
        //determine the distance of the eyePoint to the screen at the center.
        distanceToScreen = (nearPlane / scale);

        //determine the pixel size in world coordinates
        //pixelSize = (minimumFrustumSize * 2) / (totalScale * minDimension);
        pixelSize = maxPixelSize/totalScale;
        //cerr << "pixelSize: " << pixelSize << " scale: " << totalScale << endl;
    }

    viewInfoCurrent = true;
}

/*---------------------------------------------------------------------------*/
/** Returns the current projection matrix, updating the information if
 *  necessary and requested.  NULL is returned if updating is needed,
 *  but we are told not to update if necessary.
 *
 *  @return             Projection Matix / NULL.
 */
const GLdouble *ViewManager::getProjectionMatrix(GLboolean UpdateIfNecessary)
{
    if (!viewInfoCurrent)
    {
        if (UpdateIfNecessary)
            updateViewInfo();
        else
            return NULL;
    }
    return projectionMatrix;
}

/*---------------------------------------------------------------------------*/
/** Returns the current model matrix, updating the information if
 *  necessary and requested.  NULL is returned if updating is needed,
 *  but we are told not to update if necessary.
 *
 *  @return             Model Matix / NULL.
 */
const GLdouble *ViewManager::getModelMatrix(GLboolean UpdateIfNecessary)
{
    if (!viewInfoCurrent)
    {
        if (UpdateIfNecessary)
            updateViewInfo();
        else
            return NULL;
    }
    return modelMatrix;
}

/*---------------------------------------------------------------------------*/
/** Returns the current viewport, updating the information if
 *  necessary and requested.  NULL is returned if updating is needed,
 *  but we are told not to update if necessary.
 *
 *  @return             viewport / NULL.
 */
const GLint *ViewManager::getViewport(GLboolean UpdateIfNecessary)
{
    if (!viewInfoCurrent)
    {
        if (UpdateIfNecessary)
            updateViewInfo();
        else
            return NULL;
    }
    return viewport;
}

/*---------------------------------------------------------------------------*/
/** GetDistanceToScreen returns the current distance between the eyepoint
 *  and the center of the screen in world coordinates.  Updates the information
 *  first if necessary.
 *
 *  @return             Distance from eyepoint to screen center
 */
GLdouble ViewManager::getDistanceToScreen()
{
    if (!viewInfoCurrent)
        updateViewInfo();

    return distanceToScreen;
}

/*---------------------------------------------------------------------------*/
/** GetCenterToScreenDistance returns the current distance between the center
 *  of workspace and the center of the screen in world coordinates.  Updates
 *  the information first if necessary.
 *
 *  @return             Distance from workspace center to screen center
 */
GLdouble ViewManager::getCenterToScreenDistance()
{
    if (!viewInfoCurrent)
        updateViewInfo();

    return centerToScreen / pose->getScale();
}

/*---------------------------------------------------------------------------*/
/** GetCenterToEyepoint returns the current distance between the center
 *  of workspace and the eyepoint in world coordinates.  Updates
 *  the information first if necessary.
 *
 *  @return             Distance from eyepoint to workspace center
 */
GLdouble ViewManager::getCenterToEyepoint(void)
{
    if (!viewInfoCurrent)
        updateViewInfo();

    return centerToEyepoint / pose->getScale();
}


/*---------------------------------------------------------------------------*/
GLdouble ViewManager::getPixelSize()
{
    if (!viewInfoCurrent)
        updateViewInfo();

    return pixelSize;
}

/*---------------------------------------------------------------------------*/
GLdouble ViewManager::getPixelSizeAt(gz4d::Point<double> const &worldPos)
{
    GLdouble dotProduct = 0.0;
    int minDimension = *windowWidth;
    int i;

    if (perspectiveFlag)
    {
        getEyePoint(); //make sure we have all the info we need.
        if (minDimension > *windowHeight)
            minDimension = *windowHeight;

        //Transpose the screen and world coordinates by the eyepoint
        //so we can perform a proper dot product (we will be
        //normalizing afterward).  This loop composes the dot
        //product dimension by dimension.
        for (i = X; i <= Z; i++)
            dotProduct += (screenWorldPos[i] - eyePoint[i]) * (worldPos[i] - eyePoint[i]);

        //Normalize the dot product.
        dotProduct /= (distanceToScreen * distanceToScreen);

        return dotProduct * pixelSize  //why am I off by roughly a factor of 2 ??? (need to multiply here by 2+ to get a believable answer...)
                /* * minDimension / (2 * minimumFrustumSize + centerToNearPlaneOffset * .689)*/;
    }
    else
        return pixelSize;
}

/*---------------------------------------------------------------------------*/
/** UpdateExtraViewInfo updates the world coordinates of the eye point and
 *  screen center, updating the referencePoint if necessary.
 *  Returns GL_TRUE if successful, and GL_FALSE if unsuccessful.
 *
 *  @return             GL_TRUE / GL_FALSE
 */
int ViewManager::updateExtraViewInfo()
{
    GLdouble distanceFactor;
    float perspectiveScale;
    int retval, axis;
    enum {X, Y, Z, numAxes};

    if (!viewInfoCurrent)
        updateViewInfo();
    getReferencePoint(); //update the reference point, if not current.

    // get the world coordinates of the center projected to the screen plane.
    gz4d::Point<double> screenPos(*windowWidth/2.0, *windowHeight/2.0, 0.0);
    retval = getWorldPoint(screenPos, screenWorldPos);
    if (retval == GL_FALSE)
        return retval;

    perspectiveScale = perspectivePosable->getScale();
    //distanceFactor = 1 + (nearPlane / ((glTranslated-z-component) * perspectiveScale))
    //I'm not sure why the following is right, but it works for the frustum
    //values currently in place (mdp 7/21/2009)
    distanceFactor = 20.0 / (20.0 - (16.0 / perspectiveScale));

    for (axis = X; axis < numAxes; axis++)
    {   //connect the dots to get the eyePoint
        eyePoint[axis] = referencePoint[axis]
                       + distanceFactor*(screenWorldPos[axis] - referencePoint[axis]);
    }

    extraViewInfoCurrent = true;
    return retval;
}

/*---------------------------------------------------------------------------*/
gz4d::Point<double> const &ViewManager::getReferencePoint()
{
    if (!referenceIsCurrent)
    {
        referencePoint = pose->getPosition();
        referenceIsCurrent = true;
    }
    return referencePoint;
}

/*---------------------------------------------------------------------------*/
gz4d::Point<double> ViewManager::getScreenCenterPoint()
{
    if (!extraViewInfoCurrent)
    {
        if (updateExtraViewInfo() == GL_FALSE)
            return gz4d::Point<double>::Invalid();
    }
    return screenWorldPos;
}



gz4d::Point<double> ViewManager::getEyePoint()
{
    if (!extraViewInfoCurrent)
    {
        if (updateExtraViewInfo() == GL_FALSE)
            return gz4d::Point<double>::Invalid();
    }
    return eyePoint;
}

/*------------------------Rendering for Proxy--------------------------------*/
/*---------------------------------------------------------------------------*/
/**
 *  This function draws its proxy in the caller's context.
 */
void ViewManager::DrawIn(Window3D *inWindow)
{
    anchor->DrawIn(inWindow);
}

/*---------------------------------------------------------------------------*/
/**
 *  This function allows its proxy to be drawn for picking.
 */
void ViewManager::enablePicking()
{
    anchor->enablePick();
}

/*-----------------------Interaction Facilitation----------------------------*/
/*---------------------------------------------------------------------------*/
/** This function checks for view change events.
 */
int ViewManager::HandleInput(unsigned int inputType, InputInfo *info)
{
    if (inputType & EnumeratedAction)
    {
        if (inputType == spaceWarpedInputType->value)
            viewUpdated();
    }
/*
    if (inputType & KeyStroke)
    {
        if (info->val.i == '[')
        {
            angle[Z] += 10;
            Togl_PostRedisplay(info->window->GetTogl());
        }
        if (info->val.i == ']')
        {
            angle[Z] -= 10;
            Togl_PostRedisplay(info->window->GetTogl());
        }
    }
*/
    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/** This function allows outside classes to decide whether or not this view
 *  draws itself and allows itself to be picked.
 */
void ViewManager::setVisible(GLboolean vis)
{
    anchor->setVisible(vis);
}

/*---------------------------------------------------------------------------*/
/** This function allows outside classes to decide whether or not this view
 *  draws itself in a minimized form.
 */
void ViewManager::setMinimized(GLboolean state)
{
    anchor->setMinimized(state);
}

/*-----------------------View Locking Facilitation---------------------------*/
/*---------------------------------------------------------------------------*/
/** This function sets which object this view is locked to.
 */
void ViewManager::lockTo(Object *ob, long lockType)
{
    PosableStateBase *objectPose = ob->getPosable();

    unlockFromObject();
    lockToObject->manage(objectPose);
    if (lockToObject->getPoseOfTarget() == NULL)
        lockToObject->manage(pose);
    if (lockType != -1)
        lockToObject->lock(lockType);
    else if (gz4d::IsNan(objectPose->getOrientationAxis()[0]))
        lockToObject->lock(PosableStateLock::LockDeltaTranslate |
                           PosableStateLock::LockDeltaScale |
                           PosableStateLock::LockFromOwner);
    else
        lockToObject->lock(PosableStateLock::LockDeltaTranslate |
                           PosableStateLock::LockDeltaScale |
                           PosableStateLock::LockFromOwner |
                           PosableStateLock::LockLocalized |
                PosableStateLock::LockDynamic); //PosableStateLock::LockDeltaRotateY |
}

/*---------------------------------------------------------------------------*/
/** This function unlocks the object this view is locked to.
 */
void ViewManager::unlockFromObject()
{
    lockToObject->unlock(PosableStateLock::LockDeltaRotate |
                       PosableStateLock::LockDeltaTranslate |
                       PosableStateLock::LockFromOwner |
                       PosableStateLock::LockLocalized);
    lockToObject->relinquish(lockToObject->getPoseOfOwner());
}

/*---------------------------------------------------------------------------*/
PosableStateBase *ViewManager::getObjectLockedTo()
{
    return lockToObject->getPoseOfOwner();
}

/*---------------------------------------------------------------------------*/
/** This function sets which view this view is locked to.
 */
void ViewManager::lockTo(ViewManager *view)
{
    if (lockToParent)
        lockToParent->manage(view->getPosable());
}

/*---------------------------------------------------------------------------*/
/** This function sets up a new lock (of the passed type) on the current view.
 *  Locks and delta-locks are mutually exclusive, so this function removes the
 *  one if the other is set.  This function goes ahead and enforces the lock
 *  between the two views if it is a true lock (not a delta-lock).
 *
 *  @param  lockType        LockType enum value or composite to be set up
 */
void ViewManager::lock(long lockType)
{
    if (lockToParent)
        lockToParent->lock(lockType);
}

/*---------------------------------------------------------------------------*/
/** This function breaks an existing lock (of the passed type) on the current
 *  view.
 *
 *  @param  lockType        LockType enum value or composite to be removed
 */
void ViewManager::unlock(long lockType)
{
    if (lockToParent)
        lockToParent->unlock(lockType);
}

/*---------------------------------------------------------------------------*/
/** This function returns the current lock settings.
 */
long ViewManager::getLockType()
{
    if (lockToParent)
        return lockToParent->getType();
    else
        return 0;
}

/*---------------------------------------------------------------------------*/
void ViewManager::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    float tempMax, newOrientation, diff;
    GLboolean needsChange = GL_FALSE;

    if (whatChanged & ChangeTranslateMask)
        referenceIsCurrent = false;

    if (maxOrientationChange >= 0.0f && (whatChanged & ChangeRotateYawMask))
    {
        newOrientation = pose->getOrientation(ChangeRotateYaw)->axis[Y];
        diff = newOrientation - oldHeading;
        if (diff > maxOrientationChange)
        {
            newOrientation = oldHeading + maxOrientationChange;
            needsChange = GL_TRUE;
        }
        else if (diff < -maxOrientationChange)
        {
            newOrientation = oldHeading - maxOrientationChange;
            needsChange = GL_TRUE;
        }

        if (needsChange)
        {
            tempMax = maxOrientationChange;
            maxOrientationChange = -2.0;
            pose->setOrientation(0, newOrientation, ChangeRotateYaw);
            maxOrientationChange = tempMax;
        }
        oldHeading = newOrientation;
    }
    else if (maxOrientationChange == -2.0) //don't keep recursing, don't trigger multiple events
        return;
    
    if (whatChanged & ChangePose)
    {
        viewInfoCurrent = false;
        extraViewInfoCurrent = false;
        cornerInfoCurrent = GL_FALSE;
    }

    parent->inputMediator.Trigger(viewChangedInputType->value, this, (void*) whatChanged);
}

/*---------------------------------------------------------------------------*/
void ViewManager::PoseContinuouslyChanging(PosableStateBase *whichPose,
                                           GLboolean isChanging)
{
    parent->setPriorityInteractive();
    if (!isChanging && (whichPose == pose))
    {
        parent->restoreDefaultPriority();
        parent->inputMediator.Trigger(viewStabilizedInputType->value, this);
    }
}

/*---------------------------------------------------------------------------*/
/** This function allows other classes to force this view to take on attributes
 *  of its parent, or force the parent to take on attributes of this view.
 */
void ViewManager::setView(long lockType)
{
    Window3D *parentsParent;

    if (!lockToParent)
        return;

    if (lockType & PosableStateLock::LockFromOwner)
        lockToParent->handleChangeIn(pose, lockType, GL_TRUE);
    else
    {
        parentsParent = parent->GetParent();
        if (parentsParent)
            lockToParent->handleChangeIn(parentsParent->getPoseInfo(), lockType, GL_TRUE);
    }
    lockToParent->lock(lockToParent->getType());
}

/*---------------------------------------------------------------------------*/
/** This function allows other classes to force interaction with the anchor
 *  to cease in the passed window.
 */
void ViewManager::cancelAnchor(Window3D *inView)
{
    inView->inputMediator.RemoveInteractor(anchor);
}

void ViewManager::setMeasurments(int * x, int * y, double * l, double * r, double * t, double * b,int * mr)
{
    screenWidth_pix = x;
    screenHeight_pix = y;

    screenLeft = l;
    screenRight = r;
    screenTop = t;
    screenBottom = b;

    mainRight = mr;
}
