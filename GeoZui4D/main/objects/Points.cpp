// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
// Points.cpp: implementation of the Points class.
//
//////////////////////////////////////////////////////////////////////

#include "GZincludes.h"
//#include "MainWindow.h"
#include <math.h>
//#include "engine/Scene3D.h"
#include "Points.h"

#define DEGTORADIANS	(3.1415926536 / 180)
#define CONE_BASE	2.0
#define DEFAULT_COLOR 5

extern int dominantEye;
extern MainWindow *mainWindow;

REGISTER_OBJECT(Points, "point");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------------------*/
Points::Points()
{
	pointCount = 0;

	coneNormals = new vNorms[CONE_SEGS];
	cone = new Point3f[CONE_SEGS+2];

	makeCone();

	local_palette[0][0] = 1.0f; 	local_palette[0][1] = 0.2f; 	local_palette[0][2] = 0.2f; //Red
	local_palette[1][0] = 0.2f; 	local_palette[1][1] = 0.8f; 	local_palette[1][2] = 0.2f; //Green
	local_palette[2][0] = 0.3f; 	local_palette[2][1] = 0.3f; 	local_palette[2][2] = 1.0f; //Blue
	local_palette[3][0] = 0.9f; 	local_palette[3][1] = 1.0f; 	local_palette[3][2] = 0.0f; //Yellow
	local_palette[4][0] = 0.7f; 	local_palette[4][1] = 0.1f; 	local_palette[4][2] = 0.7f; //Magenta
	local_palette[5][0] = 0.8f; 	local_palette[5][1] = 0.8f; 	local_palette[5][2] = 1.0f; //Lt. Periwinkle
	//Following are colors added by mdp to support ABCDE (blue is already covered)
	local_palette[6][0] = 0.8f;		local_palette[6][1] = 0.6f;		local_palette[6][2] = 0.2f; //Auburn (brown)
	local_palette[7][0] = 0.2f;		local_palette[7][1] = 0.9f;		local_palette[7][2] = 0.9f; //Cyan
	local_palette[8][0] = 0.1f;		local_palette[8][1] = 0.5f;		local_palette[8][2] = 0.1f; //Dark Green
	local_palette[9][0] = 0.6f;		local_palette[9][1] = 0.6f;		local_palette[9][2] = 0.6f; //Elephant Grey

	comments = NULL;
	sizes = NULL;
	colors = NULL;
}

Points::Points(char * c)
{
	coneNormals = new vNorms[CONE_SEGS];
	cone = new Point3f[CONE_SEGS+2];

	makeCone();

	local_palette[0][0] = 1.0f; 	local_palette[0][1] = 0.2f; 	local_palette[0][2] = 0.2f; //Red
	local_palette[1][0] = 0.2f; 	local_palette[1][1] = 0.8f; 	local_palette[1][2] = 0.2f; //Green
	local_palette[2][0] = 0.3f; 	local_palette[2][1] = 0.3f; 	local_palette[2][2] = 1.0f; //Blue
	local_palette[3][0] = 0.9f; 	local_palette[3][1] = 1.0f; 	local_palette[3][2] = 0.0f; //Yellow
	local_palette[4][0] = 0.7f; 	local_palette[4][1] = 0.1f; 	local_palette[4][2] = 0.7f; //Magenta
	local_palette[5][0] = 0.8f; 	local_palette[5][1] = 0.8f; 	local_palette[5][2] = 1.0f; //Lt. Periwinkle
	//Following are colors added by mdp to support ABCDE (blue is already covered)
	local_palette[6][0] = 0.8f;		local_palette[6][1] = 0.6f;		local_palette[6][2] = 0.2f; //Auburn (brown)
	local_palette[7][0] = 0.2f;		local_palette[7][1] = 0.9f;		local_palette[7][2] = 0.9f; //Cyan
	local_palette[8][0] = 0.1f;		local_palette[8][1] = 0.5f;		local_palette[8][2] = 0.1f; //Dark Green
	local_palette[9][0] = 0.6f;		local_palette[9][1] = 0.6f;		local_palette[9][2] = 0.6f; //Elephant Grey

	comments = NULL;
//	sizes = NULL;
	sizes = new float[1];
	sizes[0] = 0.1f;
	sizeflag = 1;
	colors = NULL;
	
	// single point points object with given label
	positions = new Point3f[1];
	positions[0][0] = 0.0;
	positions[0][1] = 0.0;
	positions[0][2] = 0.0;
	pointCount = 1;
	nPoints = 1;
	comments = new char*[1];
	comments[0] = new char[strlen(c)+1];
	strcpy(comments[0],c);
	textflag = true;
}

