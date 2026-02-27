/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// PlanarPointHandle.cpp
///////////////////////////////////////////////////////////////////////////////
/**
 *	This class implement a way of interacting with point information that
 *	emphasizes motion in the xy-plane.  The user can always drag a point along
 *	the xy-plane of a PlanarPointHandle, and can additionally drag the plane
 *	up and down in the z-direction.
 *	This class implements the following responsibilities:
 *	*	Rendering of plane for selected point
 *	*	Interaction for moving selected point or plane
 *
 *	Created by mdp 8/5/2002
 */
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "widgets/Sv_Standards.h"
#include "interactors/PlanarPointHandle.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/Scene3D.h"
#include "engine/TranslatableWrapper.h"
#include "engine/Object.h"
#include "interactors/DoubleClickInteractor.h"
#include "widgets/Sv_Widgets.h"

#define POWER_STEP 2.0

/*---------------------------------------------------------------------------*/
/**	Constructor
 */
PlanarPointHandle::PlanarPointHandle()
{
	draggingVertical = GL_FALSE;
	lastStep = 1.0;

	hSlider = new Sv_Slider3(0.0, 20.0, 0.0,
							 0, 0, -180, 400, GL_FALSE);
	hSlider->setRingColor(0.4f, 0.8f, 0.4f);
	hSlider->setRingNotchColor(0.3f, 0.5f, 0.3f);
	hSlider->SetInactiveColor(120, 250, 120);
	hSlider->SetHighlightColor(50, 200, 50);
	usingWidgets = GL_FALSE;
	widgetsPicked = GL_FALSE;
	scaling = GL_FALSE;
	visible = GL_TRUE;
	widgetOffsetX = widgetOffsetY = 0.0f;
	drawModeDefault = DrawDefault;
	drawMode = drawModeDefault;
}

/*---------------------------------------------------------------------------*/
/**	Destructor
 */
PlanarPointHandle::~PlanarPointHandle()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	Implements both the dragging of a point on the plane and the dragging
 *	of the plane.  Mouse down events are handled through the picking mechanism.
 */
int PlanarPointHandle::HandleInput(unsigned int inputType, InputInfo *info)
{
	float scale, amount;
	double newX, newY, newZ;
	Window3D *window;

	if (inputType == NoInput)
	{
		if (widgetsPicked) //Idle event
		{
			amount = hSlider->updateAzimuth();
			targetPoint->setOrientation(0, amount, ChangeRotateYaw);
		}
		return PointHandle::HandleInput(inputType, info);
	}

	if ((inputType & EnumeratedAction)/* || widgetsPicked*/)
		return PointHandle::HandleInput(inputType, info);

	/*---Handle idle event---*/
    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle Mouse Button Events---*/
    if (inputType & MouseMotion)
    {
		if (!targetPoint)
		{
			reset();
			return PASS_EVENT_THROUGH;
		}
		DYNAMIC_CAST(window, Window3D*, info->window)
		if (!window)
			return PASS_EVENT_THROUGH;
		if (draggingVertical)
		{
			//roughly approximate right feel
			scale = (float) (0.0005f / window->getViewInfo()->getPosable()->getScale());
			newZ = startZ + scale * (starty - info->y.i);
			targetPoint->setPosition(0.0, 0.0, newZ, ChangeTranslateZ);
		}
		else if (widgetsPicked)
		{
			hSlider->setAzimuthVel((info->x.i - startx) / 2);
			startx = info->x.i;
		}
		else if (scaling)
		{
			targetPoint->setScale((float)(startScale * pow(1.03, starty - info->y.i)));
		}
		else
		{
			if (!layoutMode)
			{
				if (currentWin != window)
                                    std::cerr << "Look out: editing window changed without software being fully aware...\n";
				currentWin = window;
			}
			gz4d::Point<double> vec = targetPoint->getPosition();
                        if (vec.IsValid())
				newZ = vec[Z];
			else
				newZ = 0;

			if (window->getCursorPosOnPlane(newX, newY, newZ))
				targetPoint->setPosition(newX + widgetOffsetX, newY + widgetOffsetY, 0.0, ChangeTranslateX | ChangeTranslateY);
		}
		return PASS_EVENT_THROUGH; //allow other actions to occur
	}
	return PointHandle::HandleInput(inputType, info);
}

