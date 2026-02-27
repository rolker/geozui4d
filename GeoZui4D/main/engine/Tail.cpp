/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// Tail.cpp: implementation of the Tail class.
//
//////////////////////////////////////////////////////////////////////

#include "engine/Tail.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include <iostream>

using namespace std;

extern int dominantEye;

#define PI 3.1415926536
#define ZERO_TOLERANCE 0.00001

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Tail::Tail(int len,float dia)
{
	int i,j;

	float cval;

	truncateLength = Maxtail = len;

	nFacets = 20;
	maxDiameter = dia;

	Colors = NULL;

	// construct the data structures here
	norms_array = new vNorms*[Maxtail];
	discs = new Point3d*[Maxtail];
	Colors = new Point3f[Maxtail];

	//allocate memory for normals
	for (i = 0; i < Maxtail; i++)
	{
		norms_array[i] = new vNorms[nFacets];
	}
	
	// allocate memory for disks
	for (i = 0; i < Maxtail; i++)
	{
		discs[i] = new Point3d[nFacets];
		cval = float(i)/Maxtail;
	
		Colors[i][0] = 1.0f - cval;
		Colors[i][1] = 1.0f - cval;
		Colors[i][2] = 0.8f;	

		if ((i/3)%2 == 0)
		{
			for (j=0;j<3;++j)
				Colors[i][j] *= 0.7f;
		}

	}

	CalculateCircles();  // we make a circle for future calculation

	tailpos = 0;  // tail pos is a circular Q (built on an array)
	taillength = 0;
}

