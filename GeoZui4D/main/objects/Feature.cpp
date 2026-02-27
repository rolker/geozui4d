/* Copyright (c) 2008 Data Visualization Research Lab,
   //                    Center for Coastal and Ocean Mapping,
   //                    University of New Hampshire.  All rights reserved.
   //                    http://www.ccom.unh.edu/vislab
   // Not to be copied or distributed without written agreement.
   // Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include "MainWindow.h"
#include "engine/PosableStateBase.h"
#include "Feature.h"
#include "X3DImageInstance.h"
#include "utilities/DottedAddress.h"
#include <iostream>
#include "engine/Scene3D.h"
#include <filesystem>
#include "engine/ObjectPoseState.h"
#include "TextureScreen.h"
#include "engine/ViewManager.h"

using namespace std;

REGISTER_OBJECT(Feature, "");

extern MainWindow * mainWindow;

float Feature::emphasisFactor = 1.0;
float Feature::CriticalPitch = 60.0;
const float Feature::minEmphasisTFactor = 4000.0;
const float Feature::maxEmphasisTFactor = 20000.0;
const float Feature::emphasisMagic = 133.3;

/*--------------------------------------------------------*/
Feature::Feature(GLboolean allocatePosable, int pickingTypes)
:ObjectCollection(allocatePosable, pickingTypes),
scaleChildren(0, 0, 0),
priorityRange(-1, -1, -1),
lineColor(0.9f, 0.7f, 0.6f)
{
    priority = 0;
    lastBoundaryIndex = 0;
    child = NULL;
    scaleFactor = 1;
    minRange = -1.0;
    maxRange = 20000.0;
    ownEmphasisFactor = 1.0;
}

/*--------------------------------------------------------*/
void Feature::setPriority(float p)
{
    priority = p;
}

/*--------------------------------------------------------*/
void Feature::setMinRange(float p)
{
    minRange = p;
}

/*--------------------------------------------------------*/
void Feature::setMaxRange(float p)
{
    maxRange = p;
}

/*--------------------------------------------------------*/
void Feature::setEmphasisFactor(float p)
{
    ownEmphasisFactor = p;
}

/*--------------------------------------------------------*/
void Feature::setScale(gz4d::SFVec3f scale)
{
    scaleChildren = scale;
    if (scaleChildren[0] != 0)
        pose->setScale(scaleChildren[0]);
}

/*--------------------------------------------------------*/
void Feature::setRange(gz4d::SFVec3f range)
{
    priorityRange = range;
}

/*--------------------------------------------------------*/
void Feature::setLineColor(gz4d::SFColor color)
{
    lineColor = color;
}

/*--------------------------------------------------------*/
void Feature::setLocationUTM(gz4d::SFVec3d location)
{
    pose->setPosition(location[X], location[Y], location[Z]);
}

/*--------------------------------------------------------*/
void Feature::addChild(gz4d::SFNode child)
{
    X3DPrototypeInstance *proto;
    Object *ob;

    DYNAMIC_CAST(proto, X3DPrototypeInstance *, &(*child));
    if (!proto)
        return;
        
    ob = proto->getObject();
    if (!ob)
        return;

    ob->SetPosition(ob->GetPosition()+pose->getPosition());
    ob->ignore(PICK_CLICK1); //we want this feature to be clicked, not the child.
    AddObject(ob);
}

struct ImageRecord {
    Object *ob;
    float quality;
    float yaw;

    bool operator <(const ImageRecord &other) {return (yaw < other.yaw);}
};
/*--------------------------------------------------------*/
/** Propogates and color changes to children, and recomputes the quantities
 *  needed to figure out which child to display when
 */
