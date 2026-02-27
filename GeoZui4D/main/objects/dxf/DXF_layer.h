/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef DXF_LAYER_H
#define DXF_LAYER_H

#include"engine/SimpleObjectCollection.h"
#include"DXF_thing.h"

///\ingroup DXFGroup
class DXF_layer: public DXF_thing, public SimpleObjectCollection
{
public:
	DXF_layer();
	DERIVE_OBJECT(DXF_LAYER);
	
	void setRGB(float red, float green, float blue);
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	int readFrom(FILE * file = NULL);
private:
	float r,g,b;
};


#endif
