// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// WindowAnchor.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class WindowAnchor; //this line connects the above comments with this class.

#ifndef WINDOWANCHOR_H
#define WINDOWANCHOR_H

#include "engine/Interactor.h"
#include "interactors/Pickable.h"
#include <gz4d/Point.h>

class Window3D;
class ViewManager;
class PosableStateBase;
/**
 *	This class implements a SubWindow Interactor for modifying a view from
 *	another window.  This class operates on ViewManagers to effect the
 *	viewpoint changes in the attached SubWindow.
 *	This class implements the following responsibilities:
 *	\li	Provide mechanisms for customizing the proxy reps of subwindows
 *	\li	Provide mechanisms for modifying the subwindow view by moving the
 *		proxy representation
 *
 *	\author Matt Plumlee (mdp)
 *  \date 8/4/2000
 */
class WindowAnchor : public Interactor, public Pickable
{
	public:
		//Initialization
		WindowAnchor(ViewManager *view, Window3D *attachedWindow = NULL);
		~WindowAnchor(void);
		void UpdatePose(void);

		//Picking
                virtual int pickAction(Pickable::PickType pickType, InputInfo *inputInfo, GLuint *names, int hits);
	    virtual void renderPickableObject(Window3D *inWindow);
		static void nextPick();
		void enablePick();

		//Interaction functions
		int HandleInput(unsigned int inputType, InputInfo *info);
		int DrawIn(Window3D *inWindow);
		void setVisible(GLboolean vis = GL_TRUE);
		void setMinimized(GLboolean vis = GL_TRUE);

		//Visual manifestations
		int showProxy(int show = -2);
		GLboolean pickProxy(int pick = -2);
		GLboolean showTethers(int show = -2);
		GLboolean showFlagpole(int show = -2);
		GLboolean showEyepoint(int show = -2);
		GLboolean showCenter(int show = -2);
		float proxyLineWidth(float width = -1.0f);
		float proxyStrength(float strength = -1.0f);
		float tetherStrength(float strength = -1.0f);
		float flagpoleStrength(float strength = -1.0f);
		float centerStrength(float strength = -1.0f);

	protected:
		Window3D	*currentWindow;
		ViewManager *currentView;
		PosableStateBase *currentPose;
		ViewManager *draggingView;
		GLboolean	visible, gotClickEvent, minimized;
		int			_showProxy;
		GLboolean	_pickProxy;
		GLboolean	_showTethers;
		GLboolean	_showFlagpole;
		GLboolean	_showEyepoint;
		GLboolean	_showCenter;
		float		_proxyStrength;
		float		_tetherStrength;
		float		_flagpoleStrength;
		float		_centerStrength;
		float		_proxyLineWidth;

		static int pickMatch;
		int pickValue;

		enum {FRONT_CORNERS, BACK_CORNERS};
		GLfloat corners[2][9][3]; //front/back, which corner/side/center, which dimension
		double scale;
                gz4d::Point<double> refPoint, eyePos; //reference point and eyepoint
                gz4d::Point<float> refPointInView, eyePosInView;

		void DrawCross();
		void DrawEyepointLines();
		void DrawBox(int numClippingPlanesEnabled = 0);
		void DrawFlagPole(Window3D *inWindow, GLboolean picking = GL_FALSE);

};
#endif
