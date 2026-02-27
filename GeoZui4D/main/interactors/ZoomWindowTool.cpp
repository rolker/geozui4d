/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ZoomWindowTool.cpp
// 
// Interactor for instantiating SubWindows.
//
// Created by mdp 8/16/2000
// 
///////////////////////////////////////////////////////////////////////////////

#include "engine/PosableStateBase.h"  //CORBA before X11
#include "interactors/ZoomWindowTool.h"
#include "SubWindow.h"
#include "widgets/Sv_Standards.h"
#include "engine/ViewManager.h"
#include "tclLinkage.h"
#include "interactors/DoubleClickInteractor.h"

#define SCALE_FACTOR 2.5
#define SCALE_ADDEND 0.0
#define SCALE_AT_FAR_END 1

/*---------------------------------------------------------------------------*/
ZoomWindowTool::ZoomWindowTool()
:Tool3D("zoomportCreator")
{   hotKey = 'z';
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int ZoomWindowTool::HandleInput(unsigned int inputType, InputInfo *info)
{
    gz4d::Point<double> worldPos;
    double zoomFactor;
	Window3D *parentWindow;
	ViewManager *currentView;
	SubWindow *subwin;
	int minSub, minParent;
	const GLint *viewport;

	preprocessInput(inputType, info);

	if (inputType & EnumeratedAction)
	{
		if (inputType == CLICK_EVENT && ((InputInfo*)info->val.p)->type & MouseButton1)
		{
			info = (InputInfo*)info->val.p;
			parentWindow = Window3D::cast(info->window);
			if (parentWindow && parentWindow->getSelectedLocation(info, worldPos))
			{
				currentView = parentWindow->getViewInfo();
				viewport = currentView->getViewport();
				if (viewport[2] < viewport[3])
					minParent = viewport[2];
				else
					minParent = viewport[3];
				if (SUBWINDOW_WIDTH < SUBWINDOW_HEIGHT)
					minSub = SUBWINDOW_WIDTH;
				else
					minSub = SUBWINDOW_HEIGHT;
				zoomFactor = currentView->getPixelSize() / currentView->getPixelSizeAt(worldPos);//adjust zoom factor for depth
				zoomFactor *= SCALE_FACTOR * minParent / minSub; //adjust zoom factor for window size
				subwin = new SubWindow(parentWindow, worldPos,
									   currentView->getPosable()->getScale() * zoomFactor);
				parentWindow->AddSubwindow(subwin);
			}
			return REMOVE_FROM_STREAM;
		}
	}
	return PASS_EVENT_THROUGH;
}