void Feature::updateChildren()
{
	Node * cursor;
	Object * currentObject = NULL;
    TextureScreen *ts;
    float priorYaw, currentYaw;
    float priorQuality, currentQuality;
    bool havePrior = false;
    bool haveExtraBound = false;
    float extraBound, lastYawBoundary;

    yawBoundaries.clear();
    objectsByBounds.clear();

	if(nodeList)
	{
        struct ImageRecord currentRecord, otherRecord;
        std::list<ImageRecord> imageList;

		cursor = nodeList;
	    while (cursor != NULL)
		{
			currentObject = cursor->object;
            DYNAMIC_CAST(ts, TextureScreen *, currentObject);
            if (ts)
            {
                ts->setLineColor(&(lineColor[0]));

                //construct an ordered list of these dudes.
                currentRecord.ob = currentObject;
                currentYaw =  ts->getPosable()->getOrientationAxis()[Yaw];
                //ensure 0 <= yaw < 360
                while (currentYaw < 0.0)
                    currentYaw += 360.0;
                while (currentYaw >= 360.0)
                    currentYaw -= 360.0;
                currentRecord.yaw = currentYaw;
                imageList.push_back(currentRecord);
            }
			cursor = cursor->next;
		}
        imageList.sort();//sorted! now we can create a vector that "maps" yaw to objects
        //Find out which end of the list belongs at yaw 0
        currentRecord = imageList.front();
        otherRecord = imageList.back();
        priorYaw = otherRecord.yaw - 360.0;
        lastYawBoundary = (priorYaw * otherRecord.quality + currentRecord.yaw * currentRecord.quality)/(otherRecord.quality + currentRecord.quality);
        yawBoundaries.push_back(0.0); ///and the winner is...
        if (lastYawBoundary < 0)
        {
            objectsByBounds.push_back(currentRecord.ob);
            haveExtraBound = true;
            extraBound = lastYawBoundary + 360.0;
        }
        else
        {
            objectsByBounds.push_back(otherRecord.ob);
            yawBoundaries.push_back(lastYawBoundary);
            objectsByBounds.push_back(currentRecord.ob);
        }
        priorYaw = currentRecord.yaw;
        priorQuality = currentRecord.quality;
        //work on the rest of the list
        for (std::list<ImageRecord>::iterator i = imageList.begin(); i != imageList.end(); i++)
        {
            if (!havePrior)
            {   //skip the first item, since we already have it!
                havePrior = true;
                continue;
            }
            currentYaw = i->yaw;
            currentQuality = i->quality;
            lastYawBoundary = (priorYaw * priorQuality + currentYaw * currentQuality)/(priorQuality + currentQuality);
            yawBoundaries.push_back(lastYawBoundary);
            objectsByBounds.push_back(i->ob);
            priorYaw = currentYaw;
            priorQuality = currentQuality;
        }
        if (haveExtraBound) //first item is also last item
        {
            yawBoundaries.push_back(extraBound);
            objectsByBounds.push_back(imageList.front().ob);
        }
        lastBoundaryIndex = 0;
	}
}

/*--------------------------------------------------------*/
Object *Feature::chooseObjectToDisplay(Window3D *win)
{
	Object * currentObject = NULL;
    OrientationInfo resultInfo, ri2;
    ViewManager *viewInfo;
    PosableStateBase *winPose;
    double workspaceDistance;
    int end = yawBoundaries.size() - 1;

	if(win && end >= 0)
	{
        viewInfo = win->getViewInfo();
        winPose = viewInfo->getPosable();
        //Calculate the angle of approach from the window to the object
        getPosable()->approachFromPosition(resultInfo, approachDistance, viewInfo->getEyePoint());
        getPosable()->approachFromPosition(ri2, workspaceDistance, viewInfo->getReferencePoint());
        if (workspaceDistance < minRange || workspaceDistance > maxRange)
            return NULL;

        if (winPose->getOrientationAxis()[Pitch] >= CriticalPitch)
            approachAngle = winPose->getOrientationAxis()[Yaw];
        else
        {
            approachAngle = resultInfo.axis[Yaw];
        }

        //Determine which child is the best to show for that approach angle
        if (approachAngle < 0) //result of vectorToAnglePoseInfo is -180 to 180... move: 0 to 360
            approachAngle += 360.0;
        while ((approachAngle < yawBoundaries[lastBoundaryIndex]) && (lastBoundaryIndex > 0))
            lastBoundaryIndex--;
        while (lastBoundaryIndex < end && approachAngle >= yawBoundaries[lastBoundaryIndex + 1])
            lastBoundaryIndex++;
        currentObject = objectsByBounds[lastBoundaryIndex];
        if (currentObject && !viewInfo->pointIsOnScreen(getPosable()->getPosition()))
            currentObject = NULL;
	}
    else
    {
        approachAngle = approachDistance = 0;
    }
    return currentObject;
}

