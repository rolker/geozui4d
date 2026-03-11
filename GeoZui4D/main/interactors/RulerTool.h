// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// RulerTool.h

///////////////////////////////////////////////////////////////////////////////

#ifndef RULERTOOL_H
#define RULERTOOL_H

#include "Tool3D.h"
/** 
 * Interactor for making measurements and placing depth markers.
 *
 *
 * \author Matt Plumlee (mdp)
 * \date 3/3/2000
 */
class RulerTool : public Tool3D
{	public:
		RulerTool();
		int HandleInput(unsigned int inputType, InputInfo *info);
		const char *getToolName() {return "Ruler";}

	protected:

	private:
};

#endif