/*---------------------------------------------------------------------------*/
Tail::~Tail()
{
	if (discs)
		delete [] discs;
	if (norms_array)
		delete [] norms_array;
	if (Colors)
		delete [] Colors;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Tail::newPosition(gz4d::Point<double> const &p)
{
	GLboolean calcResult;

	// this is the index into
	// the tail arrays - it wraps around
	// we create ring and normals at current pos
	calcResult = CalcNormals(tailpos, p);

	if (calcResult)
	{
		++taillength;
		if(taillength > Maxtail)
			taillength = Maxtail;

		++tailpos;
		tailpos = tailpos % Maxtail; 
	}
}

/*---------------------------------------------------------------------------*/
void Tail::ResetTail()
{
	tailpos = 0;
	taillength = 0;
}

/*---------------------------------------------------------------------------*/
/*
void Tail::DrawLowRes(float degradation, Window3D *win)
{
	int i,j,gap;

 	gap =  1 ;//+ int(2.0*degradation);  // no degradation
 
	glShadeModel(GL_FLAT);
	glPushMatrix();
	for(i = 0; i < taillength - 2; ++i)
	{
		glColor3fv(Colors[i]);
		j = (tailpos - i - 1);
		if (j < 0)
			j = Maxtail + j;
		DrawShadedRing(j, gap);
	}
	glPopMatrix(); 
}
*/

/*---------------------------------------------------------------------------*/
void Tail::DrawIn(Window3D *win)
{
	int i,j, endlen, prev;
	float scale;
	PosableStateBase *pose = win->getViewInfo()->getPosable();
	const double *p1, *p2;

	pos = pose->getPosition();

	glShadeModel(GL_FLAT);
	glPushMatrix();
	if (truncateLength < taillength)
		endlen = truncateLength;
	else
		endlen = taillength;

	scale = pose->getScale();
	glLineWidth(2.0f);
	for(i = 0; i < endlen - 2; ++i)
	{
		glColor3fv(Colors[i]);
		j = (tailpos - i - 1);
		if (j < 0)
			j = Maxtail + j;
		if (scale > 0.001)
			DrawShadedRing(j, 1);
		else
		{
			prev = j - 1;
			if (prev < 0)
				prev = Maxtail + prev;
			glBegin(GL_LINES);
			p1 = discs[prev][0];
			p2 = discs[j][0];
			glVertex3f(p1[X] - pos[X], p1[Y] - pos[Y], p1[Z] - pos[Z]);
			glVertex3f(p2[X] - pos[X], p2[Y] - pos[Y], p2[Z] - pos[Z]);
			glEnd();
		}
	}
	glPopMatrix();
	glLineWidth(1.0f);
	glShadeModel(GL_SMOOTH);
}

/*---------------------------------------------------------------------------*/
void Tail:: DrawShadedRing(int seg, int gap)
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;
	int prev;

	vNorms *norm1, *norm2;
	Point3d *disc1,*disc2;
	const double *p1, *p2;
	prev = seg-1;
	if (prev < 0) prev = Maxtail + prev;

	disc1 = discs[prev];
	disc2 = discs[seg];// gap is 1
	
	norm1 = norms_array[prev];
	norm2 = norms_array[seg]; // gap is 1

	glPushMatrix(); // set relative offset
	//do pairs of points
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < nFacets; i+=gap)
	{
		p1 = disc1[i];
		p2 = disc2[i];
		//glNormal3s(norm1[i].nx, norm1[i].ny, norm1[i].nz);
		glNormal3sv(norm1[i]);
		glVertex3f(p1[X] - pos[X], p1[Y] - pos[Y], p1[Z] - pos[Z]);
		//glNormal3s(norm2[i].nx, norm2[i].ny, norm2[i].nz);
		glNormal3sv(norm2[i]);
		glVertex3f(p2[X] - pos[X], p2[Y] - pos[Y], p2[Z] - pos[Z]);
	} 
	// finish the ring
	p1 = disc1[0];
	p2 = disc2[0];
	//glNormal3s(norm1[0].nx, norm1[0].ny, norm1[0].nz);
	glNormal3sv(norm1[0]);
	glVertex3f(p1[X] - pos[X], p1[Y] - pos[Y], p1[Z] - pos[Z]);
	//glNormal3s(norm2[0].nx, norm2[0].ny, norm2[0].nz);
	glNormal3sv(norm2[0]);
	glVertex3f(p2[X] - pos[X], p2[Y] - pos[Y], p2[Z] - pos[Z]);
	glEnd();

	/* Points for debugging
	glPointSize(10);
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_POINTS);
		glVertex3fv(disc1[0]);
	glEnd();
	*/

	glPopMatrix();
}

