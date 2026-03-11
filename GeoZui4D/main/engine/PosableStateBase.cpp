// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// PosableStateBase.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This is the interface for classes that may have position, orientation, and
 *  scale properties.  Implementations of this interface are responsible for
 *  the following.  Default behaviors for most of these are provided.
 *      * Allowing for modification and retrieval of pose information
 *      * Applying proper transformation for display
 *      * Remembering a home position
 *      * Respond to queries about preferred poses, focus, and context
 *      * Provide hooks that allow other objects to respond to changes
 *        in this posable's pose
 *  This class actually implements the following responsibilities:
 *      * Retain a position
 *      * Provide a pose changed event
 *      * Retain a counter for being in flug (continuously changing)
 *
 *  Created by mdp 12/13/2000
 */
/*---------------------------------------------------------------------------*/
#include <iostream>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "engine/PosableStateBase.h"
#include "PosableStateUser.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "utilities/DottedAddress.h"
#include "engine/SessionRecorder.h"
#include "engine/InputMediator.h"
#include "utilities/SmartValue.h"
#include <stdlib.h>
#include <gz4d/util/Utilities.h>
#include <gz4d/vector/operations.h>

using namespace std;

Addressable PosableNeighborhoodAnchor;
DottedAddress *PosableNeighborhood = new DottedAddress(&PosableNeighborhoodAnchor, "Posable");
const EventType *PosableStateBase::posableDeletedInputType = Interactor::newEventType("posableDeleted", Interactor::DataAsAddress);
int PosableStateBase::supressRecordingRequest = 0;
GLboolean PosableStateBase::askingPointerForSet = GL_FALSE;


/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
/**
\param initialUser The intended intial user of this PoseableState. Defaults to NULL.
\param name The name to use to refer to this PoseState. Defaults to NULL.
*/
PosableStateBase::PosableStateBase(PosableStateUser *initialUser, string name)
{
    init();
    user = initialUser;
    serialNumber = relationshipManager->manage(this);

    stabilityThreshold = 0;

    if (!name.empty())
        publishName(name);
}

/*---------------------------------------------------------------------------*/
/**
\overload
\param toBeReplaced The PosableStateBase this will replace.
*/
PosableStateBase::PosableStateBase(PosableStateBase *toBeReplaced)
{
    init();
    serialNumber = toBeReplaced->getSerialNumber();
}

