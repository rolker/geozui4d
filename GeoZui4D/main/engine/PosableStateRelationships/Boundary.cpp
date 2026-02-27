/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Boundary.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This class provides a way to detect the crossing of a 2D or 3D boundary
 *  by one or more PosableStateBase.  Position, orientation, and scale are
 *  fixed with respect to the boundary and do not update with respect to
 *  any of the managed PosableStateBases.
 *  This class implements the following responsibilities.
 *      * Managing PosableStateBases by collecting change events
 *        and triggering events when any one of them crosses the boundary.
 *      * Rendering to the screen (renders a wire or wire box, if visible)
 *      * Loading and saving from/to disk
 *
 *  Created by mdp 10/9/2002
 */
/*---------------------------------------------------------------------------*/

#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "Boundary.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
#include "engine/InputMediator.h"
#include "engine/gzCommandInterface.h"
#include "utilities/DottedAddress.h"
#include "objects/gutm.h"
#include "engine/PosableStateBase.h"

#define REGION_QUANTUM 5

REGISTER_OBJECT(Boundary, "boundary");

extern MainWindow *mainWindow;

const char *BoundaryNames[] = {COMPOSITE_OBJECT_NAMES, "SCALE", "HEIGHT_RATIO",
                                "YAW", "PITCH", "ROLL", ""};

const EventType *Boundary::boundaryCrossedInInputType =
                Interactor::newEventType("boundaryCrossedIn", Interactor::DataAsAddress);
const EventType *Boundary::boundaryCrossedOutInputType =
                Interactor::newEventType("boundaryCrossedOut", Interactor::DataAsAddress);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
