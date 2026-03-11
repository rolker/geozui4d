// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef SWATHSTREAM_H
#define SWATHSTREAM_H

//#include "RawMultibeam.h"

#include "objects/SDV_Quadtree.h"
//#include "SDV_Tiles.h"

#include "SwathBuffer.h"
#include "engine/Object.h"
#include <sys/types.h>
#include <sys/stat.h>


class Window3D;

#define INBUFSIZE 66000

class SwathStream : public Object
{
public:
    SwathStream();
	DERIVE_OBJECT(SwathStream);

	void Initialize(float resolution,const char * type);
	void InitializeWA(double radius);

	int readFrom(FILE * file);
	virtual void Idle(double time);
    void LoadNext();
	void DrawLowRes(float degradation, Window3D *win);
	int DrawHighRes(int eye, Window3D *win);
	void DrawShip();
    void ShowInfo(GLuint *names, int numNames);
private:
    static SDV_Quadtree * quadtree;
//    static SDV_Tiles * quadtree;
	long int soundingcount;
   	long int swathcount;

    static char * soundingtype;
	static int masterpresent;
	int ismaster;

	SwathBuffer * swathbuffer;
	unsigned char degres;
    int degcount;  // used to smooth out deg problems.

	#ifndef WIN32
//    RawMultibeam * rawmb;
	#endif
    
//	paramaters used for weighted averages
    double radius;

	double roll,pitch,heading;
    
    
    FILE * swsfile;
	FILE * asciiIn;
    char ftype[100];
};

#endif
