// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// SubWindow.h
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
class SubWindow; //this line connects the above comments with this class.

#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include "engine/Window3D.h"

class PosableStateLock;
/**
 *	This class implements a subwindow (child) viewport Window3D.
 *	This class implements the following responsibilities:
 *	\li	Instantiate a child Window3D of a parent Window3D.
 *	\li	Provide information about this windows location relative to its parent.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 7/27/2000
 */
class SubWindow : public Window3D
{
public:
    SubWindow(Window3D *parentWin, GLboolean overview = GL_FALSE, std::string name = "");
    SubWindow(Window3D *parentWin, gz4d::Point<double> const &center, GLdouble scale = 0.0, std::string name = "");
	~SubWindow();
	virtual void Init(int argc = 0, char *argv[] = NULL);

	/*--Input Handler---*/
	int HandleInput(unsigned int inputType, InputInfo *info);
	virtual void startPickingRound(int pickType);
	virtual void PostOptionMenu(int x, int y);

	/*---Location Accessors---*/
	virtual int getX(GLboolean relativeToMain = GL_FALSE);
	virtual int getY(GLboolean relativeToMain = GL_FALSE);
	virtual int getOuterWidth();
	virtual int getOuterHeight();

	/*---Proxy representations---*/
	virtual void DrawIn(Window3D *inWindow);
	virtual void PerformDrawIns();
	virtual void enableProxy(GLboolean show = GL_TRUE);

	/*---Visibility---*/
	GLboolean IsVisible();
	int SetVisible(GLboolean vis);

	/*---View Locking---*/
	int SettingParent(GLboolean state);
	int LockScale(GLboolean state);
	int LockPosition(GLboolean state);
	int LockRotate(GLboolean state, int direction);
	enum ViewAction {SetView, GetView};
	int ViewAttribute(const char *attrib, ViewAction action);

protected:
	static int subwindowsBehavior;
	GLboolean visible, overview, proxyEnabled;
	GLboolean setParent, lockRotateX, lockRotateY, lockScale, lockPosition;

	int xLocation, yLocation;
	int xSize, ySize;

	PosableStateLock *lightingLock;

	virtual int SubCommand(int argc, char *argv[]);
	virtual int PositionUpdated(int argc = 0, char *argv[] = NULL);
	virtual void Idle();
	virtual void Reshape(int x = 0, int y = 0, int w = 0, int h = 0);

private:
	static int totalSubwindowCount;

	//Initialization information
	static int subwindowToglInitialized;
        void construct(std::string name = "");
};

#endif

