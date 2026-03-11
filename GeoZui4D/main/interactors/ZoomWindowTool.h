// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ZoomWindowTool.h

///////////////////////////////////////////////////////////////////////////////
class ZoomWindowTool;

#ifndef ZOOM_WINDOW_TOOL_H
#define ZOOM_WINDOW_TOOL_H

#include "Tool3D.h"

class SubWindow;
/** 
 *	Interactor for instantiating SubWindows.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 8/16/2000
 */
class ZoomWindowTool : public Tool3D
{	public:
		ZoomWindowTool();
		int HandleInput(unsigned int inputType, InputInfo *info);
		const char *getToolName() {return "Zoom Window";}

	protected:

	private:
};

#endif