/*---------------------------------------------------------------------------*/
GLboolean Tail::CalcNormals(int i, gz4d::Point<double> const &path)
{
	int j, dim;
	Point3f tempNorm;

	if (path[X] == oldPath[X] &&
		path[Y] == oldPath[Y] &&
		path[Z] == oldPath[Z])
	{
		if (taillength > 0)
			taillength--;
		return GL_FALSE;
	}

	if(taillength == 0) // for the very first one we simply save
	{
		oldPath = path;
		return GL_TRUE;
	}
	if(taillength == 1)
	{
		// we compute the normals to the disks one at a time
		// Need to prime the pump so as to compute the first disc
		// This is simply a permuted version of the first forward
		// vector
		for (dim = X; dim <= Z; dim++)
		{
			Normal[dim] = path[(dim + 2) % 3] - oldPath[(dim + 2) % 3];
			Forward[dim] = path[dim] - oldPath[dim];
		}
	}			
	else 
	{
		for (dim = X; dim <= Z; dim++)
			Forward[dim] = path[dim] - oldoldPath[dim];
	}

	CalcNormal(Forward, Normal, Side);  // S is the resulting side vector
	CalcNormal(Side, Forward, Normal);  // This is the resulting orthogonal vector
	
	//now rotate the disk into the S and N vectors  

	for (j = 0; j < nFacets; j++)
	{	
		// rotate it into place
		for (dim = X; dim <= Z; dim++)
		{
			tempNorm[dim] = (Normal[dim] * circX[j] + Side[dim] * circY[j]);
			discs[i][j][dim] = path[dim] + tempNorm[dim] * maxDiameter;
		}

/*		norms_array[i][j].nx = short(tempNorm[X] * 32767);
		norms_array[i][j].ny = short(tempNorm[Y] * 32767);
		norms_array[i][j].nz = short(tempNorm[Z] * 32767);*/
		norms_array[i][j][X] = short(tempNorm[X] * 32767);
		norms_array[i][j][Y] = short(tempNorm[Y] * 32767);
		norms_array[i][j][Z] = short(tempNorm[Z] * 32767);
	}
	oldoldPath -  oldPath;
	oldPath - path;

	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
int Tail::CalcNormal(gz4d::Point<double> const &v1, gz4d::Point<double> const &v2, gz4d::Point<double> &result)
{
	// do the cross product defined by the vectors
	// v1 - v2

	float length;

	// calculate the cross product

	result[X] = v1[Y] * v2[Z] - v1[Z] * v2[Y];
	result[Y] = v1[Z] * v2[X] - v1[X] * v2[Z];
	result[Z] = v1[X] * v2[Y] - v1[Y] * v2[X];

	// and normalize to a unit vector
	length = (float)sqrt(result[X] * result[X] 
		+ result[Y] * result[Y] + result[Z] * result[Z]);

	if (length < ZERO_TOLERANCE)
		return GL_FALSE;

	result[X] /= length;
	result[Y] /= length;
	result[Z] /= length;

	return GL_TRUE;
}


/*---------------------------------------------------------------------------*/
/** This procedure calculates the circle points based on nFacets, and stores
 *	the x and y values in new arrays, passed back through the x and y
 *	argument pointers.
 */
void Tail::CalculateCircles()
{
	float angleIncrement = (float) (2.0 * PI / (double)nFacets);
	int i;

	//Calculate the first disk;
	circX = new float[nFacets+1];
	circY = new float[nFacets+1];

	for(i = 0; i < nFacets; ++i)
	{
		circX[i] = float(sin(i * angleIncrement));
		circY[i] = float(cos(i * angleIncrement));
	}
	circX[nFacets] = circX[0];
	circY[nFacets] = circY[0];
}

/*---------------------------------------------------------------------------*/
void Tail::truncateTailAt(int index)
{
	if (index >= 0)
		truncateLength = index;
	else
		truncateLength = taillength;
}

/*---------------------------------------------------------------------------*/
/** returns speed estimate, sets orientation of passed pose
 */
float Tail::getAverageCourse(int howFarBack, PosableStateBase *pose)
{
	Point3f vect;
	int pastIndex, i;
	const double *current, *past;
	float speed, val, flatval;

	if (howFarBack >= Maxtail)
		howFarBack = Maxtail - 1;

	pastIndex = tailpos - 1 - howFarBack;
	if (taillength < Maxtail)
	{
		if (pastIndex < 0)
			pastIndex = 0;
	}
	else
	{
		while (pastIndex < 0)
			pastIndex += Maxtail;
	}

	if (tailpos > 0)
		current = discs[tailpos - 1][0];
	else if (taillength >= Maxtail)
		current = discs[Maxtail - 1][0];
	else
		current = discs[taillength][0];
	past = discs[pastIndex][0];
	speed = 0;
	for (i = X; i <= Z; i++)

	{
		val = current[i] - past[i];
		vect[i] = val;
		flatval = speed;
		speed += val * val;
	}
	speed = (float) (sqrt(speed) / (double)howFarBack);
	flatval = (float) sqrt(flatval) / howFarBack; //this is the length of the vector in the x-y plane.
	if (pose && speed > 0.0f)
		pose->setOrientation((float)(atan2(-vect[Z] / howFarBack, -flatval) * RADTODEG), (float)(atan2(-vect[X], -vect[Y]) * RADTODEG));
	return speed;
}
