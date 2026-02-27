/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ZoomInteractor.cpp
// 
//
//	This class implements a zooming interactor that zooms in and out at a rate
//	proportional to the distance to the selected object.  This class operates
//	on ViewManagers to effect the viewpoint changes.
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "interactors/ZoomInteractor.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Scene3D.h"
#include "engine/Object.h"
#include "interactors/DoubleClickInteractor.h"
#include <tcl.h>
#include <math.h>
#include <gz4d/vector/ios.h>

#define DISTANCE_TO_THRESHHOLD 4
#define MAX_TOOL_COMMAND 2048

ZoomInteractor::InteractionType ZoomInteractor::interactionType = ZoomInteractor::SwitchDirectionOnThreshhold;
double ZoomInteractor::zoomInRate = 8.0;
double ZoomInteractor::zoomOutRate = 8.0;
double ZoomInteractor::translateRate = 25.0;

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes all variables to a neutral position, while
 *	getting a handle on the target ViewManager.
 */
ZoomInteractor::ZoomInteractor(ViewManager *targetView, InteractionType it)
:Interactor("zoom")
{
	locksOnWindow = NULL;
	changeMask = 0;

	/*---Initialize interaction values---*/
	zoomDirection = NotZooming;
	if (it != DontChangeInteractionType)
		interactionType = it;
	shiftPressed = GL_FALSE;
	usingWheel = GL_FALSE;

	/*---set the target ViewManager---*/
	currentView = targetView;
}

/*---------------------------------------------------------------------------*/
/**	Destructor.  Resets the continuous change flag if we were in the middle of
 *	interacting.
 */
ZoomInteractor::~ZoomInteractor( void )
{
	if (zoomDirection != NotZooming)
		currentView->getPosable()->setContinuousChange(GL_FALSE);
}

/*---------------------------------------------------------------------------*/
/**	Adds the "Zoom Interface" menu to
 *	allow the Tcl/Tk interface some control on the interaction.
 */
int ZoomInteractor::RegisterWithTcl(ToglWindow *throughWin)
{
	#define ITEMVALUEPAIRS "{ Use \"Shift\" Key to Zoom Out}                 %d\
							{ Throttle Zoom}                                 %d\
							{ Reverse Throttle Zoom}                         %d"

	char tclCommand[MAX_TOOL_COMMAND];
	int retval;

	sprintf(tclCommand, "addOption {Zooming Interface} cascade {" ITEMVALUEPAIRS "} zoomInterface",
			SwitchDirectionOnShift,
			SwitchDirectionOnThreshhold,
			SwitchDirectionOnReverseThresh);

	throughWin->Tcl_Eval(tclCommand);

	retval = throughWin->Tcl_LinkVar("zoomInterface", &interactionType);
	retval |= throughWin->Tcl_LinkVar("zoomInRate", &zoomInRate, TCL_LINK_DOUBLE);
	retval |= throughWin->Tcl_LinkVar("zoomOutRate", &zoomOutRate, TCL_LINK_DOUBLE);
	retval |= throughWin->Tcl_LinkVar("translateRate", &translateRate, TCL_LINK_DOUBLE);

	return retval;
}

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	HandleInput handles button 2 and idle events, as well as mouse motion events.
 */
int ZoomInteractor::HandleInput(unsigned int inputType, InputInfo *info)
{

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction && zoomDirection != NotZooming)
		{
			shiftPressed = GL_FALSE;
endZoom:
			zoomDirection = NotZooming;
			usingWheel = GL_FALSE;
			changeMask = 0;
			if (locksOnWindow)
				delete locksOnWindow;
			currentView->getPosable()->setContinuousChange(GL_FALSE);
			if (inputType != BreakAction)
				return SINK_EVENT;
		}
		return PASS_EVENT_THROUGH;
	}

	/*---Handle idle event---*/
	if (inputType == NoInput && zoomDirection != NotZooming)
	{
		continueZoom();
		return PASS_EVENT_THROUGH;
	}

    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle Mouse Button Events---*/
    if (inputType & MouseButton2)    //Is it a button2 event?
    {
		if ((inputType & MouseButtonDown) && !usingWheel)
			return startZoom(info);
		else if (inputType & MouseButtonUp)
		{
			if (zoomDirection == NotZooming)
				return PASS_EVENT_THROUGH;
			goto endZoom;
		}


		return SINK_EVENT;
	}

	if( inputType & MouseButtonDown && info->val.i >= 4 && info->val.i <= 5 )
 	{
		double zoom;
    if(info->modifier.i & ModControl)
			if(info->val.i == 4)
				zoom=1.0/0.6;
			else
				zoom=0.6;
		else if(info->modifier.i & ModShift)
			if(info->val.i == 4)
				zoom=1.0/0.95;
			else
				zoom=0.95;
		else
			if(info->val.i == 4)
				zoom=1.0/0.8;
			else
				zoom=0.8;
		Zoom(zoom);//zoom    -- I feel lika a Mazda commercial..
		return SINK_EVENT;
 	}

	