void PosableStateBase::init()
{
    continuouslyChanging = 0;
        accumulatingChanges = GL_FALSE;
        accumulatedChange = ChangeNone;
        minPixelsPerMeter = 0.0;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::replace(PosableStateBase *toBeReplaced)
{
    PosableStateInfo info;

    info.position[X] = 0.0;
    info.position[Y] = 0.0;
    info.position[Z] = 0.0;

    info.scale = 1.0;

    info.orientation.axis[Yaw] = 0.0;
    info.orientation.axis[Pitch] = 0.0;
    info.orientation.axis[Roll] = 0.0;

    user = NULL;
    toBeReplaced->getAddress()->substituteItem(this);
    toBeReplaced->getPoseInfo(info.position, info.orientation, info.scale);
    setPoseInfo(info);
    user = toBeReplaced->getUser();
    user->setPosable(this);
    user->PoseReplaced(toBeReplaced, this);
    relationshipManager->replace(toBeReplaced, this);
    delete toBeReplaced;
}

/*---------------------------------------------------------------------------*/
/** Publish refrence for scripting use using the passed name.
 */
void PosableStateBase::publishName(string newName)
{
    Addressable::publishName(newName);
}

/*---------------------------------------------------------------------------*/
PosableStateBase::~PosableStateBase()
{
    relationshipManager->relinquish(this);
    globalInputMediator->Trigger(posableDeletedInputType->value, this);
}

/*-----------------------------------Toolbox---------------------------------*/
/*---------------------------------------------------------------------------*/
/** This is an internal function that can be used by each derived class to
 *  take advantage of the versions of the functions written here (which
 *  no child necessarily has to use).
 */
PosableStateInfo *PosableStateBase::pointerForMask(long changeMask)
{
    askingPointerForSet = GL_FALSE; //reset the global flag
    return NULL;
}

/*---------------------------------------------------------------------------*/
int PosableStateBase::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    static SmartValue smartVal;
    char result[64];
    int index;
    char designator, colon;
    GLboolean badSyntax = GL_FALSE;

    colon = 0;

    if (argc >= 1)
    {
        if (strcmp(argv[0], "continuousChange") == 0)
        {
            char returnString[8];
            unsigned char val;

            if (argc >= 2)
            {
                if(atoi(argv[1]))
                    val = true;
                else
                    val = false;
                setContinuousChange(val);
            }
            val = isInContinuousChange();
            sprintf(returnString, "%d", val);
            Tcl_SetResult(interp, returnString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (strcmp(argv[0], "objectType") == 0)
        {
            if (user)
            {
                string name;
                GET_CLASS_NAME(user, name);
                Tcl_SetResult(interp, (char *)name.c_str(), TCL_VOLATILE);
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "attribute"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "count"))
                {
                    sprintf(result, "%d", getNumAttributes());
                    Tcl_SetResult(interp, result, TCL_VOLATILE);
                }
                else if (argc >= 3)
                {
                    if (!strcmp(argv[1], "panel"))
                    {
                        stringstream command;
                        command << "attributePanel " << getAddress()->getAddress() << " " << argv[2];
                        return Tcl_Eval(interp, command.str().c_str());
                    }
                    else if (!strcmp(argv[1], "index"))
                    {
                        sprintf(result, "%d", getAttributeIndex(argv[2]));
                        Tcl_SetResult(interp, result, TCL_VOLATILE);
                    }
                    else if (!strcmp(argv[1], "name"))
                        Tcl_SetResult(interp, (char *)getAttributeName(atoi(argv[2])).c_str(), TCL_VOLATILE);
                    else if (!strcmp(argv[1], "value"))
                    {
                        if (argv[2][0] >= '0' && argv[2][0] <= '9')
                            index = atoi(argv[2]);
                        else
                            index = getAttributeIndex(argv[2]);
                        if (argc >= 4)
                        {
                            if (strlen(argv[3]) > 2)
                            {
                                designator = argv[3][0];
                                colon = argv[3][1];
                                if (colon == ':')
                                {
                                    switch (designator)
                                    {
                                        case 'i': case 'I':
                                            smartVal.set(atoi(&argv[3][2]));
                                            break;
                                        case 'f': case 'F': case '%':
                                            smartVal.set((float)atof(&argv[3][2]));
                                            break;
                                        case 'p': case 'P': case '@':
                                            smartVal.set((void *)&argv[3][2]);
                                            break;
                                        case 's': case 'S': case '$':
                                            smartVal.set(&argv[3][2]);
                                            break;
                                        default: //invalid
                                            colon = '\0';
                                            break;
                                    }
                                }
                            }
                            if (colon != ':')
                                smartVal.set(argv[3]);

                            if (setAttribute(smartVal, index) != TCL_OK)
                            {
                                Tcl_AppendResult(interp, "Unable to set attribute \"", argv[2], "\".", NULL);
                                return TCL_ERROR;
                            }
                        }
                        else
                        {
                            if (getAttribute(smartVal, index) != TCL_OK)
                            {
                                Tcl_AppendResult(interp, "Unable to get attribute \"", argv[2], "\".", NULL);
                                return TCL_ERROR;
                            }
                            Tcl_SetResult(interp, (char *)smartVal.asString(), TCL_VOLATILE);
                        }
                    }
                    else
                        badSyntax = GL_TRUE;
                }
                else
                    badSyntax = GL_TRUE;
            }
            else
                badSyntax = GL_TRUE;

            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Incorrect usage.  Should be one of:\n",
                                 "attribute panel {show | hide | close}\n",
                                 "attribute index <name>\n",
                                 "attribute name <index>\n",
                                 "attribute count\n",
                                 "attribute value <index> [<value>]\n", NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        if (strcmp(argv[0], "list") == 0)
        {
            return list(interp);
        }
        //fall through
    }
    if (user)
        return user->perform(argc, argv, interp);
    else
        return TellableInterface::perform(argc, argv, interp);
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateBase::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "continuousChange objectType attribute ", NULL);
    if (user)
        return user->list(interp);
    else
        return Addressable::list(interp);
    return TCL_OK;
}

/*-----------------------------------Accessors-------------------------------*/
/*---------------------------------------------------------------------------*/
/** getPoseInfo returns position, orientation, and scale information back
 *  through the passed arguments.  If a class does not implement one of
 *  these attributes, it should not change the passed values.
 */
void PosableStateBase::getPoseInfo(gz4d::Point<double> &position, OrientationInfo &rot, double &scale, int changeMask)
{
    int i;
    PosableStateInfo *currentPose = NULL;

    if ((changeMask & ChangeTranslateMask) && (changeMask & ChangeCurrentMask))
    {
        currentPose = pointerForMask(changeMask & (ChangeTranslateMask | ChangeCurrentMask)); //done separate for TranslatableWrapper
        if (!currentPose)
            return;

        if (changeMask & ChangeTranslateXMask)
            position[X] = currentPose->position[X];
        if (changeMask & ChangeTranslateYMask)
            position[Y] = currentPose->position[Y];
        if (changeMask & ChangeTranslateZMask)
            position[Z] = currentPose->position[Z];
    }

    currentPose = pointerForMask(changeMask);
    if (!currentPose)
        return;

    if (changeMask & ChangeRotateMask)
    {
        rot.type = currentPose->orientation.type;
        if (rot.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotateYawMask)
                rot.axis[Yaw] = currentPose->orientation.axis[Yaw];
            if (changeMask & ChangeRotatePitchMask)
                rot.axis[Pitch] = currentPose->orientation.axis[Pitch];
            if (changeMask & ChangeRotateRollMask)
                rot.axis[Roll] = currentPose->orientation.axis[Roll];
        }
        else
        {
            for (i = X; i <= Z; i++)
                rot.axis[i] = currentPose->orientation.axis[i];
        }
        rot.angle = currentPose->orientation.angle;
    }

    if (changeMask & ChangeScaleMask)
        scale = currentPose->scale;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::getPoseInfo(PosableStateInfo &info, int changeMask)
{
    getPoseInfo(info.position, info.orientation, info.scale, changeMask);
}


