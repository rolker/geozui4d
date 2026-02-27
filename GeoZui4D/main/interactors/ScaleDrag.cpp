/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ScaleDrag.cpp
/* 
 *	This class implements a zooming interactor that zooms in and out according
 *	to the dragging of a fixed point away from or toward the center of the
 *	screen (in the screen-x-direction).
 *	This class implements the following responsibilities:
 *	*	Implement ZoomDrag as described above
 *
 *	Created by mdp 4/7/2001
 */
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "interactors/ScaleDrag.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/Scene3D.h"

#define MIN_RATIO 0.01

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes all variables to a neutral position, while
 *	getting a handle onthe target ViewManager.
 */
ScaleDrag::ScaleDrag(GLboolean standalone, GLboolean vertical)
:Interactor("scaleDrag")
{
	isStandalone = standalone;
	isVertical = vertical;
	currentPose = NULL;
	currentView = NULL;
}

/*---------------------------------------------------------------------------*/
/**	Destructor.  Resets the continuous change flag if we were in the middle of
 *	interacting.
 */
ScaleDrag::~ScaleDrag()
{
	reset();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	Resets the currentPose to NULL after setting its continuous change flag
 *	to GL_FALSE.
 */
void ScaleDrag::reset()
{
	if (currentPose)
		currentPose->setContinuousChange(GL_FALSE);
	currentPose = NULL;
	currentView = NULL;
}

/*---------------------------------------------------------------------------*/
/**	If we are not a standalone, delete ourselves upon removal.
 */
void ScaleDrag::RemovalNotify(InputMediator *fromList)
{
	if (!isStandalone)
		delete this;
}

/*---------------------------------------------------------------------------*/
/**	HandleInput handles button 1 events.  It is assumed that 
 */
int ScaleDrag::HandleInput(unsigned int inputType, InputInfo *info)
{
	GLboolean startZoom = GL_FALSE;
	Window3D *currentWindow;
	GLfloat center, currentRatio;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
		{
			reset();
			if (!isStandalone)
				return REMOVE_FROM_STREAM;
		}
		return PASS_EVENT_THROUGH;
	}

	/*---Handle idle event---*/
    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle Mouse Button Events---*/
    if (inputType & MouseButton1)    //Is it a button1 event?
    {
		if (inputType & MouseButtonDown)
			startZoom = GL_TRUE;
		else if (inputType & MouseButtonUp)
		{
			if (isVertical && currentView)
				currentView->getParentWin()->getScene3D()->setHeightExaggeration(currentView->getHeightExaggeration());
			reset();
			if (!isStandalone)
				return REMOVE_FROM_STREAM;
			return SINK_EVENT;
		}
	}

	/*---Handle Mouse Motion Events---*/
	else if (inputType & MouseMotion)
	{
		if (isStandalone && !currentPose)
			startZoom = GL_TRUE;
		else
		{
			if (!currentPose)
				return PASS_EVENT_THROUGH;
			else
			{
				if (isVertical)
				{
					center = info->window->getHeight() / 2.0f;
					currentRatio = (info->y.i - center) / center;
				}
				else
				{
					center = info->window->getWidth() / 2.0f;
					currentRatio = (info->x.i - center) / center;
				}
				if (currentRatio < 0)
					currentRatio = -currentRatio;
				if (currentRatio > MIN_RATIO)
				{
					if (isVertical && currentView)
						currentView->setHeightExaggeration(currentView->getHeightExaggeration() * currentRatio/startingRatio);
					else
						currentPose->scaleBy(currentRatio/startingRatio);
					startingRatio = currentRatio;
				}
				return SINK_EVENT;
			}
		}
	}

	//Initialize the zoom.
	if (startZoom)
	{
		DYNAMIC_CAST(currentWindow, Window3D*, info->window)
		if (currentWindow)
		{
			if (isVertical)
			{
				center = currentWindow->getHeight() / 2.0f;
				startingRatio = (info->y.i - center) / center;
			}
			else
			{
				center = currentWindow->getWidth() / 2.0f;
				startingRatio = (info->x.i - center) / center;
			}
			if (startingRatio < 0)
				startingRatio = -startingRatio;
			if (startingRatio > MIN_RATIO)
			{
				currentPose = currentWindow->getPoseInfo();
				currentPose->setContinuousChange(GL_TRUE);
				currentView = currentWindow->getViewInfo();
				return SINK_EVENT;
			}
		}
	}

	return PASS_EVENT_THROUGH;
}