/*
	else if (inputType & MouseButtonDown && !(inputType & MouseButtons)) //Scroll event
	{
		oldZoomDirection = zoomDirection;
		if (zoomDirection == NotZooming) // if weren't zooming, startzoom
		{
			usingWheel = GL_TRUE;
			startZoom(NULL);
		}
		if (!usingWheel)
			return PASS_EVENT_THROUGH;

		startingPos[Y] = 0; //determine if we should zoom in or out.
#ifdef WIN32
		uponThreshhold((info->val.i > 0)? -20 : 20);
#else
		uponThreshhold((info->val.i == 4)? -20 : 20);
#endif
		if (zoomDirection != -oldZoomDirection) //continue zoom if same direction
		{
			usingWheel = GL_TRUE;
			continueZoom();
		}
		else //otherwise stop.
			goto endZoom;
	}
*/
	/*---Handle Mouse Motion Events---*/
	else if ((interactionType & SwitchDirectionOnSomeThreshhold) && (inputType & MouseMotion)
			  && zoomDirection != NotZooming && !usingWheel)
	{
		uponThreshhold(info->y.i);
		return SINK_EVENT;
	}

	/*---Handle Keyboard Events---*/
	else if ((interactionType & SwitchDirectionOnShift) && (inputType & KeyAction))
	{
		if (info->val.i == KeyLeftShift || info->val.i == KeyRightShift)
		{
			return uponShift(inputType);
		}
	}
	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	This function sets up a zoom/translation operation, but does not perform
 *	any of that operation.
 *
 */
GLboolean ZoomInteractor::startZoom(InputInfo *info)
{
	Window3D *window;
        gz4d::Point<double> currentPosition;

	GLboolean translating = GL_TRUE;
        gz4d::Point<double> wPos;
    
	if (info)
	{
		window = (Window3D*)info->window;
		if (Object::getFrameOfReference() == NULL)
			window->getViewInfo()->unlockFromObject();

		//Remember where the mouse is (for fancy future interactions).
		startingPos[X] = (double)info->x.i;
		startingPos[Y] = (double)info->y.i;
		startingPos[Z] = -1.0;

		//Determine where the user wants to go
		wPos = window->getCursorPosOnSurfaceOrPlane(currentView->getReferencePoint()[Z], GL_TRUE);
                if (!wPos.IsValid())
			translating = GL_FALSE;
	}
	else
		translating = GL_FALSE;

	if (translating)
	{
                desiredTranslation = wPos - currentView->getReferencePoint();
        }
	else
	{
		desiredTranslation[X] = desiredTranslation[Y] = desiredTranslation[Z] = 0.0;
		changeMask = 0;
	}

	if ((interactionType & SwitchDirectionOnSomeThreshhold) || !info)
		zoomDirection = TranslatingOnly;
	else
	{
		if (info->modifier.i & ModShift)
			zoomDirection = ZoomingOut;
		else
			zoomDirection = ZoomingIn;
	}
//	changeMask = PosableStateLock::LockDeltaTranslate | PosableStateLock::LockDeltaScale;
//	locksOnWindow = relationshipManager->getThoseThatUpdate(currentView->getPosable(), changeMask, "PosableStateLock");

	doubleClickInteractor->reset();
	currentView->getPosable()->setContinuousChange(GL_TRUE);
	return SINK_EVENT;
}

/*---------------------------------------------------------------------------*/
/**	This function performs another iteration of a zoom and translation.
 */
