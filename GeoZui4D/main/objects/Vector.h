/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Vector.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class displays vector fields
 *
 *	Created by rja Jan 4th, 2002
 */
/*---------------------------------------------------------------------------*/
class Vector;

#ifndef VECTOR_H
#define VECTOR_H

#include "engine/Object.h"
#include "interactors/Pickable.h"

class PosableStateLock;

class Vector : public Object
{
  public:
    Vector();
	DERIVE_OBJECT(Vector);

	//File-related functions
    int readFrom(FILE *file); //read from a file
    virtual int processField(int option, char **Field);
    //int writeTo(FILE *file);  //define this if you want to be able to save to a file

	//Display-related functions
    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);
    
	virtual void Idle(double time);

  private:
  	struct PointVect
  	{
        Point3d position;
        Point3d v;
	};
    
    struct Arrow
    {
	    Point3f start;  // the
	    Point3f end; // the end of the arrow
	    Point3f * Base; // to store the cylinder vertices
	    vNorms * Normals; 
    };
    
    struct Vects
    {
        int count;
        double time;
        PointVect * vect;
        Arrow * arrow;
    };
    
    int timecount;
    Vects * vects;
    
    int timestep;
    
    double scale;
    
    int dynamic;
    
	void makeArrow(Point3d start, Point3d dir, Arrow & arrow);
        
    
	enum VectorParams {UTM = FirstObjectKeyword, LATLONG, STATIC, DYNAMIC, SCALE};
};

#endif
