/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// ScaleDrag.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class ScaleDrag; //this line connects the above comments with this class.

#ifndef SCALEDRAG_H
#define SCALEDRAG_H

#include "engine/Interactor.h"
class PosableStateBase;
class ViewManager;
/**
 *	This class implements a zooming interactor that zooms in and out according
 *	to the dragging of a fixed point away from or toward the center of the
 *	screen (in the screen-x-direction).
 *	This class implements the following responsibilities:
 *	\li	Implement ZoomDrag as described above
 *
 *	\author Matt Plumlee (mdp)
 *  \date 4/7/2001
 */
class ScaleDrag : public Interactor
{
   public:
		ScaleDrag(GLboolean standalone = GL_TRUE, GLboolean vertical = GL_FALSE);
		//int RegisterWithTcl(ToglWindow *throughWin);
		~ScaleDrag(void);

		//Interaction functions
		void reset();
		int HandleInput(unsigned int inputType, InputInfo *info);
		virtual void RemovalNotify(InputMediator *fromList);

   protected:
		GLfloat          startingRatio;   // Where did the mouse start?
		PosableStateBase *currentPose;
		ViewManager      *currentView;
		GLboolean        isStandalone;    // Are we standalone (vs. part of another interactor)
		GLboolean        isVertical;      // Is this a vertical scale/exaggeagion or scale (controlled horizontally)?
};
#endif