void ZoomInteractor::continueZoom()
{
    gz4d::Point<double> currentTranslation;	// The amount we are translating this time around.
	PosableStateBase *currentPose;
//	PosableStateRelationship *currentRel;
//	PosableStateLock *currentLock;
	float power = Scene3D::timeForLastFrame()/40.0;
	float moveBy, leftwith;

	currentPose = currentView->getPosable();

//	//if there are delta locks, unlock temporarily.
//	for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//	{
//		currentLock = (PosableStateLock*)currentRel;
//		currentLock->unlock(changeMask);
//	}

	//Need to do Zooming first, since certain view transformations will alter
	//translation based on zooming.
	if (zoomDirection == ZoomingIn)
		currentPose->scaleBy(pow(1.0 + zoomInRate / 100, (double)power));
	if (zoomDirection == ZoomingOut)
		currentPose->scaleBy(pow(1.0 - zoomOutRate / 100, (double)power));
	if (zoomDirection != NotZooming)
	{
		leftwith = pow(1.0 - translateRate / 100, (double)power);
		moveBy = 1.0 - leftwith;

		currentTranslation[X] = desiredTranslation[X]*moveBy;
		currentTranslation[Y] = desiredTranslation[Y]*moveBy;
		currentTranslation[Z] = desiredTranslation[Z]*moveBy;

		desiredTranslation[X] *= leftwith;
		desiredTranslation[Y] *= leftwith;
		desiredTranslation[Z] *= leftwith;

		currentPose->moveBy(currentTranslation);
	}

	//if there are delta locks, relock them.
//	for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//	{
//		currentLock = (PosableStateLock*)currentRel;
//		currentLock->restorePreviousLock();
//	}
}

void ZoomInteractor::Zoom(double amount)
{
//	PosableStateRelationship *currentRel;
//	PosableStateLock *currentLock;

//	changeMask = PosableStateLock::LockDeltaTranslate | PosableStateLock::LockDeltaScale;
//	locksOnWindow = relationshipManager->getThoseThatUpdate(currentView->getPosable(), changeMask, "PosableStateLock");

	
	//if there are delta locks, unlock temporarily.
//	for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//	{
//		currentLock = (PosableStateLock*)currentRel;
//		currentLock->unlock(changeMask);
//	}

	currentView->getPosable()->scaleBy(amount);
	
	//if there are delta locks, relock them.
//	for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//	{
//		currentLock = (PosableStateLock*)currentRel;
//		currentLock->restorePreviousLock();
//	}
}

/*---------------------------------------------------------------------------*/
/**	This function checks the passed Y value against the stored one to see if
 *	the zoom direction should be changed.  If so, the stored one may be updated
 *	to make interaction easier.
 */
void ZoomInteractor::uponThreshhold(int y)
{
	int zoomIndicator;
	int screenCenter = currentView->getViewport()[3] / 2;

	//determine which zone we are in
	if (y >= startingPos[Y] + DISTANCE_TO_THRESHHOLD)
	{
		zoomIndicator = 1;
		if (y > screenCenter + DISTANCE_TO_THRESHHOLD)
			startingPos[Y] = screenCenter;
	}
	else if (y <= startingPos[Y] - DISTANCE_TO_THRESHHOLD)
	{
		zoomIndicator = -1;
		startingPos[Y] = y + DISTANCE_TO_THRESHHOLD;
		if (y < screenCenter - DISTANCE_TO_THRESHHOLD)
			startingPos[Y] = screenCenter;
	}
	else
		zoomIndicator = 0;

	//now adjust for which direction we want to use for zooming in
	if (interactionType & SwitchDirectionOnReverseThresh)
		zoomIndicator *= -1;

	//now adjust for the state of the shift key
	if (shiftPressed)
		zoomIndicator *= -1;

	//make the adjustment.
	zoomDirection = (ZoomDirection) ((zoomDirection & TranslatingOnly) + zoomIndicator);

//	changeMask = changeMask & PosableStateLock::LockDeltaTranslate;
//	if (zoomIndicator)
//		changeMask |= PosableStateLock::LockDeltaScale;
}

/*---------------------------------------------------------------------------*/
/**	This function reverses the direction of zoom based on the status of the
 *	shift key.
 */
int ZoomInteractor::uponShift(int inputType)
{
	GLboolean previousShiftState = shiftPressed;

	if (inputType & KeyDown)
		shiftPressed = GL_TRUE;
	else
		shiftPressed = GL_FALSE;

	if (zoomDirection == NotZooming)
		return PASS_EVENT_THROUGH;

	if (interactionType & SwitchDirectionOnSomeThreshhold)
	{
		if (previousShiftState == shiftPressed)
			return SINK_EVENT;

		zoomDirection = (ZoomDirection) (zoomDirection * -1);
	}
	else // Only using shift key, no threshholds
	{
		if (zoomDirection == ZoomingIn && (inputType & KeyDown))
			zoomDirection = ZoomingOut;
		else if (zoomDirection == ZoomingOut && (inputType & KeyUp))
			zoomDirection = ZoomingIn;
	}

	return SINK_EVENT;
}

/*---------------------------------------------------------------------------*/
void ZoomInteractor::SetScale( GLfloat sval )
{
	currentView->getPosable()->setScale(sval);
}
