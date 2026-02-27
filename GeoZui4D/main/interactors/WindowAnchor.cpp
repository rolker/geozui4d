/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// WindowAnchor.cpp
// 
//
//	This class implements a SubWindow Interactor for modifying a view from
//	another window.  This class operates on ViewManagers to effect the
//	viewpoint changes in the attached SubWindow.
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "interactors/WindowAnchor.h"
#include "SubWindow.h"
#include "widgets/Sv_Standards.h"
#include "interactors/PickInteractor.h"
#include "engine/ViewManager.h"
#include <typeinfo>
#include <gz4d/glo/Vertex.h>

#define FLAG_HEIGHT 40
#define FLAG_SIZE (PICK_WIDTH / 2 + 1.5)
//#define FOUR_CORNER_TETHER
#define TWO_CORNER_TETHER
//#define ONE_CORNER_TETHER

#define EYE_SIZE 0.02
#define CROSS_SIZE 0.13

//#define DRAW_FRUSTUM_AND_EYEPOINT
#define DRAW_SPOTLIGHT
#define DRAW_FLAGPOLE

#ifdef DRAW_FRUSTUM_AND_EYEPOINT
int cornerMapping[4] = {0, 2, 8, 6};
#endif

int WindowAnchor::pickMatch = 0;

/*---------------------------------------------------------------------------*/
/**	Constructor.  Initializes connection to the "parent" window.
 */
WindowAnchor::WindowAnchor(ViewManager *view, Window3D *attachedWindow)
:Interactor("proxy"),
 Pickable(PICK_PRESS1 | PICK_DOUBLE1)
{
	currentWindow = attachedWindow;
	currentView = view;
	currentPose = currentView->getPosable();
	draggingView = NULL;


	pickValue = -1;
	visible = GL_TRUE;
	minimized = GL_FALSE;

	_showProxy = 1;
	_pickProxy = GL_TRUE;
	_showTethers = GL_TRUE;
	_showFlagpole = GL_TRUE;
	_showEyepoint = GL_TRUE;
	_showCenter = GL_TRUE;
	_proxyStrength = 0.3f;
	_tetherStrength = 0.4f;
	_flagpoleStrength = 0.5f;
	_centerStrength = 0.5f;
	_proxyLineWidth = 1.0f;
}

/*---------------------------------------------------------------------------*/
/**	Destructor.  Cleans up any loose ends.
 */
WindowAnchor::~WindowAnchor(void)
{
}

