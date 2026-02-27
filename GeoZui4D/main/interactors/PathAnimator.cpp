/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// PathAnimator.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  Instances of this class animate a PosableStateBase toward along a path
 *  using a PosableStateWrapper.  It determines direction from a simple
 *  calculation based on the current and previous locations.  Input is
 *  also accepted from various input streams.  This class is responsible for
 *  the following.
 *      * Managing operand and resultant PosableStateBases
 *      * Process input events
 *      * Recognize a "cancel" event
 *
 *  Created by mdp 3/8/2002
 */
/*---------------------------------------------------------------------------*/

#include "engine/Object.h"
#include "interactors/PathAnimator.h"
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/gzCommandInterface.h"
#include "engine/TranslatableWrapper.h"
#include "engine/PosableWrapperSupport.h"
#include <string.h>
#include <stdlib.h>
//#include "engine/Joystick.h"
#include "engine/Scene3D.h"
#include "utilities/DottedAddress.h"
#include <iostream>
#include <gz4d/util/Utilities.h>
#include "engine/TimeControl.h"

using namespace std;

REGISTER_ANIMATOR(PathAnimator);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PathAnimator::PathAnimator(PosableStateBase *resultant,
                                     PosableStateBase *offsetOperand)
: Animator(resultant, 2)
{
    init(offsetOperand);
}

/*---------------------------------------------------------------------------*/
PathAnimator::PathAnimator(string name,
                                     PosableStateBase *resultant,
                                     PosableStateBase *offsetOperand)
: Animator(resultant, name, 2)
{
    init(offsetOperand);
}