/*--------------------------------------------------------*/
void Feature::determineChildTransform(Window3D *win)
{
    if (!priority || !child || !win || !approachDistance)
        return;

    ViewManager* viewInfo = win->getViewInfo();
    PosableStateBase *winPose = viewInfo->getPosable();

    //const double *winPos = viewInfo->getEyePoint(); //we should do these things w.r.t the eyepoint!
    //const double *myPos = pose->getPosition();
    double scale = winPose->getScale();
    double invscale = 1.0 / scale;
    double distanceFromWin = approachDistance;
    double heading;
    scaleFactor = 1;
    viewPitch = winPose->getOrientationAxis()[Pitch];

    if (invscale > emphasisMagic)
        distanceFromWin += invscale - emphasisMagic;

    float pixSize = viewInfo->getPixelSize();
    float blendEdge, threshold = minEmphasisTFactor * pixSize * emphasisMagic / invscale;
    if (distanceFromWin > threshold) //only do this if far enough away
    {
        //find the difference between the eyepoint's heading and its heading toward us.
        float winHeading = winPose->getOrientationAxis()[Yaw];
        heading = fabs(approachAngle - winHeading);
        while (heading >= 180.0)
            heading -= 360.0;
        while (heading < -180.0)
            heading += 360.0;
        heading = fabs(heading);

        scaleFactor = log(1.0 + distanceFromWin + scale) / 6.0;
        if (scaleFactor < 1.0)
            scaleFactor = 1.0;
        if (heading <= 1.0)
            scaleFactor *= 1.0 + priority / 100.0;
        else if (heading <= 11.0)
            scaleFactor *= 1.0 + priority / (100.0 + (heading - 1.0) * 10.0);
        else
            scaleFactor *= 1.0 + priority / 200.0;

        blendEdge = maxEmphasisTFactor * pixSize;
        if (distanceFromWin < blendEdge)
            scaleFactor = (distanceFromWin - threshold)*scaleFactor/(blendEdge - threshold);
        //magic to get little change close up (invscale = 130), but a fair rescaling at invscale == 2000...
        if (invscale > emphasisMagic)
        {
            invscale /= (emphasisMagic * 130.0 / 133.3);
            scaleFactor *= invscale - (pow(invscale, 1.5) / 15) + 0.045; 
        }
        scaleFactor *= emphasisFactor * ownEmphasisFactor;
    }
}

/*--------------------------------------------------------*/
void Feature::preTransformChild()
{
    if (viewPitch >= CriticalPitch) //if the view is mostly from above, rotate all images to be "flat"
        glRotatef(-90.0, 1.0, 0.0, 0.0);
    else if (scaleFactor > 1.0) //don't scale images when we're looking at them from above.
    {
        TextureScreen *ts;
        DYNAMIC_CAST(ts, TextureScreen *, child);
        if (ts)
        {
            float screen_height = ts->getHeight();
            glTranslatef(0.0, 0.0, (scaleFactor * screen_height - screen_height)/2.0);
        }
        glScalef(scaleFactor, scaleFactor, scaleFactor);
    }
}


/*--------------------------------------------------------*/
int Feature::DrawHighRes(int eye, Window3D *win)
{
  if (doneFlag)
		return doneFlag;

  doneFlag = 1;
  if(nodeList)
  {
    child = chooseObjectToDisplay(win);
    if (child)
    {
        glPushMatrix();
        determineChildTransform(win);
		doneFlag &= DrawObjectHighRes(child, eye, win);
        glPopMatrix();
    }
  }
  return doneFlag;
}

/*--------------------------------------------------------*/
void Feature::DrawLowRes(float degradation, Window3D *win)
{
    child = chooseObjectToDisplay(win);
    if (child)
    {
        glPushMatrix();
        determineChildTransform(win);
        DrawObjectLowRes(child, degradation, win);
        glPopMatrix();
    }
}

/*---------------------------------------------------------------------------*/
int Feature::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	char retval[64];

	if (argc >= 1)
    {
        string message = argv[0];
        if (message == "borderColor")
		{
			if (argc >= 4)
            {
				lineColor[0] = atof(argv[1]);
				lineColor[1] = atof(argv[2]);
				lineColor[2] = atof(argv[3]);
            }
			sprintf(retval, "%f %f %f", lineColor[0], lineColor[1], lineColor[2]);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
        if (message == "emphasisFactor")
        {
            if (argc >= 2)
                emphasisFactor = atof(argv[1]);
            sprintf(retval, "%f", emphasisFactor);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (message == "minRange")
        {
            if (argc >= 2)
                minRange = atof(argv[1]);
            sprintf(retval, "%f", minRange);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (message == "maxRange")
        {
            if (argc >= 2)
                maxRange = atof(argv[1]);
            sprintf(retval, "%f", maxRange);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (message == "ownEmphasisFactor")
        {
            if (argc >= 2)
                ownEmphasisFactor = atof(argv[1]);
            sprintf(retval, "%f", ownEmphasisFactor);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (message == "currentChild")
        {
            if (child)
                return child->getPosable()->perform(argc - 1, &(argv[1]), interp);
			Tcl_SetResult(interp, const_cast<char*>(""), TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "priority")
        {
            if (argc >= 2)
                setPriority(atof(argv[1]));
            sprintf(retval, "%f", priority);
            Tcl_SetResult(interp, retval, TCL_VOLATILE);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
	}
	return Object::perform(argc, argv, interp);

}

/*---------------------------------------------------------------------------*/
int Feature::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "borderColor emphasisFactor minRange maxRange currentChild priority ", NULL);
	return Object::list(interp);
}