/*---------------------------------------------------------------------------*/
void WindowAnchor::UpdatePose(void)
{
	currentPose = currentView->getPosable();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	HandleInput allows the user to drag a viewpoint across a scene.
 */
int WindowAnchor::HandleInput(unsigned int inputType, InputInfo *info)
{
	Window3D *window;
	gz4d::Point<double> screenPos;
        gz4d::Point<double> wPos;

	if (inputType & EnumeratedAction)
	{
		if (inputType == BreakAction)
		{
			currentPose->setContinuousChange(GL_FALSE);
			currentView->lock((currentView->getLockType() & PosableStateLock::LockAnyMask) | PosableStateLock::LockBoth);
			draggingView = NULL;
			return REMOVE_FROM_STREAM;
		}
		return PASS_EVENT_THROUGH;
	}

    if (!info || !(inputType & DataAsInt))
        return PASS_EVENT_THROUGH;

	/*---Handle Mouse Button Events---*/
    if (!gotClickEvent && (inputType & MouseButton1) && (inputType & MouseButtonDown)) //Is it the initial button1Down event?
	{
		gotClickEvent = GL_TRUE;
		PickInteractor::disable();
		draggingView->plotWorldPoint(refPoint, screenPos);
		currentView->unlock(PosableStateLock::LockBoth);
		window = (Window3D*) info->window;
		window->MoveMouse((int)screenPos[X], (int)screenPos[Y] + 1);
		return SINK_EVENT;
	}
	if (!gotClickEvent || ((inputType & MouseButton1) && (inputType & MouseButtonUp)))  //Is it a button1up event?
	{
		currentPose->setContinuousChange(GL_FALSE);
		currentView->lock((currentView->getLockType() & (PosableStateLock::LockAnyMask)) | PosableStateLock::LockSetFromOwner);
		currentView->lock(((currentView->getLockType() & PosableStateLock::LockAnyMask)) | PosableStateLock::LockBoth);
		PickInteractor::enable();
		draggingView = NULL;
		gotClickEvent = GL_FALSE;
		return REMOVE_FROM_STREAM;
	}

	/*---Handle Mouse Motion Events---*/
	else if (inputType & MouseMotion)
	{
		window = Window3D::cast(info->window);
		if (window)
		{
			wPos = window->getCursorPosOnSurfaceOrPlane(currentView->getReferencePoint()[Z]);
                        if (wPos.IsValid())
			{
				currentPose->setPosition(wPos[X], wPos[Y], wPos[Z]);
				window->requestUpdate();
			}
		}
		return SINK_EVENT;
	}
	else if (inputType & MouseAction)
		return SINK_EVENT;

	return PASS_EVENT_THROUGH;
}

/*---------------------------------------------------------------------------*/
/**	This function allows other classes to control whether or not this anchor
 *	is visible.
 */
void WindowAnchor::setVisible(GLboolean vis)
{
	visible = vis;
	//should really do some reposting here, but there is no gui for this,
	//so it doesn't matter (only used in constructors, etc.).
}

/*---------------------------------------------------------------------------*/
/**	This function allows other classes to control whether or not this anchor
 *	is displayed as minimized (if visible).
 */
void WindowAnchor::setMinimized(GLboolean state)
{
	minimized = state;
	//should really do some reposting here, but there is no gui for this,
	//so it doesn't matter (only used in constructors, etc.).
}

/*-----------------------------Drawing Functions-----------------------------*/
/*---------------------------------------------------------------------------*/
/**	This function renders the entire anchor.
 */
int WindowAnchor::DrawIn(Window3D *inWindow)
{
	gz4d::Point<double> screenPos;
        gz4d::Point<double> posInView;
        gz4d::Point<double> resultingWorldPos;
	SubWindow *subwin = NULL;
	const GLint *viewportInView;
	GLfloat scaleInView, extraFrustumDepth = 0.0f;
	int outerWidth, outerHeight, outerX, outerY;
	GLboolean drawingProxy = GL_FALSE;
#ifdef FOUR_CORNER_TETHER
        gz4d::Point<float> cornersInView[4];
#elif defined(TWO_CORNER_TETHER)
        gz4d::Point<double> screenPoint, screenPos2;
        gz4d::Point<float> cornersInView[2]; 
#elif defined(ONE_CORNER_TETHER)
        gz4d::Point<double> screenPoint;
        gz4d::Point<float> cornerInView;
#else
        gz4d::Point<float> centerInView;
#endif
	
	//coordinates in current window
	int plane, i;
	GLdouble width, height;

	if (!visible)
		return 1;

	//coordinates in window we are drawing
	ViewManager *inView = inWindow->getViewInfo();
	viewportInView = inView->getViewport();
	scaleInView = inView->getPosable()->getScale();
	posInView = inView->getPosable()->getPosition();
	
	glPushMatrix();

	/*----Determine the vital information about the currentView----*/
	scale = currentPose->getScale();
	width = (double)currentView->getViewport()[2];
	height = (double)currentView->getViewport()[3];
	eyePos = currentView->getEyePoint();
	refPoint = currentView->getReferencePoint();
	DIFF_POINT(refPoint, posInView, refPointInView);
        if (eyePos.IsValid())
            eyePosInView = eyePos - posInView;

	if (scale > scaleInView)
	{
		drawingProxy = GL_TRUE;
		if (scale < 5 * scaleInView)
			extraFrustumDepth = 0.1f + (scale/scaleInView) * .0314f;
		else
			extraFrustumDepth = 0.1f + (float)log(log(scale/scaleInView)) * .33;

		if (!minimized)
		{
			screenPos[Z] = 0.0; //for front corners
			//Find the corners of the current window relative to inView (where we are drawing)
			//We're getting the four corners (and midpoints) at its screen, somewhere just
			//past the center of workspace (CoW), and the very back.  We're effectively getting
			//the current window's view frustum.  The indexing scheme is as follows:
			// 0 1 2
			// 3 4 5
			// 6 7 8
			for (plane = FRONT_CORNERS; plane <= BACK_CORNERS; plane++)
			{
				screenPos[X] = 0.0;
				screenPos[Y] = 0.0;
				currentView->getWorldPoint(screenPos, resultingWorldPos);
				DIFF_POINT(resultingWorldPos, posInView, corners[plane][0]);
				screenPos[X] = width;
				currentView->getWorldPoint(screenPos, resultingWorldPos);
				DIFF_POINT(resultingWorldPos, posInView, corners[plane][2]);
				screenPos[Y] = height;
				currentView->getWorldPoint(screenPos, resultingWorldPos);
				DIFF_POINT(resultingWorldPos, posInView, corners[plane][8]);
				screenPos[X] = 0.0;
				currentView->getWorldPoint(screenPos, resultingWorldPos);
				DIFF_POINT(resultingWorldPos, posInView, corners[plane][6]);
				for (i = X; i <= Z; i++)
				{
					corners[plane][1][i] = (corners[plane][0][i] + corners[plane][2][i]) / 2.0;
					corners[plane][3][i] = (corners[plane][0][i] + corners[plane][6][i]) / 2.0;
					corners[plane][4][i] = (corners[plane][0][i] + corners[plane][8][i]) / 2.0;
					corners[plane][5][i] = (corners[plane][2][i] + corners[plane][8][i]) / 2.0;
					corners[plane][7][i] = (corners[plane][6][i] + corners[plane][8][i]) / 2.0;
				}
				//draw the CoW in currentView's own space to figure out what that
				//wacky depth-buffer value is for it, so we can get corners at a
				//depth relative to that (on 2nd time through loop).
				currentView->plotWorldPoint(refPoint, screenPos);
				screenPos[Z] += extraFrustumDepth; // 0.2;
				if (_showProxy == 3) //GeoNav specialty
					screenPos[Z] = 0.98f;
				if (screenPos[Z] > 1.0)
					screenPos[Z] = 1.0f;
			}
		}
	}

	/*----Determine the vital information about the inView----*/
	if (typeid(*currentWindow) == typeid(SubWindow))
	{
		subwin = (SubWindow *)currentWindow;
		screenPos[Z] = 0.01;
		outerWidth = subwin->getOuterWidth();
		outerHeight = subwin->getOuterHeight();
		outerX = subwin->getX();
		outerY = subwin->getY();
#ifdef FOUR_CORNER_TETHER
		screenPos[X] = outerX;
		screenPos[Y] = outerY;
		inView->getWindowPoint(screenPos, cornersInView[0]);
		screenPos[X] = outerX + outerWidth;
		inView->getWindowPoint(screenPos, cornersInView[1]);
		screenPos[Y] = outerY + outerHeight;
		inView->getWindowPoint(screenPos, cornersInView[2]);
		screenPos[X] = outerX;
		inView->getWindowPoint(screenPos, cornersInView[3]);
#elif defined(TWO_CORNER_TETHER)
		float diffx, diffy;

		inView->plotWorldPoint(refPoint, screenPoint); //get screen coord of center
		diffx = screenPoint[X] - outerX - (outerWidth / 2.0);
		diffy = screenPoint[Y] - outerY - (outerHeight / 2.0);
		if (diffy > (diffx * outerHeight / outerWidth))
		{
			screenPos[X] = outerX;
			screenPos[Y] = outerY + outerHeight;
		}
		else
		{
			screenPos[X] = outerX + outerWidth;
			screenPos[Y] = outerY;
		}
		if (diffy < -(diffx * outerHeight / outerWidth))
		{
			screenPos2[X] = outerX;
			screenPos2[Y] = outerY;
		}
		else
		{
			screenPos2[X] = outerX + outerWidth;
			screenPos2[Y] = outerY + outerHeight;
		}

		screenPos2[Z] = screenPos[Z];
		inView->getWindowPoint(screenPos, cornersInView[0]);
		inView->getWindowPoint(screenPos2, cornersInView[1]);
#elif defined(ONE_CORNER_TETHER)
		inView->plotWorldPoint(refPoint, screenPoint); //get screen coord of center
		if (screenPoint[X] <= outerX + outerWidth)
			screenPos[X] = outerX;
		else
			screenPos[X] = outerX + outerWidth;
		if (screenPoint[Y] <= outerY + outerHeight)
			screenPos[Y] = outerY;
		else
			screenPos[Y] = outerY + outerHeight;
		inView->getWindowPoint(screenPos, cornerInView);
#else
		screenPos[X] = outerX + outerWidth/2;
		screenPos[Y] = outerY + outerHeight/2;
		inView->getWindowPoint(screenPos, centerInView);
#endif
	}

#ifdef DRAW_SPOTLIGHT
	if (drawingProxy && _showProxy && !minimized && _showEyepoint)
	{
		glLineWidth(_proxyLineWidth); //was 1.0
		DrawEyepointLines();
	}
#endif

	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (drawingProxy && !minimized)
	{
		if (_showCenter)
			DrawCross();
#ifdef DRAW_FRUSTUM_AND_EYEPOINT
		DrawEyepointLines();
#endif
	}

	//connect cross to the actual window this anchor belongs to.
	if (subwin && subwin->IsVisible() && (_showTethers == GL_TRUE ||
			(_showTethers != GL_FALSE && draggingView)))
	{
		glColor4f(0.5, 0.5, 0.5, _tetherStrength);
		glLineWidth(4.0);
		glBegin(GL_LINE_STRIP);
#ifdef FOUR_CORNER_TETHER
		glVertex3fv(cornersInView[0]);
		glVertex3fv(refPointInView);
		glVertex3fv(cornersInView[1]);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3fv(cornersInView[2]);
		glVertex3fv(refPointInView);
		glVertex3fv(cornersInView[3]);
#elif defined(TWO_CORNER_TETHER)
		gz4d::glo::Vertex(cornersInView[0]);
                gz4d::glo::Vertex(refPointInView);
                gz4d::glo::Vertex(cornersInView[1]);
#elif defined(ONE_CORNER_TETHER)
		glVertex3fv(refPointInView);
		glVertex3fv(cornerInView);
#else
		glVertex3fv(refPointInView);
		glVertex3fv(centerInView);
#endif
		glEnd();

		glColor4f(1.0, 1.0, 1.0, _tetherStrength * 1.25f);
		glLineWidth(2.0);
		glBegin(GL_LINE_STRIP);
#ifdef FOUR_CORNER_TETHER
		glVertex3fv(cornersInView[0]);
		glVertex3fv(refPointInView);
		glVertex3fv(cornersInView[1]);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3fv(cornersInView[2]);
		glVertex3fv(refPointInView);
		glVertex3fv(cornersInView[3]);
#elif defined(TWO_CORNER_TETHER)
                gz4d::glo::Vertex(cornersInView[0]);
                gz4d::glo::Vertex(refPointInView);
                gz4d::glo::Vertex(cornersInView[1]);
#elif defined(ONE_CORNER_TETHER)
		glVertex3fv(refPointInView);
		glVertex3fv(cornerInView);
#else
		glVertex3fv(refPointInView);
		glVertex3fv(centerInView);
#endif
		glEnd();
	}

#ifdef DRAW_FLAGPOLE
	DrawFlagPole(inWindow);
#endif

	glColor4f(1.0, 1.0, 1.0, _proxyStrength);
	if (drawingProxy && _showProxy && !minimized)
	{
		if (_showProxy == 3) //GeoNav specialty
		{
			if (eyePos.IsValid())
			{
				glLineWidth(_proxyLineWidth/2.0 + 1.0);
				glColor4f(0.5f, 0.5f, 0.3f, 0.8f);
				glBegin(GL_LINE_STRIP);
					glVertex3fv(corners[BACK_CORNERS][0]);
                                        gz4d::glo::Vertex(eyePosInView);
					glVertex3fv(corners[BACK_CORNERS][2]);
					glVertex3fv(corners[BACK_CORNERS][0]);
				glEnd();
				glLineWidth(_proxyLineWidth + 2.0);
				glColor4f(0.9f, 0.8f, 0.4f, 0.3f);
				glBegin(GL_LINE_STRIP);
					glVertex3fv(corners[BACK_CORNERS][0]);
                                        gz4d::glo::Vertex(eyePosInView);
					glVertex3fv(corners[BACK_CORNERS][2]);
					glVertex3fv(corners[BACK_CORNERS][0]);
				glEnd();
			}
		}
		else
		{
			DrawBox(currentWindow->EnableClippingPlanes());//&&&not right unless at pitch 0!
			currentWindow->DisableClippingPlanes();
		}
	}

	glDepthMask(GL_TRUE);
	glBlendFunc(GL_ONE, GL_ZERO);

	glLineWidth(1.0);
glPopMatrix();
	return 1;
}

/*---------------------------------------------------------------------------*/
/**	This function renders the central cross of the current view.
 */
void WindowAnchor::DrawCross()
{
	GLfloat additive;
	
        if (!refPoint.IsValid())
		return;

	additive = (float)(CROSS_SIZE/scale);

	//Draw the cross corresponding to the widgets (at the reference point)
	glPushMatrix();
	glTranslatef(refPointInView[X], refPointInView[Y], refPointInView[Z]);
	glRotated(-currentPose->getOrientationAxis()[Y], 0.0, 0.0, 1.0 );
	glColor4f(0.5, 0.5, 0.5, _centerStrength * .8);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, additive);
	glVertex3f(0.0, 0.0, -additive);
	glVertex3f(additive, 0.0, 0.0);
	glVertex3f(-additive, 0.0, 0.0);
	glEnd();
	glColor4f(1.0, 1.0, 1.0, _centerStrength);
	glLineWidth(2.0);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, additive);
	glVertex3f(0.0, 0.0, -additive);
	glVertex3f(additive, 0.0, 0.0);
	glVertex3f(-additive, 0.0, 0.0);
	glEnd();
	glPopMatrix();
}