/*---------------------------------------------------------------------------*/
void PlanarPointHandle::drawSupportTools(Window3D *inWindow, gz4d::Point<double> const &pointPos)
{
    float step = 1.0, desiredStep;

    if (visible)
    {
        if (drawMode & DrawGrid)
        {
            //determine the grid step
            desiredStep = (float) (0.03 / inWindow->getViewInfo()->getPosable()->getScale());
            while (desiredStep > POWER_STEP)
            {
                desiredStep /= POWER_STEP;
                step *= POWER_STEP;
            }
            while (desiredStep < 1.0)
            {
                desiredStep *= POWER_STEP;
                step /= POWER_STEP;
            }
            lastStep = step;
        }

        PointHandle::drawSupportTools(inWindow, pointPos);
    }
}

/*---------------------------------------------------------------------------*/
void PlanarPointHandle::drawPickableTools(Window3D *inWindow, gz4d::Point<double> const &pointPos)
{
	int dim;
	double start[2], stop[2], step = lastStep;
	float lineWidth, current, height;
	float x, y, z, length, halfLength, quarterLength, scale=1.0, radius=0.0;
	gz4d::Point<double> orient;

	if (visible)
	{
		glDisable(GL_LIGHTING);
		x = pointPos[X];
		y = pointPos[Y];
		z = pointPos[Z];

		glGetFloatv(GL_LINE_WIDTH, &lineWidth);
		//define the grid
		for (dim = X; dim <= Y; dim++)
		{
			start[dim] = (pointPos[dim] - 5.0 * step);
			stop[dim] = (pointPos[dim] + 5.01 * step);
		}

		if (drawMode & DrawGrid)
		{

			//draw the grid
			glPushName(PositionHandle);
			glColor3f(0.7f, 0.7f, 0.7f);
			glLineWidth(2.0);
			glBegin(GL_LINES);
			for (current = start[Y]; current < stop[Y]; current += step)
			{
				glVertex3d(start[X], current, z);
				glVertex3d(stop[X], current, z);
			}

			for (current = start[X]; current < stop[X]; current += step)
			{
				glVertex3f(current, start[Y], z);
				glVertex3f(current, stop[Y], z);
			}
			glEnd();

		}

		if ((drawMode & DrawGrid) || (drawMode & DrawVerticalTools))
		{
			//set up the new measurement parameters
			scale = (float) (0.09f / inWindow->getPoseInfo()->getScale());
			if (drawMode & DrawGrid)
				radius = (float)(2.0 * step);
			else
				radius = scale * 0.5f;
			for (dim = X; dim <= Y; dim++)
			{
				start[dim] = (float) (pointPos[dim] - radius);
				stop[dim] = (float) (pointPos[dim] + radius);
			}
		}

		if (drawMode & DrawGrid)
		{
			//Draw the selectable crosshairs
			glColor3f(0.7f, 0.1f, 0.1f);
			length = 2 * step;
			height = z + step * 0.01;
			glLineWidth(3.0);
			glBegin(GL_LINES);
				glVertex3f(start[X] - length, start[Y], height);
				glVertex3f(start[X] - length, stop[Y], height);
				glVertex3f(start[X] - length, y, height);
				glVertex3f(start[X] + step, y, height);

				glVertex3f(stop[X] + length, start[Y], height);
				glVertex3f(stop[X] + length, stop[Y], height);
				glVertex3f(stop[X] + length, y, height);
				glVertex3f(stop[X] - step, y, height);

				glVertex3f(start[X], start[Y] - length, height);
				glVertex3f(stop[X], start[Y] - length, height);
				glVertex3f(x, start[Y] - length, height);
				glVertex3f(x, start[Y] + step, height);

				glVertex3f(start[X], stop[Y] + length, height);
				glVertex3f(stop[X], stop[Y] + length, height);
				glVertex3f(x, stop[Y] + length, height);
				glVertex3f(x, stop[Y] - step, height);
			glEnd();

			//Draw the selectable flat squares
			length = 4 * step;
			halfLength = step * 0.5f;
			glBegin(GL_QUADS);
				glVertex3f(x - length - halfLength, y - halfLength, height);
				glVertex3f(x - length - halfLength, y + halfLength, height);
				glVertex3f(x - length + halfLength, y + halfLength, height);
				glVertex3f(x - length + halfLength, y - halfLength, height);

				glVertex3f(x + length - halfLength, y - halfLength, height);
				glVertex3f(x + length - halfLength, y + halfLength, height);
				glVertex3f(x + length + halfLength, y + halfLength, height);
				glVertex3f(x + length + halfLength, y - halfLength, height);

				glVertex3f(x - halfLength, y - length - halfLength, height);
				glVertex3f(x - halfLength, y - length + halfLength, height);
				glVertex3f(x + halfLength, y - length + halfLength, height);
				glVertex3f(x + halfLength, y - length - halfLength, height);

				glVertex3f(x - halfLength, y + length - halfLength, height);
				glVertex3f(x - halfLength, y + length + halfLength, height);
				glVertex3f(x + halfLength, y + length + halfLength, height);
				glVertex3f(x + halfLength, y + length - halfLength, height);
			glEnd();
		}
		else if (drawMode & DrawSimpleArrows)
		{
			glLineWidth(5.0);
			glColor3f(0.8f, 0.2f, 0.2f);
			scale = (float) (0.01f / inWindow->getPoseInfo()->getScale());
			glPushName(PositionHandle);
			glPushMatrix();
			glTranslatef(x, y, z);
			glScalef(scale, scale, scale);
			glBegin(GL_LINES);
				glVertex3f(0, 1.5, 0);
				glVertex3f(0, 3, 0);
				glVertex3f(0, -1.5, 0);
				glVertex3f(0, -3, 0);
				glVertex3f(1.5, 0, 0);
				glVertex3f(3, 0, 0);
				glVertex3f(-1.5, 0, 0);
				glVertex3f(-3, 0, 0);
			glEnd();
			glBegin(GL_TRIANGLES); //arrow heads
				glVertex3f(0, 3.2f, 0);
				glVertex3f(0.5f, 2.5f, 0);
				glVertex3f(-0.5f, 2.5f, 0);
				glVertex3f(0, -3.2f, 0);
				glVertex3f(0.5f, -2.5f, 0);
				glVertex3f(-0.5f, -2.5f, 0);
				glVertex3f(3.2f, 0, 0);
				glVertex3f(2.5f, 0.5f, 0);
				glVertex3f(2.5f, -0.5f, 0);
				glVertex3f(-3.2f, 0, 0);
				glVertex3f(-2.5f, 0.5f, 0);
				glVertex3f(-2.5f, -0.5f, 0);
			glEnd();
			glPopMatrix();
			scale *= 5.0;
		}
		else
			glPushName(0);

		//Draw the grid-level (height) control
		if (drawMode & DrawVerticalTools)
		{
                    glPushAttrib(GL_POINT_BIT);
			glPointSize(7.0);
			//glColor3f(0.4f, 0.9f, 0.4f);
			glLineWidth(1.0);
			glColor3f(0.6f, 0.9f, 0.6f);
			glLoadName(VerticalHandle);
	//		glBegin(GL_POINTS);
	//			glVertex3f(start[X], start[Y], z);
	//			glVertex3f(stop[X], start[Y], z);
	//			glVertex3f(start[X], stop[Y], z);
	//			glVertex3f(stop[X], stop[Y], z);
	//		glEnd();
			length = 10 * radius;
			//length = radius * 0.01f;
			glBegin(GL_LINES); //posts
				glVertex3f(start[X], start[Y], z - length);
				glVertex3f(start[X], start[Y], z + step);
				glVertex3f(stop[X], start[Y], z - length);
				glVertex3f(stop[X], start[Y], z + step);
				glVertex3f(start[X], stop[Y], z - length);
				glVertex3f(start[X], stop[Y], z + step);
				glVertex3f(stop[X], stop[Y], z - length);
				glVertex3f(stop[X], stop[Y], z + step);
			glEnd();
			glLineWidth(3.0);
			radius = scale;
			length = radius / 9.0f;
			halfLength = radius / 15.0f;
			quarterLength = radius / 30.0f;
			glBegin(GL_LINES); //arrow bodies
				glVertex3f(start[X], start[Y], z - length);
				glVertex3f(start[X], start[Y], z + length);
				glVertex3f(stop[X], start[Y], z - length);
				glVertex3f(stop[X], start[Y], z + length);
				glVertex3f(start[X], stop[Y], z - length);
				glVertex3f(start[X], stop[Y], z + length);
				glVertex3f(stop[X], stop[Y], z - length);
				glVertex3f(stop[X], stop[Y], z + length);
			glEnd();
			length = radius / 7.5f;
			glBegin(GL_TRIANGLES); //arrow heads
				glVertex3f(start[X] - quarterLength, start[Y] - quarterLength, z + halfLength);
				glVertex3f(start[X], start[Y], z + length);
				glVertex3f(start[X] + quarterLength, start[Y] + quarterLength, z + halfLength);
				glVertex3f(start[X] - quarterLength, start[Y] - quarterLength, z - halfLength);
				glVertex3f(start[X], start[Y], z - length);
				glVertex3f(start[X] + quarterLength, start[Y] + quarterLength, z - halfLength);

				glVertex3f(stop[X] + quarterLength, start[Y] - quarterLength, z + halfLength);
				glVertex3f(stop[X], start[Y], z + length);
				glVertex3f(stop[X] - quarterLength, start[Y] + quarterLength, z + halfLength);
				glVertex3f(stop[X] + quarterLength, start[Y] - quarterLength, z - halfLength);
				glVertex3f(stop[X], start[Y], z - length);
				glVertex3f(stop[X] - quarterLength, start[Y] + quarterLength, z - halfLength);

				glVertex3f(start[X] + quarterLength, stop[Y] - quarterLength, z + halfLength);
				glVertex3f(start[X], stop[Y], z + length);
				glVertex3f(start[X] - quarterLength, stop[Y] + quarterLength, z + halfLength);
				glVertex3f(start[X] + quarterLength, stop[Y] - quarterLength, z - halfLength);
				glVertex3f(start[X], stop[Y], z - length);
				glVertex3f(start[X] - quarterLength, stop[Y] + quarterLength, z - halfLength);

				glVertex3f(stop[X] - quarterLength, stop[Y] - quarterLength, z + halfLength);
				glVertex3f(stop[X], stop[Y], z + length);
				glVertex3f(stop[X] + quarterLength, stop[Y] + quarterLength, z + halfLength);
				glVertex3f(stop[X] - quarterLength, stop[Y] - quarterLength, z - halfLength);
				glVertex3f(stop[X], stop[Y], z - length);
				glVertex3f(stop[X] + quarterLength, stop[Y] + quarterLength, z - halfLength);
			glEnd();
                        glPopAttrib();
		}

		glLineWidth(1.0);

		orient = inWindow->getPoseInfo()->getOrientationAxis();

		if (targetPoint && targetPoint->getScale() && (drawMode & DrawScaleTools))
		{
			glColor3f(0.4f, 0.9f, 0.4f);
			glLoadName(ScaleHandle);
			glPushMatrix();
			glTranslatef(x, y, z + (scale * 0.5f));
			glRotatef(-orient[Y], 0.0, 0.0, 1.0);
			glRotatef(-orient[X], 1.0, 0.0, 0.0);
			glScalef(scale, scale, scale);
			glBegin(GL_POINTS);
				glVertex3f(0, 0, 0);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f(-.2f, 0, 0.05f);
				glVertex3f(0, 0, .1f);
				glVertex3f(0, 0, .1f);
				glVertex3f(.2f, 0, 0.05f);

				glVertex3f(-0.1f, 0, -0.05f);
				glVertex3f(0, 0, -.1f);
				glVertex3f(0, 0, -.1f);
				glVertex3f(0.1f, 0, -0.05f);
			glEnd();
			glPopMatrix();
		}

		if (targetPoint && targetPoint->getOrientation() && (drawMode & DrawRotationTools))
		{
			glLoadName(RotationHandle);
			usingWidgets = GL_TRUE;
			glPushMatrix();
			glTranslatef(x, y, z);
			glRotatef(-orient[Y], 0.0, 0.0, 1.0);
			glRotatef(90.0, 1.0, 0.0, 0.0);
			if (drawMode & DrawSimpleArrows)
			{
				scale /= 400.0;
				glScalef(scale,scale,scale);
			}
			else
			{
				step *= 0.005f;
				glScalef(step, step, step);
			}
			hSlider->Draw();
			glPopMatrix();
		}
		glPopName();

		glEnable(GL_LIGHTING);
		glLineWidth(lineWidth);
		PointHandle::drawPickableTools(inWindow, pointPos);
	}
}

