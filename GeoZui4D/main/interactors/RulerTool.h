/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
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