/*---------------------------------------------------------------------------*/
/** getPosition returns a 3D position, if supported by this class, otherwise
 *  returns NULL.  This can be used to see if a particular position attribute
 *  is presently set, as well.
 */
gz4d::Point<double> PosableStateBase::getPosition(int changeMask)
{
    PosableStateInfo *currentPose;

    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeTranslateMask))
        return currentPose->position;

    return gz4d::Point<double>::Invalid();
}

/*---------------------------------------------------------------------------*/
/** getOrientationMagnitude returns the rotation magnitude information back
 *  for the specified axis, plane, or space of rotation.  If a class does not
 *  implement orientation, the return value is undefined.
 */
double PosableStateBase::getOrientationMagnitude(int changeMask)
{
    PosableStateInfo *currentPose;

    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
    {
        if (currentPose->orientation.type != OrientationInfo::typeYawPitchRoll)
            return currentPose->orientation.angle;
    }
    return 0.0;
}

/*---------------------------------------------------------------------------*/
/** getOrientationAxis returns a 3D orientation axis (or a vector of
 *  YawPitchRoll), if supported by this class, otherwise returns NULL.
 *  This can be used to see if a particular orientation attribute is
 *  presently set, as well.
 */
gz4d::Point<double> PosableStateBase::getOrientationAxis(int changeMask)
{
    PosableStateInfo *currentPose;

    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
        return currentPose->orientation.axis;

    return gz4d::Point<double>(gz4d::Nan<double>(),gz4d::Nan<double>(),gz4d::Nan<double>());
}

/*---------------------------------------------------------------------------*/
/** getOrientation returns a pointer to an Orientation, if supported by
 *  this class, otherwise returns NULL.  This can be used to see if a
 *  particular orientation attribute is presently set, as well.
 */
const OrientationInfo *PosableStateBase::getOrientation(int changeMask)
{
    PosableStateInfo *currentPose;

    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
        return &(currentPose->orientation);

    return NULL;
}

/*---------------------------------------------------------------------------*/
/** getScale returns a scalar scale value, if supported by this class, otherwise
 *  returns NULL.  This can be used to see if a particular orientation
 *  attribute is presently set.
 */
double PosableStateBase::getScale(int changeMask)
{
    PosableStateInfo *currentPose;

    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeScaleMask))
        return currentPose->scale;

    return 0;
}

/*-----------------------------Preference Accessors--------------------------*/
/*---------------------------------------------------------------------------*/
/** getFocusInfoUsing returns focus position, orientation, and scale
 *  information back through the passed arguments.  If a class does not
 *  implement one of these attributes, it should not change the passed
 *  values.  Values are only change if they are not within the acceptable
 *  focus range of the posable, and even then are changed only to the
 *  one of the closest acceptable set of focus values.
 */
void PosableStateBase::getFocusInfoUsing(gz4d::Point<double> &position, OrientationInfo &rot, double &scale)
{
}

/*---------------------------------------------------------------------------*/
/** getContextInfoUsing returns context position, orientation, and scale
 *  information back through the passed arguments.  If a class does not
 *  implement one of these attributes, it should not change the passed
 *  values.  Values are only change if they are not within the acceptable
 *  context range of the posable, and even then are changed only to the
 *  one of the closest acceptable set of context values.
 */
void PosableStateBase::getContextInfoUsing(gz4d::Point<double> &position, OrientationInfo &rot, double &scale)
{
}