/*---------------------------------------------------------------------------*/
void PlanarPointHandle::toolAction(int pickType, InputInfo *inputInfo,
								   GLuint *names, int hits)
{
	int name;
	double x, y, newZ;

	if (inputInfo->type & MouseButton1)
	{
		name = names[2];
		if (name == VerticalHandle)
		{
			draggingVertical = GL_TRUE;
			startZ = targetPoint->getPosition()[Z];
		}
		else if (name == RotationHandle)
		{
			widgetsPicked = GL_TRUE;
			hSlider->setAzimuth(targetPoint->getOrientationAxis()[Yaw]);
			hSlider->HighlightOn();
		}
		else if (name == ScaleHandle)
		{
			scaling = GL_TRUE;
			startScale = targetPoint->getScale();
		}

		startx = inputInfo->x.i;
		starty = inputInfo->y.i;

		widgetOffsetX = widgetOffsetY = 0.0;
		if (name == PositionHandle)
		{
			//find the world offset so we can move as if we grabbed on to the red handle.
			if (currentWin)
			{
				newZ = targetPoint->getPosition()[Z];
				if (currentWin->getCursorPosOnPlane(x, y, newZ))
				{
					widgetOffsetX = targetPoint->getPosition()[X] - (float)x;
					widgetOffsetY = targetPoint->getPosition()[Y] - (float)y;
				}
			}
		}

		//make the interface react appropriately
		doubleClickInteractor->reset();
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
	else
		PointHandle::toolAction(pickType, inputInfo, names, hits);
}

/*---------------------------------------------------------------------------*/
void PlanarPointHandle::cancelAction()
{
	draggingVertical = GL_FALSE;
	scaling = GL_FALSE;
	usingWidgets = GL_FALSE;
	if (widgetsPicked)
	{
		hSlider->resetAzimuth();
		hSlider->HighlightOff();
	}
	widgetsPicked = GL_FALSE;
	widgetOffsetX = widgetOffsetY = 0.0;
	PointHandle::cancelAction();
}

/*---------------------------------------------------------------------------*/
int PlanarPointHandle::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	const char *message = argv[0];

	if (argc >= 1)
	{
		if (!strcmp(message, "hide"))
		{
			visible = GL_FALSE;
			globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
			return TCL_OK;
		}
		if (!strcmp(message, "show"))
		{
			visible = GL_TRUE;
			globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
			return TCL_OK;
		}
	}
	return PointHandle::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int PlanarPointHandle::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "hide show ", NULL);
	return PointHandle::list(interp);
}