/*--------------------------------------------------------------------------*/
Points::~Points()
{
	
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Points::DrawLowRes(float degradation, Window3D *)
{
	int i;

	glShadeModel(GL_FLAT);
		
	for(i = 0; i < nPoints; i++)
	{
		glPushMatrix();
		glTranslatef(positions[i][0], positions[i][1], positions[i][2]);
		if (colors)
			glColor3fv(local_palette[colors[i]]);
		else
			glColor3fv(local_palette[DEFAULT_COLOR]);
		if (sizeflag)
			glScalef(sizes[i], sizes[i], sizes[i]);
		if (renderingForPicking)
		{
			glLoadName(i);
			DrawCone();
		}
		else
			DrawLoresCone();
		glPopMatrix(); 
	}
	doneFlag = pointCount = 0;
	glColor3f(1.0,1.0,1.0);
}

/*--------------------------------------------------------------------------*/
int Points::DrawHighRes(int eye, Window3D *win)
{
	glShadeModel(GL_FLAT);
	if (!doneFlag)
	{
		if (pointCount < nPoints)
		{
			glPushMatrix();
			glTranslatef(positions[pointCount][0], positions[pointCount][1], positions[pointCount][2]);

			if (colors)
				glColor3fv(local_palette[colors[pointCount]]);
			else
				glColor3fv(local_palette[DEFAULT_COLOR]);
			if (sizeflag)
				glScalef(sizes[pointCount], sizes[pointCount], sizes[pointCount]);
			DrawCone();

			glPopMatrix(); 
			if (eye == dominantEye || eye == MONO_EYE) pointCount++;
		}
		else
			doneFlag = 1;
				
	}
	else
		doneFlag = 1;
	
	glColor3f(1.0,1.0,1.0);
	return doneFlag;
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void Points::ResetHighRes()
{
	doneFlag = pointCount = 0;
}

/*---------------------------------------------------------------------------*/
/**	Display info about the selected point in the info window on the GUI.
 */
void Points::ShowInfo(GLuint *names, int numNames)
{
	Scene3D *scene;

	scene = mainWindow->getScene3D();

	if (numNames && comments)
	{
		scene->appendInfo(comments[names[0]], GL_TRUE);
		scene->appendInfo("--- Generic Info:---\n");
	}

	Object::ShowInfo(names, numNames);
}

/*--------------------------------------------------------------------------*/
void Points:: DrawCone()
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;

	//do pairs of points
	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(cone[CONE_SEGS]);
	for (i = 0; i < CONE_SEGS; i++)
	{
		//glNormal3s(coneNormals[i].nx, coneNormals[i].ny, coneNormals[i].nz);
		glNormal3sv(coneNormals[i]);
		glVertex3fv(cone[i]);
	}
	//glNormal3s(coneNormals[0].nx, coneNormals[0].ny, coneNormals[0].nz);
	glNormal3sv(coneNormals[0]);
	glVertex3fv(cone[0]);
	glEnd();

	glBegin(GL_LINES);
	glVertex3fv(cone[CONE_SEGS]);
	glVertex3fv(cone[CONE_SEGS+1]);
	glEnd();
}

/*--------------------------------------------------------------------------*/
void Points:: DrawLoresCone()
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;

	//do pairs of points
	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(cone[CONE_SEGS]);
	for (i = 0; i < CONE_SEGS; i=i+8)
	{
		//glNormal3s(coneNormals[i].nx, coneNormals[i].ny, coneNormals[i].nz);
		glNormal3sv(coneNormals[i]);
		glVertex3fv(cone[i]);
	}
	//glNormal3s(coneNormals[0].nx, coneNormals[0].ny, coneNormals[0].nz);
	glNormal3sv(coneNormals[0]);
	glVertex3fv(cone[0]);
	glEnd();

	glBegin(GL_LINES);
	glVertex3fv(cone[CONE_SEGS]);
	glVertex3fv(cone[CONE_SEGS+1]);
	glEnd();
}