/*------------------------------------Mutators-------------------------------*/
/*---------------------------------------------------------------------------*/
void PosableStateBase::setPosition(double valx, double valy, double valz, int changeMask)
{
    PosableStateInfo *currentPose;
    long whatChanged = changeMask & ChangeTypeMask;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose)
    {
        if (changeMask & ChangeTranslateXMask)
            if (currentPose->position[X] != valx)
            {
                whatChanged |= ChangeTranslateXMask;
                currentPose->position[X] = valx;
            }
        if (changeMask & ChangeTranslateYMask)
            if (currentPose->position[Y] != valy)
            {
                whatChanged |= ChangeTranslateYMask;
                currentPose->position[Y] = valy;
            }
        if (changeMask & ChangeTranslateZMask)
            if (currentPose->position[Z] != valz)
            {
                whatChanged |= ChangeTranslateZMask;
                currentPose->position[Z] = valz;
            }

        if (whatChanged & ChangeTranslateMask)
            NotifyPose(whatChanged);
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::setPosition(const double *val, int changeMask)
{
    setPosition(val[X], val[Y], val[Z], changeMask);
}

void PosableStateBase::setPosition(gz4d::Point<double> const &p, int changeMask)
{
    setPosition(p[0], p[1], p[2], changeMask);
}

void PosableStateBase::setX(double valx)
{
    setPosition(valx,0.0,0.0,ChangeTranslateXMask);
}

void PosableStateBase::setY(double valy)
{
    setPosition(0.0,valy,0.0,ChangeTranslateYMask);
}

void PosableStateBase::setZ(double valz)
{
    setPosition(0.0,0.0,valz,ChangeTranslateXMask);
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::setOrientation(const OrientationInfo &rot, int changeMask)
{
    int i;
    PosableStateInfo *currentPose;
    long whatChanged = changeMask & ChangeTypeMask;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
    {
        currentPose->orientation.type = rot.type;
        if (currentPose->orientation.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotatePitchMask)
                if (currentPose->orientation.axis[Pitch] != rot.axis[Pitch])
                {
                    whatChanged |= ChangeRotatePitchMask;
                    currentPose->orientation.axis[Pitch] = rot.axis[Pitch];
                }
            if (changeMask & ChangeRotateYawMask)
                if (currentPose->orientation.axis[Yaw] != rot.axis[Yaw])
                {
                    whatChanged |= ChangeRotateYawMask;
                    currentPose->orientation.axis[Yaw] = rot.axis[Yaw];
                }
            if (changeMask & ChangeRotateRollMask)
                if (currentPose->orientation.axis[Roll] != rot.axis[Roll])
                {
                    whatChanged |= ChangeRotateRollMask;
                    currentPose->orientation.axis[Roll] = rot.axis[Roll];
                }
        }
        else
        {
            for (i = X; i <= Z; i++)
                currentPose->orientation.axis[i] = rot.axis[i];
            whatChanged |= ChangeRotateMask;
        }
        currentPose->orientation.angle = rot.angle;

        if (whatChanged & ChangeRotateMask)
            NotifyPose(whatChanged);
        else if (changeMask & ChangeTypeMask)
            NotifyPose(changeMask);
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::setOrientation(double angx, double angy, int changeMask)
{
    PosableStateInfo *currentPose;
    long whatChanged = changeMask & ChangeTypeMask;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
    {
        if (currentPose->orientation.type != OrientationInfo::typeYawPitchRoll)
        {
            currentPose->orientation.type = OrientationInfo::typeYawPitchRoll;
            currentPose->orientation.axis[Roll] = 0.0;
        }
        if (changeMask & ChangeRotatePitchMask)
            if (currentPose->orientation.axis[Pitch] != angx)
            {
                whatChanged |= ChangeRotatePitchMask;
                currentPose->orientation.axis[Pitch] = angx;
            }
        if (changeMask & ChangeRotateYawMask)
            if (currentPose->orientation.axis[Yaw] != angy)
            {
                whatChanged |= ChangeRotateYawMask;
                currentPose->orientation.axis[Yaw] = angy;
            }

        if (whatChanged & ChangeRotateMask)
            NotifyPose(whatChanged);
        else if (changeMask & ChangeTypeMask)
            NotifyPose(changeMask);
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::setScale(double sval, int changeMask)
{
    PosableStateInfo *currentPose;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeScaleMask))
    {
        currentPose->scale = sval;

        NotifyPose(ChangeScaleMask | (changeMask & ChangeTypeMask));
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::moveBy(double valx, double valy, double valz, int changeMask)
{
    PosableStateInfo *currentPose;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose)
    {
        if (changeMask & ChangeTranslateXMask)
            currentPose->position[X] += valx;
        if (changeMask & ChangeTranslateYMask)
            currentPose->position[Y] += valy;
        if (changeMask & ChangeTranslateZMask)
            currentPose->position[Z] += valz;

        NotifyPose((changeMask & ChangeTranslateMask) |
                   (changeMask & ChangeTypeMask));
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::moveBy(gz4d::Point<double> const &val, int changeMask)
{
    moveBy(val[X], val[Y], val[Z], changeMask);
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::rotateBy(const OrientationInfo &rot, int changeMask)
{
    PosableStateInfo *currentPose;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
    {
        if (currentPose->orientation.type != rot.type)
            return;

        if (currentPose->orientation.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotatePitchMask)
                currentPose->orientation.axis[Pitch] += rot.axis[Pitch];
            if (changeMask & ChangeRotateYawMask)
                currentPose->orientation.axis[Yaw] += rot.axis[Yaw];
            if (changeMask & ChangeRotateRollMask)
                currentPose->orientation.axis[Roll] += rot.axis[Roll];
        }
        else
        {
            assert(0);//not implemented
        }

        NotifyPose((changeMask & ChangeRotateMask) |
                   (changeMask & ChangeTypeMask));
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::rotateBy(double angx, double angy, double angz, int changeMask)
{
    PosableStateInfo *currentPose;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);

    if (currentPose && (changeMask & ChangeRotateMask))
    {
        if (currentPose->orientation.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotatePitchMask)
                currentPose->orientation.axis[Pitch] += angx;
            if (changeMask & ChangeRotateYawMask)
                currentPose->orientation.axis[Yaw] += angy;
            if (changeMask & ChangeRotateRollMask)
                currentPose->orientation.axis[Roll] += angz;
        }
        else
        {
            assert(0);//not implemented
        }

        NotifyPose((changeMask & ChangeRotateMask) |
                   (changeMask & ChangeTypeMask));
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::scaleBy(double sval, int changeMask)
{
    PosableStateInfo *currentPose;

    askingPointerForSet = GL_TRUE;
    currentPose = pointerForMask(changeMask);
    if (currentPose && (changeMask & ChangeScaleMask))
    {
        currentPose->scale *= sval;

        NotifyPose((changeMask & ChangeScale) |
                   (changeMask & ChangeTypeMask));
    }
}

/*---------------------------------------------------------------------------*/
/*
Posable &PosableStateBase::operator =(const Posable &posable)
{
}
*/

/*-----------------------------Interaction Facilitation----------------------*/
/*---------------------------------------------------------------------------*/
void PosableStateBase::setContinuousChange(unsigned char changing)
{
    if (changing)
    {
        if (user && continuouslyChanging <= stabilityThreshold)
            user->PoseContinuouslyChanging(this);
        continuouslyChanging++;
    }
    else
    {
        continuouslyChanging--;

        if (user && continuouslyChanging == stabilityThreshold)
            user->PoseContinuouslyChanging(this, GL_FALSE);
        else if (continuouslyChanging < 0)
            continuouslyChanging = 0;
    }

}

/*---------------------------------------------------------------------------*/
unsigned char PosableStateBase::isInContinuousChange()
{
    return (continuouslyChanging > 0);
}

/*---------------------------------------------------------------------------*/
/**
Tell our user and relations that we changed.
\param changeMask What changed.
*/
void PosableStateBase::NotifyPose(long changeMask)
{
    if (changeMask == ChangeNone)
        return;

    if (accumulatingChanges)
        accumulatedChange |= changeMask;
    else
    {
        if (user)
            user->PoseChanged(this, changeMask);
        relationshipManager->poseChanged(this, changeMask);
        if (sessionRecorder->getRecordingState() == SessionRecorder::Recording
            && supressRecordingRequest <= 0
            && (changeMask & ChangeAnyMask) && getAddress() && !getAddress()->getAddress().empty())
        {
            char command[1024], arguments[1024], arg[32];
            int currentPos;
            gz4d::Point<double> info;

            arguments[0] = '\0';
            sprintf(command, "gz posable set %s ", &(getAddress()->getAddress()[9]));
            currentPos = strlen(command);
            if (changeMask & ChangeHomeMask)
                command[currentPos++] = 'H';
            else if (changeMask & ChangeMinMask)
                command[currentPos++] = 'm';
            else if (changeMask & ChangeMaxMask)
                command[currentPos++] = 'M';
            else if (changeMask & ChangePreferredMask)
                command[currentPos++] = 'P';
            if (changeMask & ChangeScaleMask)
            {
                command[currentPos++] = 's';
                sprintf(arg, " %f", getScale(changeMask));
                strcat(arguments, arg);
            }
            info = getPosition(changeMask);
            if (info.IsValid())
            {
                if (changeMask & ChangeTranslateXMask)
                {
                    command[currentPos++] = 'x';
                    sprintf(arg, " %f", info[X]);
                    strcat(arguments, arg);
                }
                if (changeMask & ChangeTranslateYMask)
                {
                    command[currentPos++] = 'y';
                    sprintf(arg, " %f", info[Y]);
                    strcat(arguments, arg);
                }
                if (changeMask & ChangeTranslateZMask)
                {
                    command[currentPos++] = 'z';
                    sprintf(arg, " %f", info[Z]);
                    strcat(arguments, arg);
                }
            }
            gz4d::Point<double> axis = getOrientationAxis(changeMask);
            if (!gz4d::IsNan(axis[0]))
            {
                if (changeMask & ChangeRotatePitchMask)
                {
                    command[currentPos++] = 'p';
                    sprintf(arg, " %f", axis[Pitch]);
                    strcat(arguments, arg);
                }
                if (changeMask & ChangeRotateYawMask)
                {
                    command[currentPos++] = 'h';
                    sprintf(arg, " %f", axis[Yaw]);
                    strcat(arguments, arg);
                }
                if (changeMask & ChangeRotateRollMask)
                {
                    command[currentPos++] = 'r';
                    sprintf(arg, " %f", axis[Roll]);
                    strcat(arguments, arg);
                }
            }
            command[currentPos++] = '\0';
            strcat(command, arguments);
            sessionRecorder->recordCommand(command);
        }
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::AccumulateNotifications()
{
    accumulatingChanges = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::FlushNotifications()
{
    if (accumulatingChanges && accumulatedChange != ChangeNone)
    {
        accumulatingChanges = GL_FALSE;
        NotifyPose(accumulatedChange);
        accumulatingChanges = GL_TRUE;
        accumulatedChange = ChangeNone;
    }
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::PurgeNotifications()
{
    accumulatedChange = ChangeNone;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::ResumeSingularNotification(bool purge)
{
    if (purge)
        PurgeNotifications();
    else
        FlushNotifications();
    accumulatingChanges = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
long PosableStateBase::getSerialNumber() const
{
    return serialNumber;
}

/*--------------------------Attribute Accessors/Mutators---------------------*/
/*---------------------------------------------------------------------------*/
int PosableStateBase::getNumAttributes()
{
    if (user)
        return user->getNumAttributes();
    else
        return 0;
}

/*---------------------------------------------------------------------------*/
int PosableStateBase::getAttributeIndex(string attributeName)
{
    if (user)
        return user->getAttributeIndex(attributeName);
    else
        return -1;
}

/*---------------------------------------------------------------------------*/
string PosableStateBase::getAttributeName(int attributeIndex)
{
    if (user)
        return user->getAttributeName(attributeIndex);
    else
        return "";
}

/*---------------------------------------------------------------------------*/
int PosableStateBase::getAttribute(SmartValue &val, int attributeIndex)
{
    if (user)
        return user->getAttribute(val, attributeIndex);
    else
        return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
int PosableStateBase::setAttribute(SmartValue &val, int attributeIndex)
{
    if (user)
        return user->setAttribute(val, attributeIndex);
    else
        return TCL_ERROR;
}

/*-----------------------------Generalized Interface-------------------------*/
/*---------------------------------------------------------------------------*/
/** getPoseInfo returns position, orientation, and scale information.  It
 *  calls the normal getPoseInfo function to gather the information it
 *  returns.
 */
PosableStateInfo PosableStateBase::getPoseInfo(long changeMask)
{
    PosableStateInfo info;

    getPoseInfo(info.position, info.orientation, info.scale, changeMask);

    return info;
}

/*---------------------------------------------------------------------------*/
/** setPoseInfo sets position, orientation, and/or scale information, depending
 *  on the passed changeMask.  This function calls the various setting
 *  functions of this class to set the information specified in the changeMask.
 *  Change notifications are suppressed until the end.
 */
void PosableStateBase::setPoseInfo(const PosableStateInfo &state, long changeMask)
{
    AccumulateNotifications();

    if (changeMask & ChangeTranslateMask)
        setPosition(state.position[X], state.position[Y], state.position[Z], changeMask);
    if (changeMask & ChangeRotateMask)
        setOrientation(state.orientation, changeMask);
    if (changeMask & ChangeScaleMask)
        setScale(state.scale, changeMask);

    ResumeSingularNotification();
}

/*---------------------------------------------------------------------------*/
/** setPoseInfoFromTo sets the position, orientation, and/or scale information
 *  in the second argument's category (or in the current category, if no 2nd
 *  argument given) from the information in the first argument's category.
 *  Only info specified in both arguments is changed (defaults to all in
 *  the second argument).  This function calls the various getting/setting
 *  functions of this class to set the information specified in the changeMask.
 *  Change notifications are suppressed until the end.
 */
void PosableStateBase::setPoseInfoFromTo(long fromMask, long toMask)
{
    long crossMask = fromMask & toMask;
    long fromType = fromMask & ChangeTypeMask;
    long toType = toMask & ChangeTypeMask;

    //If it's the same category, we don't need to do anything.
    if (fromType == toType)
        return;

    //redesign the masks.
    fromMask = fromType | crossMask;
    toMask = toType | crossMask;

    //Do the assignment
    AccumulateNotifications();

    if (crossMask & ChangeTranslateMask)
        setPosition(getPosition(fromMask), toMask);
    if (crossMask & ChangeRotateMask)
        setOrientation(*getOrientation(fromMask), toMask);
    if (crossMask & ChangeScaleMask)
        setScale(getScale(fromMask), toMask);

    ResumeSingularNotification();

}

/*---------------------------------------------------------------------------*/
/** setPoseInfo changes position, orientation, and/or scale information,
 *  depending on the passed changeMask.  This function calls the various *By
 *  functions of this class to change the information specified in the
 *  changeMask. Change notifications are suppressed until the end.
 */
void PosableStateBase::offsetPoseInfoBy(const PosableStateInfo &state, long changeMask)
{
    AccumulateNotifications();

    if (changeMask & ChangeTranslateMask)
        moveBy(state.position, changeMask);
    if (changeMask & ChangeRotateMask)
        rotateBy(state.orientation, changeMask);
    if (changeMask & ChangeScaleMask)
        scaleBy(state.scale, changeMask);

    ResumeSingularNotification();
}

/*---------------------------------------------------------------------------*/
/** unsetPoseInfo resets position, orientation, and/or scale information,
 *  depending on the passed changeMask, if supported.  Reseting using
 *  ChangeAll (the default) actually resets all values, including the
 *  min, max, and preferred values for the selected attributes (position,
 *  orientation, scale).
 */
void PosableStateBase::unsetPoseInfo(long changeMask)
{
}

/*---------------------------------------------------------------------------*/
/** getFocusInfoUsing returns focus position, orientation, and scale
 *  information back through the passed argument.  If a class does not
 *  implement one of these attributes, it should not change the passed
 *  values.  Values are only change if they are not within the acceptable
 *  focus range of the posable, and even then are changed only to the
 *  one of the closest acceptable set of focus values.  This function is
 *  implemented by calling the normal getFocusInfoUsing.
 */
void PosableStateBase::getFocusInfoUsing(PosableStateInfo &stateInfo)
{
    getFocusInfoUsing(stateInfo.position, stateInfo.orientation, stateInfo.scale);
}

/*---------------------------------------------------------------------------*/
/** getContextInfoUsing returns context position, orientation, and scale
 *  information back through the passed arguments.  If a class does not
 *  implement one of these attributes, it should not change the passed
 *  values.  Values are only change if they are not within the acceptable
 *  context range of the posable, and even then are changed only to the
 *  one of the closest acceptable set of context values.  This function is
 *  implemented by calling the normal getContextInfoUsing.
 */
void PosableStateBase::getContextInfoUsing(PosableStateInfo &stateInfo)
{
    getContextInfoUsing(stateInfo.position, stateInfo.orientation, stateInfo.scale);
}

/*
    //Turn highlighting on/off
    virtual void highlight(unsigned char state);

    //Is the position information editable?
    virtual unsigned char isEditable(long changeMask);
*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void PosableStateBase::incrementPoseInfo(PosableStateInfo &accum,
                                         const PosableStateInfo &addend,
                                         long changeMask)
{
    int i;

    if (changeMask & ChangeTranslateXMask)
        accum.position[X] += addend.position[X];
    if (changeMask & ChangeTranslateYMask)
        accum.position[Y] += addend.position[Y];
    if (changeMask & ChangeTranslateZMask)
        accum.position[Z] += addend.position[Z];

    if ((changeMask & ChangeRotateMask) &&
        accum.orientation.type == addend.orientation.type)
    {
        if (accum.orientation.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotateYawMask)
                accum.orientation.axis[Yaw] += addend.orientation.axis[Yaw];
            if (changeMask & ChangeRotatePitchMask)
                accum.orientation.axis[Pitch] += addend.orientation.axis[Pitch];
            if (changeMask & ChangeRotateRollMask)
                accum.orientation.axis[Roll] += addend.orientation.axis[Roll];
        }
        else //&&&this isn't really right...
        {
            for (i = X; i <= Z; i++)
                accum.orientation.axis[i] += addend.orientation.axis[i];
        }
        accum.orientation.angle += addend.orientation.angle;
    }

    if (changeMask & ChangeScaleMask)
        accum.scale *= addend.scale;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::decrementPoseInfo(PosableStateInfo &accum, const PosableStateInfo &addend, long changeMask)
{
    int i;

    if (changeMask & ChangeTranslateXMask)
        accum.position[X] -= addend.position[X];
    if (changeMask & ChangeTranslateYMask)
        accum.position[Y] -= addend.position[Y];
    if (changeMask & ChangeTranslateZMask)
        accum.position[Z] -= addend.position[Z];

    if ((changeMask & ChangeRotateMask) &&
        accum.orientation.type == addend.orientation.type)
    {
        if (accum.orientation.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotateYawMask)
                accum.orientation.axis[Yaw] -= addend.orientation.axis[Yaw];
            if (changeMask & ChangeRotatePitchMask)
                accum.orientation.axis[Pitch] -= addend.orientation.axis[Pitch];
            if (changeMask & ChangeRotateRollMask)
                accum.orientation.axis[Roll] -= addend.orientation.axis[Roll];
        }
        else //&&&this isn't really right...
        {
            for (i = X; i <= Z; i++)
                accum.orientation.axis[i] -= addend.orientation.axis[i];
        }
        accum.orientation.angle -= addend.orientation.angle;
    }

    if (changeMask & ChangeScaleMask)
        accum.scale /= addend.scale;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::multiplyPoseInfo(PosableStateInfo &accum, float factor,
                                        long changeMask)
{
    int i;

    if (changeMask & ChangeTranslateXMask)
        accum.position[X] *= factor;
    if (changeMask & ChangeTranslateYMask)
        accum.position[Y] *= factor;
    if (changeMask & ChangeTranslateZMask)
        accum.position[Z] *= factor;

    if (changeMask & ChangeRotateMask)
    {
        if (accum.orientation.type == OrientationInfo::typeYawPitchRoll)
        {
            if (changeMask & ChangeRotateYawMask)
                accum.orientation.axis[Yaw] *= factor;
            if (changeMask & ChangeRotatePitchMask)
                accum.orientation.axis[Pitch] *= factor;
            if (changeMask & ChangeRotateRollMask)
                accum.orientation.axis[Roll] *= factor;
        }
        else //&&&this isn't really right...
        {
            for (i = X; i <= Z; i++)
                accum.orientation.axis[i] *= factor;
        }
        accum.orientation.angle *= factor;
    }

    if (changeMask & ChangeScaleMask)
        accum.scale *= factor;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::vectorToAnglePoseInfo(PosableStateInfo &result, const PosableStateInfo &vect)
{
    float x, y, z, d;

    x = vect.position[X];
    y = vect.position[Y];
    z = vect.position[Z];
    d = sqrt(x * x + y * y);
    result.orientation.axis[Yaw] = atan2(x, y) * 180 / M_PI; //backwards due to 0 being in Y direction.
    result.orientation.axis[Pitch] = -atan2(z, d) * 180 / M_PI;
    result.orientation.axis[Roll] = 0.0f;
    result.orientation.type = OrientationInfo::typeYawPitchRoll;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::approachToPosition(OrientationInfo &resultOrientation, double &resultDistance, gz4d::Point<double> const &vect)
{
    if (!vect.IsValid())
        return;

    gz4d::Point<double> tp = vect-getPosition();
    resultDistance = norm(tp);

    resultOrientation.axis[Yaw] = atan2(tp[0], tp[1]) * 180 / M_PI; //backwards due to 0 being in Y direction.
    resultOrientation.axis[Pitch] = -atan2(tp[2], resultDistance) * 180 / M_PI;
    resultOrientation.axis[Roll] = 0.0f;
    resultOrientation.type = OrientationInfo::typeYawPitchRoll;
}

/*---------------------------------------------------------------------------*/
void PosableStateBase::approachFromPosition(OrientationInfo &resultOrientation, double &resultDistance, gz4d::Point<double> const &vect)
{
    if (!vect.IsValid())
        return;

    gz4d::Point<double> tp = getPosition()-vect;
    resultDistance = norm(tp);

    resultOrientation.axis[Yaw] = atan2(tp[0], tp[1]) * 180 / M_PI; //backwards due to 0 being in Y direction.
    resultOrientation.axis[Pitch] = -atan2(tp[2], resultDistance) * 180 / M_PI;
    resultOrientation.axis[Roll] = 0.0f;
    resultOrientation.type = OrientationInfo::typeYawPitchRoll;
}

void PosableStateBase::getHtmlInfo(stringstream &html, short level)
{
    const OrientationInfo *o; 
    
    Addressable::getHtmlInfo(html, level);
    html << "<h" << level << ">PosableStateBase:</h" << level << ">";
    html << "<h" << level+1 << ">Posable State</h" << level+1 << ">";
    gz4d::Point<double> p = getPosition();
    if(p.IsValid())
        html << "Position: " << p[0] << ", " << p[1] << ", " << p[2] << "<br>\n";
    o = getOrientation();
    if(o)
    {
        if (o->type == OrientationInfo::typeYawPitchRoll)
            html << "Yaw, pitch, roll: " << o->axis[0] << ", " << o->axis[1] << ", " << o->axis[2] << "<br>\n";
        else
        {
            if(o->type == OrientationInfo::typeQuaternion)
                html << "Quaternion: ";
            else
                html << "PseudoQuaternion: ";
            html << "Angle: " << o->angle << ", Axis: " << o->axis[0] << ", " << o->axis[1] << ", " << o->axis[2] << "<br>\n";
        }
    }
    
    if(getScale())
    {
        html << "Scale: 1:" << 1.0/getScale() << "<br>";
    }
    
    user->getHtmlInfo(html,level);
    
}
