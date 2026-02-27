/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// DoubleClickInteractor.cpp
// 
//
//	This class implements a double-click interactor for properly introducing
//	double-clicks to this environment.
///////////////////////////////////////////////////////////////////////////////

#include "interactors/DoubleClickInteractor.h"
#include "engine/InputMediator.h"

#define DEFAULT_MAX_MOVE 1
#define DEFAULT_MAX_IDLE 25

static DoubleClickInteractor localInstance;
DoubleClickInteractor *doubleClickInteractor = &localInstance;
const EventType *DoubleClickInteractor::clickInputType = Interactor::newEventType("click", Interactor::DataAsPointer);
const EventType *DoubleClickInteractor::doubleClickInputType = Interactor::newEventType("doubleClick", Interactor::DataAsPointer);

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes all variables to a neutral position.
 */
DoubleClickInteractor::DoubleClickInteractor()
:Interactor("doubleClick")
{
	/*---Initialize interaction values---*/
	maxMove = DEFAULT_MAX_MOVE;
	maxIdle = DEFAULT_MAX_IDLE;
	doubleClickButton = clickButton = 0;
}

/*---------------------------------------------------------------------------*/
/**	Destructor.
 */
DoubleClickInteractor::~DoubleClickInteractor( void )
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	HandleInput handles button and mouse events to determine if it should
 *	make the current event a double-click event or not.
 */
int DoubleClickInteractor::HandleInput(unsigned int inputType, InputInfo *info)
{
	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
			reset();
		return PASS_EVENT_THROUGH;
	}

	/*---Handle idle event---*/
	if (inputType == NoInput && clickButton)
	{
		idleCount++;
		if (clickButton)
			return PASS_EVENT_THROUGH;
		return PASS_EVENT_THROUGH;
	}

    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle Mouse Button Events---*/
    if (inputType & MouseButtons)    //Is it a button2 event?
    {
		if (inputType & MouseDouble)
			return PASS_EVENT_THROUGH;

		if (inputType & MouseButtonDown)
		{
			if (doubleClickButton == (inputType & MouseButtons))
			{
				clickX = doubleClickX;
				clickY = doubleClickY;
				if (test(info))
				{
					info->type |= MouseDouble;
				}
				else
					doubleClickButton = 0;
			}
			clickButton = (inputType & MouseButtons);
			clickX = info->x.i;
			clickY = info->y.i;
			idleCount = 0;
		}
		else if (inputType & MouseButtonUp)
		{
			if (doubleClickButton == (inputType & MouseButtons))
			{
				info->type |= MouseDouble;
				if (test(info))
					globalInputMediator->Trigger(doubleClickInputType->value, info);
				doubleClickButton = 0;
			}
			else
			{
				if (clickButton == (inputType & MouseButtons))
				{
					doubleClickButton = (inputType & MouseButtons);
					if (test(info))
						globalInputMediator->Trigger(clickInputType->value, info);
					doubleClickX = info->x.i;
					doubleClickY = info->y.i;
					idleCount = 0;
				}
				else
					doubleClickButton = 0;
			}
			clickButton = 0;
		}
	}
    if ((inputType & MouseMotion) && clickButton && !test(info))
		reset();

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	This function resets the state, cancelling any click or double-click
 *	information.
 */
void DoubleClickInteractor::reset()
{
	doubleClickButton = clickButton = 0;
}

/*---------------------------------------------------------------------------*/
/**	This function returns GL_TRUE if the info is within the prescribed bounds
 *	for a click or double click.
 */
int DoubleClickInteractor::test(InputInfo *info)
{
	int diffX, diffY;

	diffX = info->x.i - clickX;
	if (diffX < 0)
		diffX = -diffX;
	diffY = info->y.i - clickY;
	if (diffY < 0)
		diffY = -diffY;
	if (diffX <= maxMove && diffY <= maxMove
		/*&& idleCount <= maxIdle*/)
		return 1;
	else
		return 0;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This function sets the maximum amount that the mouse can move in the
 *	middle of a double-click in x or y (before being considered two separate
 *	clicks).
 */
void DoubleClickInteractor::setMaxMovement(int pixels)
{
	maxMove = pixels;
}

/*---------------------------------------------------------------------------*/
/**	This function sets the maximum number of idle cycles that can pass
 *	before two successive clicks are considered two separate clicks rather
 *	than a double-click.
 */
void DoubleClickInteractor::setMaxIdle(int idleCycles)
{
	maxIdle = idleCycles;
}
