// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ValueDrag.cpp
/* 
 *	This class implements a mouse-dragging interactor that increments or
 *	decrements a value based on mouse motion along a specified axis.
 *
 *	Created by Matt Plumlee (mdp) 8/17/2005
 */
///////////////////////////////////////////////////////////////////////////////

#include "interactors/ValueDrag.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/Scene3D.h"

#define MIN_RATIO 0.01

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes all variables to a neutral position.
 */
ValueDrag::ValueDrag(float *valueToDrag, float axisAngle, float scaleFactor)
:Interactor("valueDrag")
{
	dragValue = valueToDrag;
	setAxisAngle(axisAngle);
	scale = scaleFactor;
	valueAlwaysPositive = false;
	flag = NULL;
	negPolarity = false;
}

/*---------------------------------------------------------------------------*/
/**	Destructor.
 */
ValueDrag::~ValueDrag()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void ValueDrag::setAxisAngle(float axisAngle)
{
	angle = axisAngle;
	sinAngle = -sin(angle * M_PI / 180);
	cosAngle = cos(angle * M_PI / 180);
}

/*---------------------------------------------------------------------------*/
/**	HandleInput handles button 1 events.
 */
int ValueDrag::HandleInput(unsigned int inputType, InputInfo *info)
{
	GLboolean startDrag = GL_FALSE;
	Window3D *currentWindow;
	int dx, dy;
	float dragAmount;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
		{
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
			startDrag = GL_TRUE;
		else if (inputType & MouseButtonUp)
		{
			return REMOVE_FROM_STREAM;
		}
	}

	/*---Handle Mouse Motion Events---*/
	else if (inputType & MouseMotion)
	{
		dx = (int)(info->x.i - startx);
		dy = (int)(info->y.i - starty);
		dragAmount = dx * cosAngle + dy * sinAngle; //dot product with unit vector representing angle
		if (scale)
			dragAmount *= scale;
		else
		{
			DYNAMIC_CAST(currentWindow, Window3D*, info->window)
			if (currentWindow)
				dragAmount *= currentWindow->getViewInfo()->getPixelSize();
		}
		if (negPolarity)
			*dragValue = startValue + dragAmount;
		else
			*dragValue = startValue - dragAmount;
		if (valueAlwaysPositive && *dragValue < 0)
			*dragValue *= -1.0;
		if (flag)
			*flag = true;
		return SINK_EVENT;
	}

	//Initialize the drag.
	if (startDrag)
	{
		startx = info->x.i;
		starty = info->y.i;
		startValue = *dragValue;
		return SINK_EVENT;
	}

	return PASS_EVENT_THROUGH;
}

