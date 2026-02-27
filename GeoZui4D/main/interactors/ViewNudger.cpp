/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ViewNudger.cpp
// 
//
//	This class implements a translation interactor for effecting small changes
//	in scen location.  This class operates on ViewManagers to effect the
//	viewpoint changes.
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "interactors/ViewNudger.h"
#include "widgets/Sv_Standards.h"
#include "widgets/Sv_Matrix4.h"
#include "engine/Scene3D.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include <tcl.h>

#define MAX_TOOL_COMMAND 2048

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes all variables to a neutral position, while
 *	getting a handle onthe target ViewManager.
 */
ViewNudger::ViewNudger(ViewManager *targetView)
:Interactor("viewNudger")
{
	/*---Initialize interaction values---*/
	keyPressed = KeyNone;
	shiftPressed = GL_FALSE;
	cntrlPressed = GL_FALSE;

	/*---set the target ViewManager---*/
	currentView = targetView;
	currentPose = currentView->getPosable();
}

/*---------------------------------------------------------------------------*/
/**	Destructor.  Resets the continuous change flag if we were in the middle of
 *	interacting.
 */
ViewNudger::~ViewNudger( void )
{
	if (keyPressed != KeyNone)
		currentPose->setContinuousChange(GL_FALSE);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	HandleInput handles keypad and idle events.
 */
int ViewNudger::HandleInput(unsigned int inputType, InputInfo *info)
{
	int key;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction && keyPressed != KeyNone)
		{
			keyPressed = KeyNone;
			shiftPressed = GL_FALSE;
			cntrlPressed = GL_FALSE;
			currentPose->setContinuousChange(GL_FALSE);
		}
		return PASS_EVENT_THROUGH;
	}

	/*---Handle idle event---*/
	if (inputType == NoInput && keyPressed != KeyNone)
	{
		continueTranslate();
		return PASS_EVENT_THROUGH;
	}

    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

 	/*---Handle Keyboard Events---*/
	if (inputType & KeyAction)
	{
		key = info->val.i;
		if ((key & KeySpecial) || key == '{' || key == '}' || key == '9' || key == '0')
		{
			if (inputType & KeyDown)
				return startTranslate(info);
			else
			{
				if (key == keyPressed)
				{
					keyPressed = KeyNone; //end translate
					currentPose->setContinuousChange(GL_FALSE);
				}
				else
				{
					if (key == KeyLeftShift || key == KeyRightShift)
						shiftPressed = GL_FALSE;
					if (key == KeyLeftControl || key == KeyRightControl)
						cntrlPressed = GL_FALSE;
					if (key == KeyLeftAlt || key == KeyRightAlt)
						cntrlPressed = GL_FALSE;
					return PASS_EVENT_THROUGH;
				}
			}
			return SINK_EVENT;
		}
	}

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	This function sets up a translation operation, but does not perform
 *	any of that operation.
 */