Boundary::Boundary(void *creator)
: CompositeObject(creator, PICK_CLICK1 | PICK_PRESS3)
{
    // set up the reader to look for BEGIN AVERAGING_OVERVIEW 1.0,
    // and with set of keywords
    SetTypeAndVersion("BOUNDARY", 1, 0);
    SetKeywordTable(BoundaryNames);

    //default to all on.
    relationshipFromOperands = Translate;
    relationshipToResult = NoRelationship;

    //default to visible.
    visible = GL_TRUE;
    drawingHeight = 0.0;

    //empty the info.
    regionRecord = NULL;
    readingFile = GL_FALSE;
    sinPitch = sinYaw = 0.0;
    cosPitch = cosYaw = 1.0;
    numRegionRecords = 0;
    inWhenAbove = NULL;
    inWhenAboveNotFlag = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
Boundary::~Boundary()
{
}

/*---------------------------Interaction Facilitation------------------------*/
/*---------------------------------------------------------------------------*/
void Boundary::PoseChanged(PosableStateBase *whichPose,
                                    long whatChanged)
{
    int i;
    float val;

    if ((whatChanged & ChangeCurrentMask) == 0) //only care about "current"
        return;

    if (whichPose == pose)
    {
        Object::PoseChanged(whichPose, whatChanged);
        if (whatChanged & ChangeRotatePitchMask)
        {
            val = pose->getOrientationAxis()[Pitch];
            sinPitch = (float)sin(val * DEGTORAD);
            cosPitch = (float)cos(val * DEGTORAD);
        }
        if (whatChanged & ChangeRotateYawMask)
        {
            val = pose->getOrientationAxis()[Yaw];
            sinYaw = (float)sin(val * DEGTORAD);
            cosYaw = (float)cos(val * DEGTORAD);
        }
        evaluatePose();
    }
    else //evaluate the changed pose
    {
        if ((whatChanged & ChangeTranslateMask) == 0) //only care about translational motion
            return;

        for (i = 0; i < numOperands; i++)
        {
            if (poseOfOperands[i] == whichPose)
                break;
        }
        if (i >= numOperands) //wasn't one of ours!
            return;
        evaluatePose(i);
    }
}

/*---------------------------------------------------------------------------*/
/** This procedure evaluates the current region in which the pose at the
 *  passed index resides.  If the index is -1, all poses are evaluated.
 *  If the region of an object has negated, an event is triggered.
 */
void Boundary::evaluatePose(int index)
{
    int i, start, end, *newRecords, oldNumRecords, newVal;

    if (readingFile)
        return;

    if (index < 0)
    {
        start = 0;
        end = numOperands  -1;
    }
    else
        start = end = index;

    //reallocate space if now too small
    if (numOperands >= numRegionRecords)
    {
        if (regionRecord) //destroy and rebuild from scratch.
            delete [] regionRecord;

        oldNumRecords = numRegionRecords;
        while (numOperands >= numRegionRecords)
            numRegionRecords += REGION_QUANTUM;

        newRecords = new int[numRegionRecords];
        if (regionRecord)
        {
            for (i = 0; i < oldNumRecords; i++)
                newRecords[i] = regionRecord[i];
            for (; i < numRegionRecords; i++)
                regionRecord[i] = 0;

            delete regionRecord;
        }
        regionRecord = newRecords;
    }

    //calculate new region(s)
    for (i = start; i <= end; i++)
    {
        newVal = evaluatePose(poseOfOperands[i]);
        if (regionRecord[i] == -newVal)
        {
            if (newVal == 1)
                highLevelEventMediator->Trigger(boundaryCrossedOutInputType->value, pose, poseOfOperands[i]);
            else if (newVal == -1)
                highLevelEventMediator->Trigger(boundaryCrossedInInputType->value, pose, poseOfOperands[i]);
        }
        regionRecord[i] = newVal;
    }
}

/*---------------------------------------------------------------------------*/
/** This procedure evaluates the current region in which the passed pose
 *  resides.  On the "in" side, returns -1.  On the "Out" side, returns 1.
 */
int Boundary::evaluatePose(PosableStateBase *whichPose)
{
    gz4d::Point<double> position, myPosition;
    float x, y, z;
    int retval = 0;
    GLboolean above = GL_FALSE;

    //transform position into local position
    position = whichPose->getPosition();

    if (inWhenAbove)
    {
        z = inWhenAbove->GetHeightAt(position[X], position[Y]);
        if (z <= Gutm::invalidHeight)
            above = GL_FALSE;
        else
        {
            if (z >= position[Z])
                above = GL_FALSE;
            else
                above = GL_TRUE;
        }
        //only "in" if we are "in" here and "in" below.  Return immediately
        //if we are "out", continue only if we are "in".
        if (above == inWhenAboveNotFlag)
                return 1;
    }

    myPosition = pose->getPosition();
    //reverse our translation
    x = position[X] - myPosition[X];
    y = position[Y] - myPosition[Y];
    z = position[Z] - myPosition[Z];
    //reverse our heading, then our pitch (roll is immaterial)
    //the partial matrices look like this:
    // [ x ] [cosYaw -sinYaw 0 0]    [ x*cosYaw - y*sinYaw ]
    // [ y ] [sinYaw  cosYaw 0 0]    [ x*sinYaw + y*cosYaw ]
    // [ z ] [  0       0    1 0]  = [          z          ]
    // [ 1 ] [  0       0    0 1]    [          1          ]
    //
    //   [1     0         0    0]    [ x*cosYaw - y*sinYaw                       ]
    //   [0 cosPitch -sinPitch 0]    [(x*sinYaw + y*cosYaw)*cosPitch - z*sinPitch]
    // * [0 sinPitch cosPitch  0]  = [(x*sinYaw + y*cosYaw)*sinPitch - z*cosPitch]
    //   [0     0         0    1]    [          1                                ]
    //
    // and we're interested in seeing which side of zero
    // this final y-component is on.
    if (((x * sinYaw + y * cosYaw) * cosPitch - z * sinPitch) >= 0)
        retval = -1;
    else
        retval = 1;

    return retval;
}

/*---------------------------------------------------------------------------*/
long Boundary::manage(PosableStateBase *pose)
{
    long retval = CompositeObject::manage(pose);

    if (readingFile)
        return retval;

    if (numOperands >= numRegionRecords)
        evaluatePose();
    else
    {
        regionRecord[numOperands - 1] = 0;
        evaluatePose(numOperands - 1);
    }

    return retval;
}

/*-----------------------------Persistence-----------------------------------*/
/*---------------------------------------------------------------------------*/
int Boundary::processField(int option, char **fields)
{
    int i = 1;
    OrientationInfo orient;

    orient.type = OrientationInfo::typeYawPitchRoll;
    switch(option)
    {
        case SCALE:
            pose->setScale(ConvertFloat(i));
            break;
        case HEIGHT_RATIO:
            drawingHeight = ConvertFloat(i);
            break;
        case YAW:
            orient.axis[Yaw] = ConvertFloat(i);
            pose->setOrientation(orient, ChangeRotateYaw);
            break;          
        case PITCH:
            orient.axis[Pitch] = ConvertFloat(i);
            pose->setOrientation(orient, ChangeRotatePitch);
            break;          
        case ROLL:
            orient.axis[Roll] = ConvertFloat(i);
            pose->setOrientation(orient, ChangeRotateRoll);
            break;          
        default:
            return CompositeObject::processField(option, fields);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/** This readFrom procedure assumes that the proper number of operands has
 *  been allocated, and that (numOperandsAllocated - numOperands) records
 *  should be read from the file.
 */
int Boundary::readFrom(FILE *file)
{
    int retval;
    
    readingFile = GL_TRUE;

    retval = CompositeObject::readFrom(file);

    evaluatePose();
    readingFile = GL_FALSE;

    return retval;
}

/*---------------------------------------------------------------------------*/
int Boundary::writeTo(FILE *file)
{
    fprintf(file, "%s %d\n", BoundaryNames[N_RECORDS], numOperands);
    fprintf(file, "%s %f\n", BoundaryNames[SCALE], pose->getScale());
    fprintf(file, "%s %f\n", BoundaryNames[HEIGHT_RATIO], drawingHeight);
    fprintf(file, "%s %f\n", BoundaryNames[YAW], pose->getOrientationAxis(ChangeRotateYaw)[Yaw]);
    fprintf(file, "%s %f\n", BoundaryNames[PITCH], pose->getOrientationAxis(ChangeRotatePitch)[Pitch]);
    fprintf(file, "%s %f\n", BoundaryNames[ROLL], pose->getOrientationAxis(ChangeRotateRoll)[Roll]);
    return CompositeObject::writeTo(file);
}

/*---------------------------Rendering/Info----------------------------------*/
/*---------------------------------------------------------------------------*/
void Boundary::DrawLowRes(float degradation, Window3D *win)
{
    glNormal3f(0.0, 0.0, 1.0);
    glLineWidth(1.0);
    glDisable(GL_LIGHTING);
//    if(!renderingForPicking)
//      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 0.4f, 0.4f);
    glBegin(GL_QUADS);
        glVertex3f(0, 0, 0);
        glVertex3f(1, 0, 0);
        glVertex3f(1, 0, drawingHeight);
        glVertex3f(0, 0, drawingHeight);
    glEnd();
//    if(!renderingForPicking)
//      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
}

/*---------------------------------------------------------------------------*/
int Boundary::DrawHighRes(int eye, Window3D *win)
{
    DrawLowRes(0.0f, win);
    return 1;
}

/*---------------------------------------------------------------------------*/
int Boundary::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    char retval[128] = "";
    const char *arg;
    int status, nextArg;
    GLboolean notValue = GL_FALSE, badSyntax = GL_FALSE;
    Addressable *address = NULL;
    PosableStateBase *whichPose = NULL;
    Object *ob = NULL;

    if (argc >= 1)
    {
        if (!strcmp(argv[0], "heightRatio"))
        {
            if (argc >= 2)
                drawingHeight = (float)atof(argv[1]);
            sprintf(retval, "%f", drawingHeight);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "regionOf"))
        {
            if (argc >= 2)
            {
                status = gzCommandInterface::findPosable(argv[1], address, whichPose);
                if (status != TCL_OK)
                    return status;
            }
            else
            {
                Tcl_AppendResult(interp, "usage: regionOf <pose>", NULL);
                return TCL_ERROR;
            }
            if (evaluatePose(whichPose) == -1)
                Tcl_SetResult(interp, const_cast<char*>("in"), TCL_VOLATILE);
            else
                Tcl_SetResult(interp, const_cast<char*>("out"), TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "inWhen"))
        {
            if (argc >= 2)
            {
                nextArg = 1;
                arg = argv[nextArg++];
                if (!strcmp(arg, "not"))
                {
                    notValue = GL_TRUE;
                    arg = argv[nextArg++];
                    if (argc >= nextArg)
                        badSyntax = GL_TRUE;
                }
                if (!badSyntax && !strcmp(arg, "above"))
                {
                    if (nextArg < argc)
                    {
                        arg = argv[nextArg++];
                        if (arg[0] != '\0')
                        {
                            status = gzCommandInterface::findAddressIn(arg, ObjectNeighborhood, address, PosableNeighborhood);
                            if (status != TCL_OK)
                                return status;
                            DYNAMIC_CAST(whichPose, PosableStateBase *, address);
                            DYNAMIC_CAST(ob, Object*, whichPose->getUser());
                            if (!ob)
                            {
                                Tcl_AppendResult(interp, arg, " is not a valid Object.", NULL);
                                return TCL_ERROR;
                            }
                            inWhenAbove = ob;
                        }
                        else
                            inWhenAbove = NULL;
                        inWhenAboveNotFlag = notValue;
                    }
                    if (inWhenAbove)
                        Tcl_SetResult(interp, (char *)inWhenAbove->getAddress()->getAddress().c_str(), TCL_VOLATILE);
                    else
                        Tcl_SetResult(interp, const_cast<char*>(""), TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            badSyntax = GL_TRUE;
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "usage: inWhen [not] above [<object>]", NULL);
                return TCL_ERROR;
            }
        }
    }
    return CompositeObject::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Boundary::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "regionOf heightRatio inWhen ", NULL);
    return CompositeObject::list(interp);
}
