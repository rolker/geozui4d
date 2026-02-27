/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

class PlanarPointHandle; //this line connects the above comments with this class.

#ifndef PLANARPOINTHANDLE_H
#define PLANARPOINTHANDLE_H

#include "interactors/PointHandle.h"

class Sv_Slider3;

class PlanarPointHandle : public PointHandle
{
	public:
		enum {DrawNothing = 0x00, DrawGrid = 0x01, DrawVerticalTools = 0x02, DrawRotationTools = 0x04,
			DrawScaleTools = 0x08, DrawSimpleArrows = 0x10, DrawDefault = 0x0F};

		PlanarPointHandle();
		~PlanarPointHandle();

                virtual void drawSupportTools(Window3D *inWindow, gz4d::Point<double> const &pointPos);
                virtual void drawPickableTools(Window3D *inWindow, gz4d::Point<double> const &pointPos);
		virtual void toolAction(int pickType, InputInfo *inputInfo, GLuint *names, int hits);
		virtual void cancelAction();

		int HandleInput(unsigned int inputType, InputInfo *info);

		/*---Scripting funcitons---*/
		virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
		virtual int list(Tcl_Interp *interp);

	protected:
		enum {VerticalHandle, RotationHandle, ScaleHandle, PositionHandle};
		float lastStep, widgetOffsetX, widgetOffsetY;
		GLboolean draggingVertical, scaling, visible;
		int startx, starty;
		float startZ, startScale;
		GLboolean usingWidgets, widgetsPicked;
		Sv_Slider3 *hSlider, *pSlider;
};

#endif
