// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// PointHandle.h
///////////////////////////////////////////////////////////////////////////////
/**
 *	This is the interface for classes that implement various ways of
 *	interacting with point information, especially picking and modifying
 *	a point's location (but rotation, scale, others are possibilities as
 *	well).
 *	This class implements the following responsibilities:
 *	*	Default rendering for point selection
 *	*	Attachment to point information through a PosableStateBase reference
 *		or a PosableStateWrapper.
 *
 *	Created by mdp 8/2/2002
 */
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "widgets/Sv_Standards.h"
#include "interactors/PointHandle.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "engine/Scene3D.h"
#include "engine/TranslatableWrapper.h"
#include "engine/PosableWrapperSupport.h"
#include "engine/Object.h"
#include "interactors/DoubleClickInteractor.h"
#include "interactors/InputTentacle.h"
#include "utilities/DottedAddress.h"
#include <gz4d/glo/Vertex.h>
//#include <GL/glut.h>

/*---------------------------------------------------------------------------*/
/**	Constructor
 */
PointHandle::PointHandle(int type, GLboolean constructWrapper)
: Pickable(type)
{
	if (constructWrapper)
	{
		wrapper = new TranslatableWrapper();
		wrapper->setNumDimensions(3);
		wrapper->publishName("editPoint");
	}
	else
		wrapper = NULL;

	keyGrabber = new InputTentacle("keyGrabber");
	keyGrabber->setOwner(this);
	keyGrabber->setFilter(KeyAction, GL_TRUE);
	reset();
	drawModeDefault = 0;
	drawMode = 0;
	layoutMode = false;
	internalPosable = false;
	keyPressed = KeyNone;
}

/*---------------------------------------------------------------------------*/
/**	Destructor
 */
PointHandle::~PointHandle()
{
	if (wrapper)
		delete wrapper;
}

