// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0



#ifndef QUATERNION_H
#define QUATERNION_H

#define PI 3.141592653589793

#include "SDV_types.h"
/**
 *  Class for rotating and interpolating using quaternions
 *  \par Use for rotations:
 *  As a rotation tool:
 *  Create a quaternion, set its angle/axis with set(Degree)AngleVector() 
 *  Rotate a vector by calling the quaternion's Rotate() method with either
 *  a Point3f or a Quaternion behaving like a vector. Such a quaternion is
 *  created by passing x,y,z into the constructer (in effect, leaving the
 *  angle 0)
 *
 *  Adding rotations: multiplying two rotation quaternions will result in
 *  a quaternion representing the result of the two rotations.
 *
 *  \par Use for Interpolation: 
 *  SLERP method of interpolating rotations still needs to be implemented
 *  \author Colin Ware (cw)
 *  \date 2004
*/

class Quaternion  
{
public:
	Quaternion();
	Quaternion(double X, double Y, double Z);
	Quaternion(double S, double X, double Y, double Z);
	
	void set(double S, double X, double Y, double Z);
	Quaternion operator= (Quaternion quat);
	void setAngleVector(double A, double X, double Y, double Z);
	void setDegreeAngleVector(double A, double X, double Y, double Z);
	
	Quaternion Rotate(Quaternion rotquat);
	void Rotate(Point3f in);
	
	Quaternion Conjugate();
	Quaternion operator* (Quaternion quat);
	Quaternion operator+ (Quaternion quat);
	
	double getDegreeAngle();
	double getVectorX();
	double getVectorY();
	double getVectorZ();
	double getAngle();
	
	void print();
	
	double s, x, y, z;
};

#endif
