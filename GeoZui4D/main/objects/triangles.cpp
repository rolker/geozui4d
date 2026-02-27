/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// triangles.cpp: implementation of the Triangles class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>
#include "triangles.h"
#include <filesystem>

using namespace std;

extern int dominantEye;

REGISTER_OBJECT(Triangles, "strip");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Triangles::Triangles()
{
	drawrow = 0;
			
}

Triangles::~Triangles()
{
	
}

void Triangles::DrawLowRes(float degradation, Window3D *)
{
	int i,gap;

	gap = (int)(1 + degradation*4);

	glShadeModel(GL_FLAT);
	for(i = 0; i < nStrips; i++)
	{
		DrawShadedStrip(i,gap);
	}
}


int Triangles::DrawHighRes(int eye, Window3D *win)
{	
	if (!doneFlag)
	{
		glShadeModel(GL_FLAT);
		if (drawrow < nStrips)
		{
			DrawShadedStrip(drawrow,1);
			if (eye == dominantEye || eye == MONO_EYE) drawrow++;
		}
		else
			doneFlag = 1;
	}
	else
		doneFlag = 1;

	return doneFlag;
}

void Triangles:: DrawShadedStrip(int row, int gap)
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i, j, k;
	vNorms *Nstrip;
	Point3f *strip;

	Nstrip = norms_array[row];

	strip = strips[row];

	glColor3f(1.0,1.0,0.0);

	//do pairs of points
	glBegin(GL_TRIANGLE_STRIP);
	for (i = 0; i < stripLengths[row]/2; i+=gap)
	{
		j = i*2; k = i*2+1;
		//glNormal3s(Nstrip[j].nx, Nstrip[j].ny, Nstrip[j].nz);
		glNormal3sv(Nstrip[j]);
		glVertex3fv(strip[j]);
		//glNormal3s(Nstrip[k].nx, Nstrip[k].ny, Nstrip[k].nz);
		glNormal3sv(Nstrip[k]);
		glVertex3fv(strip[k]);

	}
	glEnd();
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void Triangles::ResetHighRes()
{
	doneFlag = drawrow = 0;
}

// documented in Object
int Triangles::readFrom(FILE *infile)
{
	char DummyChars[160];
	int i, j, striplen;
	float north, south, east, west,bottom, top; 
	Point3d Position;
	float x, y, z;

	printf("Loading %s ... \n",GetFilename().c_str());
	fflush(stdout);
	
	int dummy = fscanf(infile, "%s", DummyChars);// get first string

	cout << DummyChars << "\n";
	cout.flush();

        dummy = fscanf(infile, "%d", &nStrips);
	cout <<"Strips " <<  nStrips << "\n";
	cout.flush();

	//get the X,Y,Z position of the object
        dummy = fscanf(infile, "%lf %lf %lf", &Position[0], &Position[1], &Position[2]); 

	SetPosition(Position);

// Get two opposite corners of the object box
        dummy = fscanf(infile, "%f %f %f", &west,&south, &bottom); 
        dummy = fscanf(infile, "%f %f %f", &east,&north, &top);

	bbox.w = west;  bbox.e = east;
	bbox.s = south;  bbox.n = north;
	bbox.b = bottom;  bbox.t = top;

	// create the normals data structure
	norms_array = new vNorms*[nStrips];
	strips = new Point3f*[nStrips];
	stripLengths = new int[nStrips];

	// read strips one at a time
	for (i = 0; i < nStrips; i++)
	{
            dummy = fscanf(infile, "%d", &striplen);

		//cerr << "StripLen " << striplen << "\n";
		//.flush();
		stripLengths[i] = striplen;
		strips[i] = new Point3f[striplen];
		
		for(j=0;j<striplen;++j)
		{
                    dummy = fscanf(infile, "%f %f %f", &x, &y, &z);
			strips[i][j][0] = x;
			strips[i][j][1] = y;
			strips[i][j][2] = z;		
		}
	}

	//allocate memory for normals
	for (i = 0; i < nStrips; i++)
	{
		norms_array[i] = new vNorms[stripLengths[i]];
	}

	cout.flush();
	// done reading file
	printf("\nDone loading.\n");
	
	// calculate normals
	printf("Calculating normals...");
	
	CalcAllNormals();
	
	printf("Done.\n");
	
	// done
	return(0);
}


void Triangles::CalcAllNormals()
{
	Point3f v1,v2,v3,v4, normal;
	int i, j, k;
	
	for ( i = 0; i < nStrips; i++) // nStrips
	{
		// do the top triangles first
		for (j = 0; j < (stripLengths[i]/2); j++)
		{	

			for(k=0;k<3;++k)
			{
				v1[k] = strips[i][j*2][k];		
				v2[k] = strips[i][j*2 +1][k];	
				v3[k] = strips[i][(j+1)*2][k];
				v4[k] = strips[i][(j+1)*2+1][k];
			}
			CalcNormal(v1,v2,v3,normal);

/*			norms_array[i][j*2+2].nx = (GLshort)(normal[0]*32767);
			norms_array[i][j*2+2].ny = (GLshort)(normal[1]*32767);
			norms_array[i][j*2+2].nz = (GLshort)(normal[2]*32767);*/
			norms_array[i][j*2+2][X] = (GLshort)(normal[0]*32767);
			norms_array[i][j*2+2][Y] = (GLshort)(normal[1]*32767);
			norms_array[i][j*2+2][Z] = (GLshort)(normal[2]*32767);

			CalcNormal(v2,v4,v3,normal);

/*			norms_array[i][j*2+3].nx = (GLshort)(normal[0]*32767);
			norms_array[i][j*2+3].ny = (GLshort)(normal[1]*32767);
			norms_array[i][j*2+3].nz = (GLshort)(normal[2]*32767);*/
			norms_array[i][j*2+3][X] = (GLshort)(normal[0]*32767);
			norms_array[i][j*2+3][Y] = (GLshort)(normal[1]*32767);
			norms_array[i][j*2+3][Z] = (GLshort)(normal[2]*32767);
		
		}	
	}
}


void Triangles::CalcNormal(Point3f v1, Point3f v2, Point3f v3, Point3f result)
{
	// do the cross product defined by the vectors
	// v3 - v2, v3 - v1

	float dx1, dy1, dz1, dx2, dy2, dz2;
	float length;

	dx1 = v3[0] - v1[0];
	dy1 = v3[1] - v1[1];
	dz1 = v3[2] - v1[2];
	dx2 = v2[0] - v1[0];
	dy2 = v2[1] - v1[1];
	dz2 = v2[2] - v1[2];

	// calculate the cross product

	result[0] = dy1*dz2 - dz1*dy2;
	result[1] = dz1*dx2 - dx1*dz2;
	result[2] = dx1*dy2 - dy1*dx2;

	// and normalize to a unit vector
	length = (float)sqrt(result[0]*result[0] 
		+ result[1]*result[1] + result[2]*result[2]);

	result[0] /= length;
	result[1] /= length;
	result[2] /= length;

}
