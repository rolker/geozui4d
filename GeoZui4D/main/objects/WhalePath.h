/* Copyright (c) 2005 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// WhalePath.h
/** 
 *	This Object is created especially for aiding the visualization and research
 *	of whale ehtology.  Certain components of this object may be useful to
 *	implement in others to make them more powerful, but this one exists to
 *	make further development of the whale research visualizations easier to
 *	integrate with GeoZui4D.
 *
 *	Originally created by Colin Ware outside GeoZui (4/2005)
 *	Brought into GeoZui by Matthew Plumlee 5/19/2005
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef WHALEPATH_H
#define WHALEPATH_H
class WhalePath;

#include "engine/Object.h"

class WhalePath: public Object
{
public:
	
	WhalePath();
	~WhalePath();
	DERIVE_OBJECT(NavigationPath);

	void DrawLowRes(float degradation, Window3D * win);

	Object * LoadXML(bpt::ptree const &xml, char * path, gz4d::netcdf::File ncfile);

	void drawProfile();
	void advance(int step);

	//Scripting support
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

private:
	int smooth(int n, float *&in);
	void initialize();
	void average();
	void differentiate(float *in, float *&out);

	virtual void initializingLabel();

	typedef float Point3f[3];

	float *pathX, *pathY, *pathZ;
	float *azim;
	float *elev;
	float *roll;

	float *deltaElev;
	float *ddeltaElev;

	float *deltaRoll;
	float *ddeltaRoll;

	//note, aROLL, aELEV, and aAZIM are in this order to match file order
	enum {aPATHX, aPATHY, aPATHZ, aROLL, aELEV, aAZIM, aLastOrient = aAZIM, aDELTAELEV, aDDELTAELEV, aDELTAROLL, aDDELTAROLL, numArrays};
	float **arrayOfArrays[numArrays];

	Point3f *Box1, *Box2, *Box3, *Box4;

	Point3f *Sideways;

	Point3f *dNorms;
	Point3f *negdNorms;
	Point3f *dorsalAcc;
	Point3f *rotAcc;


	Point3f *LeftFlipper1;
	Point3f *LeftFlipper2;
	Point3f *RightFlipper1;
	Point3f *RightFlipper2;

	Point3f *dorsalColors;

	enum {paBox1, paBox2, paBox3, paBox4, paSideways, padNorms, panegdNorms, padorsalAcc, parotAcc, paLeftFlipper1, paLeftFlipper2, paRightFlipper1, paRightFlipper2, padorsalColors, numPointArrays};
	Point3f **arrayOfPointArrays[numPointArrays];

	int trackCenter;

	float meanprofile[100];
	float savewave;

	PosableStateBase *cursor;

	int pLen;
};

#endif