/*---------------------------------------------------------------------------*/
/**	This function renders the lines from the eyepoint to the front clipping
 *	plane.
 */
void WindowAnchor::DrawEyepointLines()
{
#ifdef DRAW_FRUSTUM_AND_EYEPOINT
	int corner;
#endif
#ifdef DRAW_SPOTLIGHT
	int i, face;
	float eyeSize = EYE_SIZE / currentPose->getScale(), val;
	gz4d::Point<double> orient;
#endif

        if (!eyePos.IsValid())
		return;

#ifdef DRAW_SPOTLIGHT
	glPushMatrix();
	glTranslatef(eyePosInView[X], eyePosInView[Y], eyePosInView[Z]);
	orient = currentPose->getOrientationAxis();
	glRotated( -orient[Y], 0.0, 0.0, 1.0 );
	glRotated( -orient[X], 1.0, 0.0, 0.0 );
	for (i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glColor4f(0.0, 0.0, 0.0, 1.0f);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glColor4f(1.0, 1.0, 0.5, 1.0f);
		}
	
		glBegin(GL_QUADS);
		for (face = -1; face < 2; face += 2)
		{
			val = face * eyeSize;
			//face one.
			glVertex3f(-eyeSize, -eyeSize, val);
			glVertex3f(eyeSize, -eyeSize, val);
			glVertex3f(eyeSize, eyeSize, val);
			glVertex3f(-eyeSize, eyeSize, val);
			//face two.
			glVertex3f(-eyeSize, val, -eyeSize);
			glVertex3f(eyeSize, val, -eyeSize);
			glVertex3f(eyeSize, val, eyeSize);
			glVertex3f(-eyeSize, val, eyeSize);
			//face three.
			glVertex3f(val, -eyeSize, -eyeSize);
			glVertex3f(val, eyeSize, -eyeSize);
			glVertex3f(val, eyeSize, eyeSize);
			glVertex3f(val, -eyeSize, eyeSize);
		}
		glEnd();
	}
	glPopMatrix();
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	//drop-down line
	glBegin(GL_LINES);
	glColor4f(1.0, 1.0, 0.5, 1.0f);
        gz4d::glo::Vertex(eyePosInView);
	glVertex3f(eyePosInView[X], eyePosInView[Y], 0.0);
	glEnd();
