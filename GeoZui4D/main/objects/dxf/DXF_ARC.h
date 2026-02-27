/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef DXF_ARC_H
#define DXF_ARC_H

#include "DXF_entity.h"

///\ingroup DXFGroup
class DXF_ARC:public DXF_entity
{
public:
	DXF_ARC();

	int readFrom(FILE *file);
	void Draw(float degradation, Window3D *win);

private:
	Point3f center;
	float radius;
	float startAngle;
	float endAngle;
};

#endif