/*---------------------------------------------------------------------------*/
void PathAnimator::init(PosableStateBase *offsetOperand)
{
    addOperand(NULL);
    addOperand(offsetOperand);

    speed = 0.1f;
    idleSpeed = 0.005f;
    negativeFactor = 1.0f;
    headingOffset = 0.0f;

    stayAbove = NULL;
    respondToArrowKeys = GL_TRUE;
    allowSpeedChange = GL_TRUE;
    allowHeadingChange = GL_TRUE;
    lastHeading = 0.0f;

    timeOffset = 0.0;
    timeScale = 1.0;
    timeBoundsNeedsReset = GL_FALSE;
    scaleAttributeIndex = rollAttributeIndex = pitchAttributeIndex = headingAttributeIndex = -1;
    logScale = invertScale = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
PathAnimator::~PathAnimator()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int PathAnimator::HandleInput(unsigned int inputType, InputInfo *info)
{
    int key;

    if (inputType & EnumeratedAction)
    {
        if (inputType == BreakAction)
        {
            resultantPose->setContinuousChange(GL_FALSE);
            return PASS_EVENT_THROUGH;
        }
        return Animator::HandleInput(inputType, info);
    }

    if (!info)
        return Animator::HandleInput(inputType, info);

    /*---Handle Keyboard Events---*/
    if ((inputType & KeyAction) && respondToArrowKeys)
    {
        key = info->val.i;
        if (key & KeySpecial && inputType & KeyDown)
        {
            if (key == KeyUpArrow)
            {
                speed *= 1.25f;
                return SINK_EVENT;
            }
            if (key == KeyDownArrow)
            {
                speed /= 1.25f;
                return SINK_EVENT;
            }/*
            if (key == KeyLeftArrow)
            {
                headingOffset -= 1.0f;
                return SINK_EVENT;
            }
            if (key == KeyRightArrow)
            {
                headingOffset += 1.0f;
                return SINK_EVENT;
            }*/
        }
    }
//     else if ((inputType & Joystick::JoystickAction))
//     {
//         Joystick::JoystickInfoType * js;
//         float val;
// 
//         js = (Joystick::JoystickInfoType *)(info->val.p);
// 
//         if (allowSpeedChange)
//         {
//             val = (float)(-js->Axes[1] / 300000.0);
//             if (val < .055f && val > -.055f)
//                 speed = idleSpeed;
//             else
//             {
//                 speed = (val / 0.055f) * idleSpeed;
//                 if (speed < 0)
//                     speed *= negativeFactor;
//             }
//         }
//         if (allowHeadingChange)
//             headingOffset = (float) (js->Axes[0] / 2000.0);
//     }

    return Animator::HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
int PathAnimator::animate()
{
    PosableStateInfo accum, temp;
    PosableStateWrapper *wrapper = NULL;
    SmartValue val;
    float height, scale;
    GLboolean result;

    //accumulate operands
    if (operandPoses[WrapperOperand])
    {
        DYNAMIC_CAST(wrapper, PosableStateWrapper *, operandPoses[WrapperOperand]);
        if (wrapper)
        {
            if (!timeAttribute.empty())
            {
                if (timeBoundsNeedsReset)
                {
                    wrapper->resetTimeBoundsOnAttribute(timeOffset, timeScale);
                    timeBoundsNeedsReset = GL_FALSE;
                }
                result = wrapper->indexByAttributeValue((float)((Scene3D::timeControl->getGZTime() - timeOffset) / timeScale));
                if (!result) //some problem with the time attribute, revert to other for remainder...
                {
                    cerr << "Error using " << timeAttribute << " to drive PathAnimator.  Attribute disabled." << endl;
                    timeAttribute = "";
                }
            }
            else
                wrapper->setIndex(wrapper->getFloatIndex() + speed);
        }
        accum.scale = 1.0;
        operandPoses[WrapperOperand]->getPoseInfo(accum, changeMask);
        if (operandPoses[OffsetOperand])
        {
            operandPoses[OffsetOperand]->getPoseInfo(temp, changeMask);
            resultantPose->incrementPoseInfo(accum, temp, changeMask);
        }
    }
    else if (operandPoses[OffsetOperand])
        operandPoses[OffsetOperand]->getPoseInfo(accum, changeMask);
    else
        return PASS_EVENT_THROUGH;

    //enforce the surface-following bit (this is a bit of a hack)
    if (stayAbove)
    {
        height = stayAbove->GetHeightAt(accum.position[X], accum.position[Y]);
        if (height != Object::invalidHeight)
        {
            height += aboveAmount;
            if (accum.position[Z] < height)
                accum.position[Z] = height;
        }
    }

    /* set the position and direction */
    resultantPose->getPoseInfo(temp, changeMask);
    resultantPose->AccumulateNotifications();
    //position
    if (changeMask & ChangeTranslateMask)
        resultantPose->setPosition(accum.position, changeMask);
    //scale
    if (changeMask & ChangeScaleMask)
    {
        if (!scaleAttribute.empty() && wrapper)
        {
            if (scaleAttributeIndex < 0)
                scaleAttributeIndex = wrapper->getAttributeIndex(scaleAttribute);
            wrapper->getAttribute(val, scaleAttributeIndex);
            if (logScale)
                scale = (float)pow((float)2.0, val.asFloat());
            else
                scale = val.asFloat();
            if (scale < 0)
                scale *= -1.0f;
            if (invertScale && scale > 0.0001)
                scale = 1.0f / scale;
            resultantPose->setScale(scale, ChangeScale);
        }
        else
            resultantPose->setScale(accum.scale, changeMask);
    }
    //orientation
    if (changeMask & ChangeRotateMask)
    {
        if (!headingAttribute.empty() && wrapper)
        {
            if (headingAttributeIndex < 0)
                headingAttributeIndex = wrapper->getAttributeIndex(headingAttribute);
            wrapper->getAttribute(val, headingAttributeIndex);
            resultantPose->setOrientation(0.0f, (float)fmod(val.asFloat(), (float)360.0), ChangeRotateYaw);
        }
        else
        {   //calculate our current direction
            resultantPose->decrementPoseInfo(accum, temp, changeMask);
            if (accum.position[X] != 0.0 ||
                accum.position[Y] != 0.0 ||
                accum.position[Z] != 0.0)
            {
                if (speed < 0)
                {
                    accum.position[X] = -accum.position[X];
                    accum.position[Y] = -accum.position[Y];
                    accum.position[Z] = -accum.position[Z];
                }
                resultantPose->vectorToAnglePoseInfo(accum, accum);
                lastHeading = fmod(accum.orientation.axis[Y], 360.0);
                accum.orientation.axis[X] = fmod(accum.orientation.axis[X], 360.0);
                accum.orientation.axis[Y] = fmod(accum.orientation.axis[Y] + headingOffset, 360.0);
                accum.orientation.axis[Z] = fmod(accum.orientation.axis[Z], 360.0);
                resultantPose->setOrientation(accum.orientation, changeMask);
            }
            else
                resultantPose->setOrientation(0.0, fmod(lastHeading + headingOffset, 360.0), ChangeRotateYaw);
        }
        if (!pitchAttribute.empty() && wrapper)
        {
            if (pitchAttributeIndex < 0)
                pitchAttributeIndex = wrapper->getAttributeIndex(pitchAttribute);
            wrapper->getAttribute(val, pitchAttributeIndex);
            resultantPose->setOrientation(val.asFloat(), 0.0, ChangeRotatePitch);
        }
        if (!rollAttribute.empty() && wrapper)
        {
            OrientationInfo info;

            if (rollAttributeIndex < 0)
                rollAttributeIndex = wrapper->getAttributeIndex(rollAttribute);
            wrapper->getAttribute(val, rollAttributeIndex);
            info.axis[Z] = val.asFloat();
            resultantPose->setOrientation(info, ChangeRotateRoll);
        }
    }
    resultantPose->ResumeSingularNotification();

    return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/* HANDLE_ATTRIBUTE_NAME just makes it so the same basic code isn't replicated
 * 4 times taking up space.  Note: it must be the last in a string of if-else's
 */
#define HANDLE_ATTRIBUTE_NAME(name, Name) \
    if (!strcmp(argv[1], "name"))\
    {\
        PosableStateWrapper *wrapper = NULL;\
        if (argc < 3)\
        {\
            Tcl_SetResult(interp, (char *) name##Attribute.c_str(), TCL_VOLATILE);\
            return TCL_OK; \
        }\
        if (operandPoses[WrapperOperand])\
        {\
            DYNAMIC_CAST(wrapper, PosableStateWrapper *, operandPoses[WrapperOperand]);\
        }\
        if (argv[2][0] == '-')\
            set##Name##Attribute(wrapper, NULL);\
        else\
        {\
            if (!set##Name##Attribute(wrapper, strdupp(argv[2])))\
                return reportAttributeError(interp, wrapper, name##Attribute);\
        }\
    }\
    else\
        badSyntax = GL_TRUE;\

/*---------------------------------------------------------------------------*/
int PathAnimator::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    Addressable *address;
    PosableStateBase *newPose;
    int retval, amount, toIndex = -1;
    char retstr[16] = "";
    GLboolean simpleFlag = GL_TRUE, speedFlag = GL_FALSE, headingFlag = GL_FALSE;
    GLboolean badSyntax = GL_FALSE;
    amount = 0;

    if (argc >= 1)
    {
        if (!strcmp(argv[0], "speed"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "idle"))
                {
                    if (argc >= 3)
                        setIdleSpeed((float)atof(argv[2]));
                    sprintf(retstr, "%f", getIdleSpeed());
                    simpleFlag = GL_FALSE;
                }
                else if (!strcmp(argv[1], "negativeFactor"))
                {
                    if (argc >= 3)
                        setNegativeSpeedFactor((float)atof(argv[2]));
                    sprintf(retstr, "%f", getNegativeSpeedFactor());
                    simpleFlag = GL_FALSE;
                }
                else if (!strcmp(argv[1], "usage"))
                {
                    Tcl_AppendResult(interp, "Usage: speed {usage | {[idle | negativeFactor] [<value>]}}", NULL);
                }
                else
                    setSpeed((float)atof(argv[1]));
            }
            if (simpleFlag)
                sprintf(retstr, "%f", getSpeed());
            Tcl_SetResult(interp, (char *) retstr, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "advanceLine"))
        {
            PosableStateWrapper *wrapper = NULL;

            if (operandPoses[WrapperOperand])
            {
                DYNAMIC_CAST(wrapper, PosableStateWrapper *, operandPoses[WrapperOperand]);
                if (!wrapper)
                {
                    Tcl_AppendResult(interp, "Cannot advanceLine--no wrapper target has been specified.", NULL);
                    return TCL_ERROR;
                }
            }
            if (argc > 1)
            {
                if (argv[1][0] == '@')
                    toIndex = atoi(&argv[1][1]);
                else
                    amount = atoi(argv[1]);
            }
            else
                amount = 1;

            wrapper->delayIndexNotification(); //make this an atomic operation
                if (!timeAttribute.empty())
                    Scene3D::timeControl->setGZTime(0.0, GL_TRUE);

                wrapper->setIndex(0.0f);
                if (toIndex >= 0)
                    wrapper->setIndex(toIndex, 2);
                else
                    wrapper->setIndex(wrapper->getIntegerIndex(2) + amount, 2);
                timeBoundsNeedsReset = GL_TRUE;
            wrapper->resumeIndexNotification();

            sprintf(retstr, "%d", wrapper->getIntegerIndex(2)); //might not be what we set it to, so don't use a variable!
            Tcl_SetResult(interp, (char *) retstr, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "timeAttribute"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "offset"))
                {
                    if (argc < 3)
                    {
                        sprintf(retstr, "%f", timeOffset);
                        Tcl_SetResult(interp, retstr, TCL_VOLATILE);
                        return TCL_OK;
                    }
                    setTimeOffset(argv[2]);
                }
                else if (!strcmp(argv[1], "scale"))
                {
                    if (argc < 3)
                    {
                        sprintf(retstr, "%f", timeScale);
                        Tcl_SetResult(interp, retstr, TCL_VOLATILE);
                        return TCL_OK;
                    }
                    timeScale = atof(argv[2]);
                }
                else
                    HANDLE_ATTRIBUTE_NAME(time, Time);
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Invalid syntax.  Usage:\n",
                                 "timeAttribute name [<attribute> | -]\n",
                                 "timeAttribute offset [<startingTime> | now]\n",
                                 "timeAttribute scale [<timeScale>]\n",
                                 NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "headingAttribute"))
        {
            if (argc >= 2)
            {
                HANDLE_ATTRIBUTE_NAME(heading, Heading);
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Usage: headingAttribute name [<attribute> | -]\n",
                                 NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "pitchAttribute"))
        {
            if (argc >= 2)
            {
                HANDLE_ATTRIBUTE_NAME(pitch, Pitch);
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Usage: pitchAttribute name [<attribute> | -]\n",
                                 NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "rollAttribute"))
        {
            if (argc >= 2)
            {
                HANDLE_ATTRIBUTE_NAME(roll, Roll);
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Usage: rollAttribute name [<attribute> | -]\n",
                                 NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "scaleAttribute"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "invert"))
                {
                    if (argc < 3)
                    {
                        sprintf(retstr, "%i", invertScale);
                        Tcl_SetResult(interp, retstr, TCL_VOLATILE);
                        return TCL_OK;
                    }
                    else
                        invertScale = atoi(argv[2]);
                }
                else if (!strcmp(argv[1], "asExponent"))
                {
                    if (argc < 3)
                    {
                        sprintf(retstr, "%i", logScale);
                        Tcl_SetResult(interp, retstr, TCL_VOLATILE);
                        return TCL_OK;
                    }
                    else
                        logScale = atoi(argv[2]);
                }
                else
                    HANDLE_ATTRIBUTE_NAME(scale, Scale);
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Invalid syntax.  Usage:\n",
                                 "scaleAttribute name [<attribute> | -]\n",
                                 "scaleAttribute invert [0 | 1]\n",
                                 "scaleAttribute asExponent [0 | 1]\n",
                                 NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "wrapper"))
        {
            PosableStateWrapper *wrapper;

            if (argc < 2)
            {
                Tcl_AppendResult(interp, "Usage: wrapper <pose>", NULL);
                return TCL_ERROR;
            }
            retval = gzCommandInterface::findPosable(argv[1], address, newPose);
            
            if (retval != TCL_OK)
                return retval;

            DYNAMIC_CAST(wrapper, PosableStateWrapper *, operandPoses[WrapperOperand]);
            if (!wrapper)
            {
                Tcl_AppendResult(interp, argv[1], " is not a wrapper.", NULL);
                return TCL_ERROR;
            }

            replaceOperand(newPose, WrapperOperand);
            if (!timeAttribute.empty())
            {
                wrapper->setIndexingAttribute(wrapper->getAttributeIndex(timeAttribute));
                timeBoundsNeedsReset = GL_TRUE;
            }
            return TCL_OK;
        }
        if (!strcmp(argv[0], "stayabove"))
        {
            if (argc < 3)
            {
                Tcl_AppendResult(interp, "Usage: stayabove <object-pose> <z-offset>", NULL);
                return TCL_ERROR;
            }
            retval = gzCommandInterface::findPosable(argv[1], address, newPose);
            if (retval != TCL_OK)
                return retval;
            DYNAMIC_CAST(stayAbove, Object *, newPose->getUser());
            if (!stayAbove)
            {
                Tcl_AppendResult(interp, argv[2], " is not a valid object", NULL);
                return TCL_ERROR;
            }
            aboveAmount = (float)atof(argv[2]);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "offset"))
        {
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "Usage: offset <pose>", NULL);
                return TCL_ERROR;
            }
            retval = gzCommandInterface::findPosable(argv[1], address, newPose);
            if (retval != TCL_OK)
                return retval;
            replaceOperand(newPose, OffsetOperand);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "keyboard"))
        {
            if (argc < 2)
            {
                char val[8] = "";
                sprintf(val, "%d", respondToArrowKeys);
                Tcl_SetResult(interp, (char *) val, TCL_VOLATILE);
            }
            else
                respondToArrowKeys = atoi(argv[1]);
            return TCL_OK;
        }
        if (!strcmp(argv[0], "joystick"))
        {
            if (argc >= 2)
            {
                if (strcmp(argv[1], "speed") == 0)
                    speedFlag = GL_TRUE;
                else if (strcmp(argv[1], "heading") == 0)
                    headingFlag = GL_TRUE;
            }
            if (argc < 2 || (!speedFlag && !headingFlag))
            {
                Tcl_AppendResult(interp, "Usage: joystick {speed | heading} [0 | 1]", NULL);
                return TCL_ERROR;
            }
            if (argc < 3)
            {
                char val[8] = "";
                if (speedFlag)
                    sprintf(val, "%d", allowSpeedChange);
                else
                    sprintf(val, "%d", allowHeadingChange);
                Tcl_SetResult(interp, (char *) val, TCL_VOLATILE);
            }
            else
            {
                if (speedFlag)
                    allowSpeedChange = atoi(argv[2]);
                else
                    allowHeadingChange = atoi(argv[2]);
            }
            return TCL_OK;
        }
    }
    return Animator::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int PathAnimator::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "keyboard speed wrapper offset stayabove advanceLine timeAttribute headingAttribute pitchAttribute rollAttribute scaleAttribute ", NULL);
    return Animator::list(interp);
}