#endif

#ifdef DRAW_FRUSTUM_AND_EYEPOINT
	glBegin(GL_LINES);
	for (corner = 0; corner < 4; corner++)
	{
		glVertex3fv(eyePosInView);
		glVertex3fv(corners[FRONT_CORNERS][cornerMapping[corner]]);
	}
	glEnd();
#endif
}

/*---------------------------------------------------------------------------*/
/**	This function renders the partial frustum from the front clipping plane
 *	to just after the cross.
 */
void WindowAnchor::DrawBox(int numClippingPlanesEnabled)
{
#ifdef DRAW_FRUSTUM_AND_EYEPOINT
	int corner;
#endif

        if (!eyePos.IsValid())
		return;

#ifdef DRAW_SPOTLIGHT
	glColor4f(1.0, 1.0, 1.0, _proxyStrength);
	if (currentView->getPerspective())
	{
		glBegin(GL_TRIANGLES);
		if (numClippingPlanesEnabled) //draw outside triangles and horizontal slice
		{
                    gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][0]);
			glVertex3fv(corners[BACK_CORNERS][2]);
                        gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][0]);
			glVertex3fv(corners[BACK_CORNERS][6]);
                        gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][3]);
			glVertex3fv(corners[BACK_CORNERS][5]);
                        gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][2]);
			glVertex3fv(corners[BACK_CORNERS][8]);
                        gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][6]);
			glVertex3fv(corners[BACK_CORNERS][8]);
		}
		else
		{
			//Draw an orienting triangle to the eyepoint.
                    gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][1]);
			glVertex3fv(corners[BACK_CORNERS][4]);
			glColor4f(1.0, 1.0, 1.0, _proxyStrength * .625);
                        gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][3]);
			glVertex3fv(corners[BACK_CORNERS][5]);
                        gz4d::glo::Vertex(eyePosInView); //eyepoint
			glVertex3fv(corners[BACK_CORNERS][7]);
			glVertex3fv(corners[BACK_CORNERS][4]);
		}
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
			if (_showProxy != 2) //draw top, bottom, and sides
			{
				glVertex3fv(corners[BACK_CORNERS][0]);
				glVertex3fv(corners[BACK_CORNERS][2]);
				glVertex3fv(corners[FRONT_CORNERS][2]);
				glVertex3fv(corners[FRONT_CORNERS][0]);
				glVertex3fv(corners[BACK_CORNERS][6]);
				glVertex3fv(corners[BACK_CORNERS][8]);
				glVertex3fv(corners[FRONT_CORNERS][8]);
				glVertex3fv(corners[FRONT_CORNERS][6]);
				glVertex3fv(corners[BACK_CORNERS][0]);
				glVertex3fv(corners[BACK_CORNERS][6]);
				glVertex3fv(corners[FRONT_CORNERS][6]);
				glVertex3fv(corners[FRONT_CORNERS][0]);
				glVertex3fv(corners[BACK_CORNERS][2]);
				glVertex3fv(corners[BACK_CORNERS][8]);
				glVertex3fv(corners[FRONT_CORNERS][8]);
				glVertex3fv(corners[FRONT_CORNERS][2]);
			}
			//draw center plane
			glVertex3fv(corners[BACK_CORNERS][3]);
			glVertex3fv(corners[BACK_CORNERS][5]);
			glVertex3fv(corners[FRONT_CORNERS][5]);
			glVertex3fv(corners[FRONT_CORNERS][3]);
		glEnd();
	}
