/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#ifndef ISIS_H
#define ISIS_H

#include "objects/SDV_Quadtree.h"
//#include "SoundingsAvg.h"
#include "engine/Object.h"
#include <sys/types.h>
#include <sys/stat.h>

class Window3D;

#define INBUFSIZE 66000

class Isis : public Object
{
public:
    Isis();
	DERIVE_OBJECT(Isis);

	int Load(Point3d georef_p);
    int readFrom(FILE *file);
    void LoadNext();
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
private:
    SDV_Quadtree * quadtree;

 	FILE * infile;
    int doneloading;
	long fp;
	struct _stat infilestat;
	char inbuf[INBUFSIZE];
	char * bufptr;
	int donechunk;
    Point3f s;
};

#endif