/*---------------------------------------------------------------------------*/
void PointHandle::publishName(std::string name)
{
	DottedAddress *oAddress;

	Interactor::publishName(name);

	oAddress = keyGrabber->getAddress();
	if (oAddress)
		oAddress->reparent(getAddress());
	oAddress = wrapper->getAddress();
	if (oAddress)
		oAddress->reparent(getAddress());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	Children should define HandleInput to handles mouse motion.  A default
 *	behavior for button release events and BreakAction is provided here.
 *	Mouse down events are handled through the picking mechanism.
 */
int PointHandle::HandleInput(unsigned int inputType, InputInfo *info)
{
	int key, index = 0;
	float amount = 1.0;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
		{
			cancelAction();
			return REMOVE_FROM_STREAM;
		}
		return PASS_EVENT_THROUGH;
	}

	keyGrabber->ignoreBefore(info->serialNumber);

	/*---Handle idle event---*/
	if (inputType == NoInput && keyPressed != KeyNone)
	{
		continueResponse();
		return PASS_EVENT_THROUGH;
	}

	/*---Handle idle event---*/
    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle keyboard events (usually from tentacle)---*/
	if (inputType & KeyAction)
	{
		currentWin = Window3D::cast(info->window);
		if (!currentWin || !targetPoint)
			return PASS_EVENT_THROUGH;
		key = info->val.i;
		if (key & KeySpecial)
		{
			if ((key == KeyUpArrow || key == KeyDownArrow) && targetPoint)
			{
				if (inputType & KeyDown)
				{
					if (keyPressed == KeyNone)
						targetPoint->setContinuousChange(GL_TRUE);
					keyPressed = key;
				}
				else if (key == keyPressed)
				{
					keyPressed = KeyNone;
					targetPoint->setContinuousChange(GL_FALSE);
					currentWin = NULL;
				}
				return SINK_EVENT;
			}
			else if (key == KeyLeftControl || key == KeyRightControl)
			{
				if (inputType & KeyDown)
					cntrlPressed = GL_TRUE;
				else
					cntrlPressed = GL_FALSE;
				return SINK_EVENT;
			}
			else if (key == KeyLeftShift || key == KeyRightShift)
			{
				if (inputType & KeyDown)
					shiftPressed = GL_TRUE;
				else
					shiftPressed = GL_FALSE;
				return SINK_EVENT;
			}
			//don't return in case some special keys are handled below
		}
		if (inputType & KeyDown)
		{
			if (cntrlPressed)
			{
				amount = 10;
				index = 2;
			}

			switch(key)
			{
				case KeyLeftArrow:
					wrapper->incrementIndex(-amount);
					break;
				case KeyHome:
					wrapper->setIndex(0, index);
					break;
				case KeyRightArrow:
					wrapper->incrementIndex(amount);
					break;
				case KeyEnd:
					wrapper->setIndex("end", index);
					break;
				case KeyPageUp:
					wrapper->incrementIndex(amount, 2);
					break;
				case KeyPageDown:
					wrapper->incrementIndex(-amount, 2);
					break;
				case KeyInsert:
					if (shiftPressed)
					{
						wrapper->insertValue(GL_TRUE, index);
						wrapper->incrementIndex(0);
					}
					else
					{
						wrapper->insertValue(GL_FALSE, index);
						wrapper->incrementIndex(1);
					}
					break;
				case KeyDelete:
					wrapper->deleteValue(index);
					wrapper->incrementIndex(0);
					break;
				case KeyEsc:
					if (layoutMode)
					{
						wrapper->deleteValue();
						wrapper->incrementIndex(-1);
						endLayout();
						return SINK_EVENT;
					}
					break;
				case 'l':
					if (!layoutMode)
					{
						cancelAction();
						beginLayout(Object::getSelectedObject());
						wrapper->insertValue(GL_FALSE, index);
						wrapper->incrementIndex(1);
						return SINK_EVENT;
					}
				default:
					return PASS_EVENT_THROUGH;
			}
			globalInputMediator->Trigger(Scene3D::objectChangedEvent());
			return SINK_EVENT;
		}
		return PASS_EVENT_THROUGH;
	}

	/*---Handle Mouse Button Events---*/
    if (inputType & MouseButton1)    //Is it a button1 event?
    {
		if (inputType & MouseButtonUp)
		{
			if (layoutMode)
			{
				wrapper->insertValue(GL_FALSE, index);
				wrapper->incrementIndex(1);
			}
			else
			{
				cancelAction();
				return REMOVE_FROM_STREAM;
			}
		}
		else if ((inputType & MouseButtonDown) && layoutMode)
		{
			doubleClickInteractor->reset();//foil any pick-clicking
		}
	}
	else if (layoutMode && (inputType & MouseButton3) && (inputType & MouseButtonDown))
	{
		endLayout();
		return SINK_EVENT;
	}
	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
void PointHandle::continueResponse()
{
	float scale = 1.0f;
	PosableStateInfo state;

	if (targetPoint)
	{
		if (keyPressed == KeyUpArrow || keyPressed == KeyDownArrow)
		{
			if (currentWin)
				scale = currentWin->getPoseInfo()->getScale();
			if (cntrlPressed)
				scale /= 10.0f;
			if (keyPressed == KeyUpArrow)
				state.position[Z] = 1.0f / scale;
			else
				state.position[Z] = -1.0f / scale;
			targetPoint->offsetPoseInfoBy(state, ChangeTranslateZ);
		}
	}
}

/*---------------------------------------------------------------------------*/
int PointHandle::pickAction(Pickable::PickType pickType, InputInfo *info, GLuint *names, int hits)
{
	PosableWrapperSupport *support;
	unsigned int numNames, currentName, dim, isOwnTool;

	if (layoutMode)
		return GL_TRUE;

	currentWin = Window3D::cast(info->window);
	if (!currentWin || dragging)
		return GL_FALSE;

	//set us up for dragging the point around
	currentWin->inputMediator.PushInteractor(this);
	Scene3D::setContinuousChange(TRUE);
	dragging = GL_TRUE;

	//did they select a point or a tool on the handle?
	numNames = names[0];
	isOwnTool = names[1];
	if (isOwnTool)
		toolAction(pickType, info, names, hits);
	else
	{
		//get the object to respond to how it was picked, if necessary
		Object *ob = Object::getSelectedObject();
		if (ob)
		{
			ob->pickAction(pickType, info, names, hits);
		}

		DYNAMIC_CAST(support, PosableWrapperSupport *, ob);

		//latch on to the selected point
		if (numNames < 2 || !support)
		{
			targetPoint = ob->getSelectedPose();
			if(!targetPoint)
			{
				if(ob->getAllowBaseExternalEdit())
					targetPoint = ob->getPosable();
				internalPosable = false;
			}
			else
				internalPosable = true;
			if(targetPoint)
			{
				COPY_POINT(lastSelectedPoint,targetPoint->getPosition());
				lastSelectedPointValid = true;
			}
			else
				lastSelectedPointValid = false;
		}
		else
		{
			ob->useInternalHandle();
			targetPoint = wrapper;
			dim = 0;
			wrapper->attachTo(support);
			for (currentName = numNames; currentName > 1; currentName--)
			{
				wrapper->setIndex((int)names[currentName], dim);
				dim += 2;
			}
		}
		//make the interface react appropriately
		doubleClickInteractor->reset();
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
	if(targetPoint)
		targetPoint->setContinuousChange(GL_TRUE);

	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void PointHandle::dragObject(Object *ob, Window3D *currentWindow)
{
	if (!currentWindow || !ob)
		return;

	//set us up for dragging the point around
	currentWin = currentWindow;
	currentWin->inputMediator.PushInteractor(this);
	Scene3D::setContinuousChange(GL_TRUE);
	dragging = GL_TRUE;

	targetPoint = ob->getSelectedPose();
	if(!targetPoint)
	{
		if(ob->getAllowBaseExternalEdit())
			targetPoint = ob->getPosable();
		internalPosable = false;
	}
	else
		internalPosable = true;
	if(targetPoint)
	{
		COPY_POINT(lastSelectedPoint,targetPoint->getPosition());
		lastSelectedPointValid = true;
	}
	else
		lastSelectedPointValid = false;

	//make the interface react appropriately
	globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	targetPoint->setContinuousChange(GL_TRUE);

	return;
}

/*---------------------------------------------------------------------------*/
void PointHandle::beginLayout(Object *obForLayout)
{
	PosableWrapperSupport *support;

	if (layoutMode == GL_TRUE)
		endLayout();

	layoutMode = GL_TRUE;
	currentWin = NULL;
	globalInputMediator->PushInteractor(this);
	Scene3D::setContinuousChange(GL_TRUE);
	dragging = GL_TRUE;

	DYNAMIC_CAST(support, PosableWrapperSupport *, obForLayout);

	wrapper->attachTo(support);
	
	obForLayout->useInternalHandle();
	targetPoint = wrapper;
	targetPoint->setContinuousChange(GL_TRUE);
	Object::setSelectedObject(obForLayout);
}

/*---------------------------------------------------------------------------*/
void PointHandle::endLayout()
{
	if (!layoutMode)
		return;

	layoutMode = GL_FALSE;
	dragging = GL_FALSE;
	globalInputMediator->RemoveInteractor(this);
	Scene3D::setContinuousChange(GL_FALSE);
	targetPoint->setContinuousChange(GL_FALSE);
	//don't set targetPoint to NULL--it is still valid for moving the last point with the handle.
}

/*---------------------------------------------------------------------------*/
void PointHandle::renderPickableObject(Window3D *inWindow)
{
	Object *ob = Object::getSelectedObject();
        gz4d::Point<double> previousPoint(0.0,0.0,0.0);
	bool previousValid;
        gz4d::Point<double> localPos;
	
	if(lastSelectedPointValid)
		previousPoint = lastSelectedPoint;
		
	previousValid = lastSelectedPointValid;

	if (layoutMode)
		return;

	if (ob)
	{
		picking = GL_TRUE;
		glPushName(0);
		lastSelectedPointValid = false;
		ob->renderPickableObject(inWindow);

		if (!lastSelectedPointValid && previousValid)
		{
			lastSelectedPoint = previousPoint;
			lastSelectedPointValid = true;
		}

		if (lastSelectedPointValid)
		{
			glPushMatrix();
			glLoadIdentity();
			glLoadName(1);
	

			if (ob->isUsingInternalHandle())// && ob->getAllowBaseExternalEdit())
			{
				ob->getPosable()->applyTransforms(inWindow->getViewInfo());
				localPos = lastSelectedPoint;
			}
			else
                            localPos = lastSelectedPoint - inWindow->getViewInfo()->getReferencePoint();
				
			if (ob->getAllowBaseExternalEdit() || wrapper->isAttached() || internalPosable)
				drawPickableTools(inWindow, localPos);
			glPopMatrix();
		}
		glPopName();
		picking = GL_FALSE;

		if (!lastSelectedPointValid)
			wrapper->detach();
	}
}

/*---------------------------------------------------------------------------*/
/// Draws point handle at given position.
/// If the local flag is true, we are already in the object's frame of reference
/// and pointPos is a local offset. Otherwise, the position is global, and needs
/// to be adjusted with the window's view information.
/// \param inWindow The window in which we need to draw.
/// \param pointPos Where we need to be drawn. the \c local flag indicates if
/// 	this position is local or global.
/// \param selectLevel If \> 1, also draws support and pickable tools. Also determines color of the point.
/// \param local Flag indicating a local coordinate
/// \param size The size in pixels to use to draw the point.
void PointHandle::drawIn(Window3D *inWindow, gz4d::Point<double> const &pointPos, int selectLevel, bool local, float size)
{
    Object *ob;
    
    gz4d::Point<double> localPos;
    
    if(local)
        localPos = pointPos;
    else
        localPos = pointPos - inWindow->getViewInfo()->getReferencePoint();

    //draw point in color of selection level
    glPushAttrib(GL_POINT_BIT);
    glPointSize(size);
    glBegin(GL_POINTS);
    /// Select level colors: 0 = yellow, 1 = red, 2 = blue, 3 = grey (repeats)
    if (dragging && selectLevel == 1)
            selectLevel = 0;
    glColor3f(1.0f - ((selectLevel & 2) == 2) * 0.6f,
                        1.0f - (selectLevel >= 1) * 0.6f,
                        0.4f + (selectLevel == 2) * 0.4f);
    glNormal3f(0.0, 0.0, 1.0);
    gz4d::glo::Vertex(localPos);
    glEnd();
    //draw outline of point
    glPointSize(size + 2.0f);
    glBegin(GL_POINTS);
    glColor3f(0.7f, 0.7f, 0.7f);
    gz4d::glo::Vertex(localPos);
    glEnd();
    glPointSize(1.0);

    if (selectLevel <= 1 && !picking)
    {
        COPY_POINT(lastSelectedPoint,pointPos);
        lastSelectedPointValid = true;
                
        if (!targetPoint)
        {
            ob = Object::getSelectedObject();
            if (ob)
            {
                targetPoint = ob->getSelectedPose();
                if(!targetPoint)
                {
                    if(ob->getAllowBaseExternalEdit())
                        targetPoint = ob->getPosable();
                    internalPosable = false;
                }
                else
                    internalPosable = true;
            }
            else
                return;
        }
        drawSupportTools(inWindow, localPos);
        drawPickableTools(inWindow, localPos);
    }
    glPopAttrib();
}

/*---------------------------------------------------------------------------*/
void PointHandle::drawSupportTools(Window3D *inWindow, gz4d::Point<double> const &pointPos)
{
}

/*---------------------------------------------------------------------------*/
void PointHandle::drawPickableTools(Window3D *inWindow, gz4d::Point<double> const &pointPos)
{
}

/*---------------------------------------------------------------------------*/
void PointHandle::toolAction(int pickType, InputInfo *inputInfo, GLuint *names,
							 int hits)
{
}

/*---------------------------------------------------------------------------*/
void PointHandle::cancelAction()
{
	if (layoutMode)
		endLayout();
	else
	{
		Scene3D::setContinuousChange(FALSE);
		if (targetPoint)
			targetPoint->setContinuousChange(FALSE);
		dragging = GL_FALSE;
	}
	globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
/**	Reset to initial conditions
 */
void PointHandle::reset()
{
	targetPoint = NULL;
	lastSelectedPointValid = false;
	internalPosable = false;
	picking = GL_FALSE;
	dragging = GL_FALSE;
	wrapper->detach();
	shiftPressed = cntrlPressed = GL_FALSE;
	currentWin = NULL;
	drawMode = drawModeDefault;
}