#endif

#ifdef DRAW_FRUSTUM_AND_EYEPOINT
	glColor4f(1.0, 1.0, 1.0, _proxyStrength);
	glBegin(GL_QUAD_STRIP);
	for (corner = 0; corner < 4; corner++)
	{
		glVertex3fv(corners[FRONT_CORNERS][cornerMapping[corner]]);
		glVertex3fv(corners[BACK_CORNERS][cornerMapping[corner]]);
		glVertex3fv(corners[FRONT_CORNERS][cornerMapping[(corner + 1) % 4]]);
		glVertex3fv(corners[BACK_CORNERS][cornerMapping[(corner + 1) % 4]]);
	}
	glEnd();
#endif
}

/*---------------------------------------------------------------------------*/
/**	This function renders a scale-independent post with a flag on top.
 */
void WindowAnchor::DrawFlagPole(Window3D *inWindow, GLboolean picking)
{
	ViewManager *inView;
	const GLint *viewportInView;
	gz4d::Point<double> screenPointPos;
        gz4d::Point<float> screenWorldPoint[2];

	inView = inWindow->getViewInfo();
	viewportInView = inView->getViewport();

	inView->plotWorldPoint(currentView->getReferencePoint(), screenPointPos); //get screen coord of center

	//Draw a box around the center plus a pole
	glColor4f(1.0, 1.0, 1.0, _flagpoleStrength);
	glLineWidth(2.0);

	if (picking)
	{
		screenPointPos[X] -= FLAG_SIZE;
		screenPointPos[Y] += FLAG_SIZE;
		inView->getWindowPoint(screenPointPos, screenWorldPoint[0]);
		screenPointPos[X] += FLAG_SIZE * 2;
		screenPointPos[Y] -= FLAG_SIZE * 2 + FLAG_HEIGHT;
		inView->getWindowPoint(screenPointPos, screenWorldPoint[1]);

		glBegin(GL_QUADS);
		gz4d::glo::Vertex(screenWorldPoint[0]);
		glVertex3f(screenWorldPoint[0][X], screenWorldPoint[1][Y], screenWorldPoint[1][Z]);
                gz4d::glo::Vertex(screenWorldPoint[1]);
		glVertex3f(screenWorldPoint[1][X], screenWorldPoint[0][Y], screenWorldPoint[0][Z]);
		glEnd();
	}
	else
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0.0, viewportInView[2], 0.0, viewportInView[3]);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		//translate into 2D coordinates
		screenPointPos[Y] = viewportInView[3] - screenPointPos[Y];
		screenPointPos[Z] = 1 - screenPointPos[Z] * 2;
		if (_showFlagpole)
		{
			glBegin(GL_LINES);
			gz4d::glo::Vertex(screenPointPos);
			glVertex3d(screenPointPos[X], screenPointPos[Y] + FLAG_HEIGHT - FLAG_SIZE, screenPointPos[Z]);
			glEnd();

			glBegin(GL_LINE_STRIP);
			glVertex3d(screenPointPos[X] - FLAG_SIZE, screenPointPos[Y] + FLAG_SIZE + FLAG_HEIGHT, screenPointPos[Z]);
			glVertex3d(screenPointPos[X] + FLAG_SIZE, screenPointPos[Y] + FLAG_SIZE + FLAG_HEIGHT, screenPointPos[Z]);
			glVertex3d(screenPointPos[X] - FLAG_SIZE, screenPointPos[Y] - FLAG_SIZE + FLAG_HEIGHT, screenPointPos[Z]);
			glVertex3d(screenPointPos[X] + FLAG_SIZE, screenPointPos[Y] - FLAG_SIZE + FLAG_HEIGHT, screenPointPos[Z]);
			glEnd();
		}

		if (_showCenter)
		{
			glBegin(GL_QUADS);
			glVertex3d(screenPointPos[X] + FLAG_SIZE, screenPointPos[Y] + FLAG_SIZE, screenPointPos[Z]);
			glVertex3d(screenPointPos[X] + FLAG_SIZE, screenPointPos[Y] - FLAG_SIZE, screenPointPos[Z]);
			glVertex3d(screenPointPos[X] - FLAG_SIZE, screenPointPos[Y] - FLAG_SIZE, screenPointPos[Z]);
			glVertex3d(screenPointPos[X] - FLAG_SIZE, screenPointPos[Y] + FLAG_SIZE, screenPointPos[Z]);
			glEnd();
		}

		glFlush();
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
}