/*---------------------------------------------------------------------------*/
int PathAnimator::reportAttributeError(Tcl_Interp *interp, PosableStateWrapper *wrapper, string attribute)
{
    int i;

    Tcl_AppendResult(interp, "Unable to find attribute ", attribute.c_str(), ".  Did you mean one of these?\n", NULL);
    for (i = 0; i < wrapper->getNumAttributes(); i ++)
        Tcl_AppendResult(interp, "\t", wrapper->getAttributeName(i).c_str(), "\n", NULL);
    return TCL_ERROR;
}

/*---------------------------------------------------------------------------*/
GLboolean PathAnimator::setTimeAttribute(PosableStateWrapper *wrapper, string name)
{
    int indexingAttribute;

    timeAttribute = name;
    if (timeAttribute.empty())
    {
        if (wrapper)
            wrapper->setIndexingAttribute(-1);
        return GL_TRUE;
    }

    if (wrapper)
    {
        indexingAttribute = wrapper->getAttributeIndex(timeAttribute);
        if (indexingAttribute < 0)
            return GL_FALSE;
        wrapper->setIndexingAttribute(indexingAttribute);
        timeBoundsNeedsReset = GL_TRUE;
    }
    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
GLboolean PathAnimator::setAttribute(PosableStateWrapper *wrapper, string name, string &attName, int &attInd)
{
    attName = name;
    if (attName.empty())
    {
        attInd = -1;
        return GL_TRUE;
    }

    if (wrapper)
    {
        attInd = wrapper->getAttributeIndex(attName);
        if (attInd < 0)
            return GL_FALSE;
    }
    else
        attInd = -1;

    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
GLboolean PathAnimator::setTimeOffset(string offset)
{
    Scene3D::timeControl->updateCurrentTime(); //update GeoZui's notion of when now is
    if (offset == "now")
        timeOffset = Scene3D::timeControl->getGZTime();
    else
        timeOffset = atof(offset.c_str());
    return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
Animator *PathAnimator::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    PosableStateWrapper *wrapper = NULL;
    PosableWrapperSupport *support;
    string text;

    Animator::LoadXML(xml, path, ncfile);

    boost::optional<std::string> attribute;
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.spline"))
        wrapper->setIndex("cubic_spline", 1);

    attribute = xml.get_optional<std::string>("<xmlattr>.useWrapper");
    if(!attribute || gz4d::IsTrue(*attribute))
    {
        wrapper = new TranslatableWrapper(NULL, "path_wrapper");
        support = dynamic_cast<PosableWrapperSupport*>(operandPoses[WrapperOperand]->getUser());
        if (!support)
        {
            cerr << "Unable to create a wrapper for XML animator target.\n";
            delete wrapper;
            wrapper = NULL;
            return this;
        }
        getAddress()->addChild(wrapper->getAddress());
        //drop the old target and replace it with the wrapper
        operandPoses[WrapperOperand] = wrapper;
        wrapper->attachTo(support);
        wrapper->setNumDimensions(2);
        wrapper->setIndex(0.0f);

        if(attribute = xml.get_optional<std::string>("<xmlattr>.spline"))
            if(gz4d::IsTrue(*attribute))
                wrapper->setIndex("cubic_spline", 1);
        
        if(attribute = xml.get_optional<std::string>("<xmlattr>.time_name"))
            setTimeAttribute(wrapper,*attribute);
        
        if(attribute = xml.get_optional<std::string>("<xmlattr>.time_offset"))
            setTimeOffset(*attribute);
        
        timeScale =  xml.get<double>("<xmlattr>.time_scale",timeScale);
        
        if(attribute = xml.get_optional<std::string>("<xmlattr>.heading_name"))
            setHeadingAttribute(wrapper,*attribute);
        
        if(attribute = xml.get_optional<std::string>("<xmlattr>.pitch_name"))
            setPitchAttribute(wrapper,*attribute);
        
        if(attribute = xml.get_optional<std::string>("<xmlattr>.roll_name"))
            setRollAttribute(wrapper,*attribute);
        
        if(attribute = xml.get_optional<std::string>("<xmlattr>.scale_name"))
            setScaleAttribute(wrapper,*attribute);

    }

    //Eventually handle other things like speed, stayabove, keyboard, and joystick stuff
    return this;
}
