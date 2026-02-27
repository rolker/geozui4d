/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// RulerPost.h
/** 
 * Display a verical line with hash-marks on it.
 *
 * Created by mdp 3/3/2000
 */
///////////////////////////////////////////////////////////////////////////////

#ifndef RULERPOST_H
#define RULERPOST_H

#include <iostream>
#include "engine/Object.h"


class RulerPost : public Object
{	public:
		//File-related
        int readFrom(FILE *file) {std::cout << "RulerPost::Load not implemented yet!\n"; return 0;};

		//Rendering
		void DrawLowRes(float degradation, Window3D *win);
		int DrawHighRes(int eye, Window3D *win);

		RulerPost();
		RulerPost(ObjectCollection * p, GLdouble xPos = 0.0, GLdouble yPos = 0.0, GLdouble zPos = 0.0);
		~RulerPost();
		
		virtual void ShowInfo(GLuint *names = NULL, int numNames = 0);
		
		DERIVE_OBJECT(RulerPost);

	protected:
		GLdouble x,y,z;

		char * position_label;

};

#endif

