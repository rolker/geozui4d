// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// ValueDrag.h
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class ValueDrag; //this line connects the above comments with this class.

#ifndef VALUEDRAG_H
#define VALUEDRAG_H

#include "engine/Interactor.h"
/**
 *	This class implements a mouse-dragging interactor that increments or
 *	decrements a value based on mouse motion along a specified axis.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 8/17/2005
 */
class ValueDrag : public Interactor
{
   public:
		ValueDrag(float *valueToDrag, float axisAngle = 0, float scaleFactor = 0.0); // a zero scale factor means to treat the value as a physical thing, and to use the scale of the window we're interacting in.
		~ValueDrag(void);

		void setAxisAngle(float axisAngle);
		void setNegativePolarity(bool isNegative = true) {negPolarity = isNegative;}
		void setValueAlwaysPositive(bool vPos = true) {valueAlwaysPositive = vPos;}

		void setFlagVariable(bool *newFlag = NULL) {flag = newFlag;}

		//Interaction functions
		int HandleInput(unsigned int inputType, InputInfo *info);

   protected:
		float startx, starty, startValue;   // Where did the mouse & dragValue start?
		float *dragValue;
		float angle, sinAngle, cosAngle;
		float scale;
		bool negPolarity, valueAlwaysPositive, *flag;
};
#endif