/*--------------------------------------------------------------------------*/
// documented in Object
int Points::readFrom(FILE *infile)
{
	char DummyChars[160];
	char colorcode[8];
	char inputstring[1024];
	char *comment;

	int i,j,c;
	Point3d p;
	float version, ptsize;

	int dummy = fscanf(infile, "%s", DummyChars);// strip the header
	while(strcmp(DummyChars,"BEGIN") != 0)
			dummy = fscanf(infile, "%s", DummyChars);

	dummy = fscanf(infile, "%s %f", DummyChars, &version); // get the type and version 

        dummy = fscanf(infile, "%s %d", DummyChars, &nPoints);

	std::cout <<  nPoints << " points" << std::endl;

	colflag = false; sizeflag = false; textflag = false;

	// determine the number of parms XYZ is assumed
        dummy = fscanf(infile, "%s", DummyChars);

	if (strcmp(DummyChars,"XYZC") == 0)
		colflag = true; // colors
	if (strcmp(DummyChars,"XYZDC") == 0)
	{
		colflag = true; 
		sizeflag = true; // diameter and color
	}
	if (strcmp(DummyChars,"XYZDCT") == 0)
	{
		colflag = true; 
		sizeflag = true; // diameter and color
		textflag = true;
	}

	if (sizeflag) sizes = new float[nPoints];
	if (colflag) colors = new int[nPoints];
	if (textflag) comments = new char*[nPoints];
// to test
//nPoints = 8;

	//get the X,Y,Z offset of the set of points
        dummy = fscanf(infile, "%*s %lf %lf %lf",&p[0], &p[1], &p[2] ); 

	SetPosition(p);

	// Get two opposite corners of the object box
	// create the normals data structure

	positions = new Point3f[nPoints];


	// read points one at a time
	for (i = 0; i < nPoints; i++)
	{
            dummy = fscanf(infile, "%lf %lf %lf", &p[0], &p[1], &p[2]);
		positions[i][0] = p[0];
		positions[i][1] = p[1];
		positions[i][2] = p[2];

		if (sizeflag)
		{
                    dummy = fscanf(infile, "%f", &ptsize ); 
			sizes[i] = ptsize;
		}

		if (colflag)
		{	
                    dummy = fscanf(infile, "%s", colorcode ); 
			switch(colorcode[0])
			{
				case 'R': colors[i] = 0; break;
				case 'G': colors[i] = 1; break;
				case 'B': colors[i] = 2; break;
				case 'Y': colors[i] = 3; break;
				case 'M': colors[i] = 4; break;
				case 'A': colors[i] = 6; break;
				case 'C': colors[i] = 7; break;
				case 'D': colors[i] = 8; break;
				case 'E': colors[i] = 9; break;
				default:
					colors[i] = DEFAULT_COLOR;
			}
		}
		if (textflag)
		{
			j = 0;
			while((c = getc(infile)) != '\"'); // get to first quote
			while((c = getc(infile)) != '\"') // get to second quote
			{
				//fprintf(stderr,"%c",c);
				inputstring[j] = c;
				++j;
			}
			inputstring[j] = '\0';
			comment = new char[j+1];
			strcpy(comment, inputstring);
			comments[i] = comment;
		}
		//printf("%s \n", comment);
	}

	return(0);
}

/*--------------------------------------------------------------------------*/
void Points::makeCone()
{
	Point3f normal;
	int i;
	float x,y,z,rad,deg;

	rad = CONE_BASE;

	cone[CONE_SEGS][0] = 0.0;
	cone[CONE_SEGS][1] = 0.0;
	cone[CONE_SEGS][2] = 0.5f*rad;

	cone[CONE_SEGS+1][0] = 0.0;
	cone[CONE_SEGS+1][1] = 0.0;
	cone[CONE_SEGS+1][2] = -50.0;
	
	for ( i = 0; i < CONE_SEGS; i++) // nStrips
	{
		deg = float(i*360)/CONE_SEGS;
		x = (float)(rad*sin(deg*DEGTORADIANS));
		y = (float)(rad*cos(deg*DEGTORADIANS));
		z = -0.5f*rad;
		cone[i][0] = x;
		cone[i][1] = y;
		cone[i][2] = z;
	}

	for ( i = 0; i < (CONE_SEGS-1); i++) // nStrips
	{
		CalcNormal(cone[CONE_SEGS],cone[i],cone[i+1],normal);

/*		coneNormals[i].nx = (GLshort)(normal[0]*32767);
		coneNormals[i].ny = (GLshort)(normal[1]*32767);
		coneNormals[i].nz = (GLshort)(normal[2]*32767);*/
		coneNormals[i][X] = (GLshort)(normal[0]*32767);
		coneNormals[i][Y] = (GLshort)(normal[1]*32767);
		coneNormals[i][Z] = (GLshort)(normal[2]*32767);
	
	}
	// now do the last one
	CalcNormal(cone[CONE_SEGS],cone[CONE_SEGS-1],cone[0],normal);
/*	coneNormals[CONE_SEGS-1].nx = (GLshort)(normal[0]*32767);
	coneNormals[CONE_SEGS-1].ny = (GLshort)(normal[1]*32767);
	coneNormals[CONE_SEGS-1].nz = (GLshort)(normal[2]*32767);*/
	coneNormals[CONE_SEGS-1][X] = (GLshort)(normal[0]*32767);
	coneNormals[CONE_SEGS-1][Y] = (GLshort)(normal[1]*32767);
	coneNormals[CONE_SEGS-1][Z] = (GLshort)(normal[2]*32767);
}

/*--------------------------------------------------------------------------*/
void Points::CalcNormal(Point3f v1, Point3f v2, Point3f v3, Point3f result)
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