/*-----------------------------Picking Functions-----------------------------*/
/*---------------------------------------------------------------------------*/
/**	The pick action pushes our interactor (this) onto the calling window's
 *	InputMediator.
 */
int WindowAnchor::pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits)
{
	Window3D *window;
	SubWindow *subwin;

	if (minimized)
	{
		if (pickType & PICK_DOUBLE1)
		{
			DYNAMIC_CAST(subwin, SubWindow *, currentWindow);
			if (subwin)
			{
				subwin->SetVisible(GL_TRUE);
				return GL_TRUE;
			}
		}
		return GL_FALSE;
	}

	if (pickType & PICK_PRESS1)
	{
		window = (Window3D*) inputInfo->window;
		if (window && !draggingView)
		{
			draggingView = window->getViewInfo();
			window->inputMediator.PushInteractor(this);
			currentPose->setContinuousChange(GL_TRUE);
			gotClickEvent = GL_FALSE;
		}
	}
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void WindowAnchor::renderPickableObject(Window3D *inWindow)
{
	if (pickValue == pickMatch && visible && _pickProxy)
	{
		if (minimized)
			DrawFlagPole(inWindow, GL_TRUE);
		else
		{
			DrawCross();
			if (_showProxy)
				DrawBox();
//			DrawEyepointLines();
		}
	}
	pickValue = pickMatch;
}

/*---------------------------------------------------------------------------*/
/**	This function updates the counter for the next enabling of picking.  This
 *	function should be called before enablePick's are called for a new round of
 *	picking.
 */
void WindowAnchor::nextPick()
{
	pickMatch ^= 1;
}

/*---------------------------------------------------------------------------*/
void WindowAnchor::enablePick()
{
	pickValue = pickMatch;
}

/*---------------------------------------------------------------------------*/
int WindowAnchor::showProxy(int show)
{
	if (show > -2)
		_showProxy = show;
	return _showProxy;
}

/*---------------------------------------------------------------------------*/
GLboolean WindowAnchor::pickProxy(int pick)
{
	if (pick > -2)
		_pickProxy = pick;
	return _pickProxy;
}

/*---------------------------------------------------------------------------*/
GLboolean WindowAnchor::showTethers(int show)
{
	if (show > -2)
		_showTethers = show;
	return _showTethers;
}

/*---------------------------------------------------------------------------*/
GLboolean WindowAnchor::showFlagpole(int show)
{
	if (show > -2)
		_showFlagpole = show;
	return _showFlagpole;
}

/*---------------------------------------------------------------------------*/
GLboolean WindowAnchor::showEyepoint(int show)
{
	if (show > -2)
		_showEyepoint = show;
	return _showEyepoint;
}

/*---------------------------------------------------------------------------*/
GLboolean WindowAnchor::showCenter(int show)
{
	if (show > -2)
		_showCenter = show;
	return _showCenter;
}

/*---------------------------------------------------------------------------*/
float WindowAnchor::proxyLineWidth(float width)
{
	if (width >= 0.0f)
		_proxyLineWidth = width;
	return _proxyLineWidth;
}

/*---------------------------------------------------------------------------*/
float WindowAnchor::proxyStrength(float strength)
{
	if (strength >= 0.0f)
		_proxyStrength = strength;
	return _proxyStrength;
}

/*---------------------------------------------------------------------------*/
float WindowAnchor::tetherStrength(float strength)
{
	if (strength >= 0.0f)
		_tetherStrength = strength;
	return _tetherStrength;
}

/*---------------------------------------------------------------------------*/
float WindowAnchor::flagpoleStrength(float strength)
{
	if (strength >= 0.0f)
		_flagpoleStrength = strength;
	return _flagpoleStrength;
}

/*---------------------------------------------------------------------------*/
float WindowAnchor::centerStrength(float strength)
{
	if (strength >= 0.0f)
		_centerStrength = strength;
	return _centerStrength;
}
