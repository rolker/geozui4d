/* Copyright (c) 2008 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// PositionDrag.cpp
/* 
 *	This class implements a translation interactor that works by enabling the
 *	user to drag the scene around as if it were laying face up on the table.
 *	It does not act as if the user is dragging a world point around, although
 *  it approaches this as the pitch reaches 90 degrees (face up).
 *
 *	Created by mdp 3/7/2008
 */
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "interactors/PositionDrag.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/Scene3D.h"

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes all variables to neutral values.
 */
PositionDrag::PositionDrag()
:Interactor("positionDrag")
{
    prevX = prevY = -1;
	currentPose = NULL;
	currentView = NULL;
}

/*---------------------------------------------------------------------------*/
/**	Destructor.  Resets the continuous change flag if we were in the middle of
 *	interacting.
 */
PositionDrag::~PositionDrag()
{
	reset();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	Resets the currentPose to NULL after setting its continuous change flag
 *	to GL_FALSE.
 */
void PositionDrag::reset()
{
	if (currentPose)
		currentPose->setContinuousChange(GL_FALSE);
    prevX = prevY = -1;
	currentPose = NULL;
	currentView = NULL;
}

/*---------------------------------------------------------------------------*/
/**	HandleInput handles button 1 events. 
 */
int PositionDrag::HandleInput(unsigned int inputType, InputInfo *info)
{
	GLboolean startTranslate = GL_FALSE;
	Window3D *currentWindow;
	GLfloat scale, heading, diffX, diffY, yScale;
    PosableStateInfo xyInfo;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
			reset();
		return PASS_EVENT_THROUGH;
	}

	/*---Handle idle event---*/
    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle Mouse Button Events---*/
    if (inputType & MouseButton1)    //Is it a button1 event?
    {
		if (inputType & MouseButtonDown)
			startTranslate = GL_TRUE;
		else if (inputType & MouseButtonUp)
		{
			reset();
			return SINK_EVENT;
		}
	}
	/*---Handle Mouse Motion Events---*/
	else if (inputType & MouseMotion)
	{
        if (prevX < 0 && prevY < 0)  //has side-effect of quiting interaction if go to upper left corner.
            return PASS_EVENT_THROUGH;

        scale = 0.001 / currentPose->getScale();
        heading = currentPose->getOrientationAxis()[Yaw] * M_PI / 180.0;
        yScale = cos(currentPose->getOrientationAxis()[Pitch] * M_PI / 180.0);
        yScale = 1.0 + (yScale * yScale * 0.6 / scale);
        diffX = info->x.i - prevX;
        diffY = (info->y.i - prevY) * yScale;
        xyInfo.position[X] = scale * (-diffX * cos(heading) + diffY * sin(heading));
        xyInfo.position[Y] = scale * (diffY * cos(heading) + diffX * sin(heading));
        currentPose->offsetPoseInfoBy(xyInfo, ChangeTranslateX | ChangeTranslateY);
        prevX = info->x.i;
        prevY = info->y.i;
		return SINK_EVENT;
	}

	//Initialize the translation.
	if (startTranslate)
	{
		DYNAMIC_CAST(currentWindow, Window3D*, info->window)
		if (currentWindow)
		{
			currentPose = currentWindow->getPoseInfo();
			currentPose->setContinuousChange(GL_TRUE);
			currentView = currentWindow->getViewInfo();
            prevX = info->x.i;
            prevY = info->y.i;
			return SINK_EVENT;
		}
	}

	return PASS_EVENT_THROUGH;
}