GLboolean ViewNudger::startTranslate(InputInfo *info)
{
	int key = info->val.i;
	SpecialKey previousKeyPressed = keyPressed;

	if ((key >= KeyHome && key <= KeyEnd) || key == KeyNoArrow || key == '{' || key == '}' || key == '9' || key == '0')
		keyPressed = (SpecialKey)key;
	else if (key == KeyLeftShift || key == KeyRightShift)
		shiftPressed = (info->type & KeyDown)? GL_TRUE : GL_FALSE;
	else if (key == KeyLeftControl || key == KeyRightControl)
		cntrlPressed = (info->type & KeyDown)? GL_TRUE : GL_FALSE;

	if (keyPressed != KeyNone)
	{
		if (previousKeyPressed == KeyNone)
		{
			currentPose->setContinuousChange(GL_TRUE);
//			locksOnWindow = relationshipManager->getThoseThatUpdate(currentView->getPosable(), PosableStateLock::LockDeltaTranslate | PosableStateLock::LockDeltaRotate, "PosableStateLock");
		}
		return SINK_EVENT;
	}
	else
		return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	This function performs another iteration of a zoom and translation.
 */
void ViewNudger::continueTranslate()
{
	Sv_Matrix4 transmatrix;
	float currentTranslation[4] = {0.0, 0.0, 0.0, 1.0}; // The amount we are translating by.
	float finalTranslation[4];
	double step;
	gz4d::Point<double> orient;
//	PosableStateRelationship *currentRel;
//	PosableStateLock *currentLock;

	if (shiftPressed && (keyPressed != KeyNoArrow))
	{
		step = (float)(0.1 * Scene3D::timeForLastFrame()/40.0);
	}
	else
	{
		step = (float)(0.001/currentPose->getScale() * Scene3D::timeForLastFrame()/40.0);
	}

	if (cntrlPressed)
		step *= 20;

	if (keyPressed == '{' || keyPressed == '}' || keyPressed == '9' || keyPressed == '0') //cntrl doesn't seem to work with this...
	{
		PosableStateBase *perspectivePose = currentView->getPerspectivePosable();
		float currentCyclopean = perspectivePose->getScale();
		if (keyPressed == '{' || keyPressed == '9' )
			currentCyclopean -= step * 0.1f;
		else
			currentCyclopean += step * 0.1f;
		if (currentCyclopean < 1.0f)
			currentCyclopean = 1.0f;
		else if (currentCyclopean > 20.0f)
			currentCyclopean = 20.0f;
		perspectivePose->setScale(currentCyclopean);
	}

	//Figure out our direction vector
	if (keyPressed == KeyHome || keyPressed == KeyUpArrow || keyPressed == KeyPageUp)
			currentTranslation[Z] += step;
	if (keyPressed == KeyEnd || keyPressed == KeyDownArrow || keyPressed == KeyPageDown)
			currentTranslation[Z] -= step;

	if (keyPressed == KeyHome || keyPressed == KeyLeftArrow || keyPressed == KeyEnd)
			currentTranslation[X] -= step;
	if (keyPressed == KeyPageUp || keyPressed == KeyRightArrow || keyPressed == KeyPageDown)
			currentTranslation[X] += step;

	if (keyPressed == KeyNoArrow)
	{
		if (shiftPressed)
			currentTranslation[Y] -= step;
		else
			currentTranslation[Y] += step;
	}

	//if there are delta locks, unlock temporarily.
//	for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//	{
//		currentLock = (PosableStateLock*)currentRel;
//		if (shiftPressed && (keyPressed != KeyNoArrow)) //rotate instead of translating
//			currentLock->unlock(PosableStateLock::LockDeltaRotate);
//		else
//			currentLock->unlock(PosableStateLock::LockDeltaTranslate);
//	}

	if (shiftPressed && (keyPressed != KeyNoArrow)) //rotate instead of translating
	{
		currentPose->rotateBy(currentTranslation[Z], currentTranslation[X]);
	}
	else //translate
	{
		//Now account for the current view
		glPushMatrix();
		glLoadIdentity();
		orient = currentPose->getOrientationAxis();
		glRotatef(orient[Roll], 0.0, 1.0, 0.0);
		glRotatef(orient[Pitch], 1.0, 0.0, 0.0);
		glRotatef(orient[Yaw], 0.0, 0.0, 1.0);
		transmatrix.GetGLMatrix();
		glPopMatrix();

		transmatrix.Multiply(currentTranslation, finalTranslation);
		
                gz4d::Point<double> t(finalTranslation[0],finalTranslation[1],finalTranslation[2]);
		currentPose->moveBy(t);
	}

	//if there are delta locks, relock them.
//	for (currentRel = locksOnWindow->first(); currentRel; currentRel = locksOnWindow->next())
//	{
//		currentLock = (PosableStateLock*)currentRel;
//		currentLock->restorePreviousLock();
//	}
}
