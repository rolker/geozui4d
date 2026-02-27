/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/

#include"MPGrid.h"
#include"utilities/Utilities.h"
#include<gz4d/glo.h>
#include<GL/glut.h>
#include"engine/ObjectPoseState.h"
//#include"MainWindow.h"
//#include"engine/Scene3D.h"

REGISTER_OBJECT(MPGrid, "");

MPGrid::MPGrid(int cid) : MPDisplayType(cid)
{
	//startPosition = 
	//oppositeCorner = 
	
	length = 5;
	width = 5;
	
//	oppositeCorner setX startPosition getPosition()[0]+width
//	oppositeCorner setY startPosition getPosition()[1]+length
	
	adjustLines = 0;
	maxSeparation = 5.0;
	restrictOddEven = 0;
	restrictToEven = 1;
	numLines = 2;
	
}

void MPGrid::DrawLowRes(float degradation, Window3D *win)
{
}

