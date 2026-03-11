// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
// dtm.cpp: implementation of the dtm class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>
#include "dtm.h"
#include <filesystem>

#define EARTH_RAD	6371000.0
#define PI 3.1415926536
#define VERT_SCALE	3.0
#define HIGH_RES_ROW_STEP 10

using namespace std;

extern int dominantEye;

REGISTER_OBJECT(Dtm, "dem");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/*--------------------------------------------------------------------------*/
Dtm::Dtm()
{
	maxx = 0;
	maxy = 0;
	drawrow = 0;
			
}

/*--------------------------------------------------------------------------*/
Dtm::~Dtm()
{
	int i;
	
	if (maxx > 0 && maxy > 0)
	{
		
		for (i = 0; i < maxy; i++)
		{
			delete norms_array[i];
		}
		delete norms_array;
		
		for (i = 0; i < maxy; i++)
		{
			delete dem[i];
		}
		delete dem;
		
	}
	
}

/*---------------------------Rendering Functions----------------------------*/
/*--------------------------------------------------------------------------*/
void Dtm::DrawLowRes(float degradation, Window3D *)
{	int i;
	int deg;

	deg = (int)(degradation*maxx/20);

	if (deg < 1)
	{
		deg = 1;
	}
	
	glShadeModel(GL_SMOOTH);
		
	EnableTexture();

	for(i = 0; i < maxx-deg-1; i+= deg)
	{
		DrawShadedStrip(i,deg);
	}
	DisableTexture();
}

/*--------------------------------------------------------------------------*/
int Dtm::DrawHighRes(int eye, Window3D *win)
{	int currentRow, last; //mdp 3/30/2000

	if (!doneFlag)
	{		
		glShadeModel(GL_SMOOTH);
		
		EnableTexture();

		//draw several rows at one (mdp 3/30/2000)
		last = drawrow + HIGH_RES_ROW_STEP;
		if (last > maxy - 1)
		{   last = (int)(maxy - 1);
		    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
			    doneFlag = 1;
		}
		for (currentRow = drawrow; currentRow < last; currentRow++)
			DrawShadedStrip(currentRow,1);

		if (eye == dominantEye || eye == MONO_EYE) //update our idea of the current row unless we're on the first of two eyes.
		 	drawrow = last;
		
		DisableTexture();
	}
	else
		doneFlag = 1;

	return doneFlag;
}

/*--------------------------------------------------------------------------*/
void Dtm:: DrawShadedStrip(int row, int gap)
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;
	vNorms *row1;
	vNorms *row2;
	float *h1;
	float *h2;

        glColor3f( 1, 1, 1 );  // added by raf 8/1/00

	if (gap < 1) //case added by mdp 2/15/2000
	    gap = 1;

	row1 = norms_array[row];
	row2 = norms_array[row+gap];

	h1 = dem[row];
	h2 = dem[row + gap];

	glBegin(GL_TRIANGLE_STRIP);
	for (i = 0; i < maxy; i+=gap)
	{

		//glNormal3s(row1[i].nx, row1[i].ny, row1[i].nz);
		glNormal3sv(row1[i]);
		glVertex3f(float(i), float(row), h1[i]);
		

		//glNormal3s(row2[i].nx, row2[i].ny, row2[i].nz);
		glNormal3sv(row2[i]);
		glVertex3f(float(i), float(row+gap), h2[i]);			
	}
	glEnd();
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void Dtm::ResetHighRes()
{
	doneFlag = drawrow = 0;
}

/*--------------------------------------------------------------------------*/
void Dtm::CalculateNormals()
{
	float v1,v2,v3;
	int i, j;
	float dhx, dhy, dhz, l;

	// use forward differences to calculate normals
	
	for ( i = 0; i < maxx-1; i++)
	{
		for (j = 0; j < maxy-1; j++)
		{		
			v1 = dem[i][j];		
			v2 = dem[i+1][j];	
			v3 = dem[i][j+1];
	
			dhx = v1 - v2;
			dhz = v1 - v3;
			// we assume the grid gap is 1.0
	
			l = (float)sqrt(dhx*dhx + dhz*dhz + 1.0);
	
			dhx /= l;
			dhz /= l;
			dhy = (float)(1.0/l);
/*			norms_array[i][j].nx = (GLshort)(32767*dhx);
			norms_array[i][j].ny = (GLshort)(32767*dhy);
			norms_array[i][j].nz = (GLshort)(32767*dhz);*/
			norms_array[i][j][X] = (GLshort)(32767*dhx);
			norms_array[i][j][Y] = (GLshort)(32767*dhy);
			norms_array[i][j][Z] = (GLshort)(32767*dhz);
		}
/*		norms_array[i][(int)maxy-1].nx = norms_array[i][(int)maxy-2].nx;
		norms_array[i][(int)maxy-1].ny = norms_array[i][(int)maxy-2].ny;
		norms_array[i][(int)maxy-1].nz = norms_array[i][(int)maxy-2].nz;*/
		norms_array[i][(int)maxy-1][X] = norms_array[i][(int)maxy-2][X];
		norms_array[i][(int)maxy-1][Y] = norms_array[i][(int)maxy-2][Y];
		norms_array[i][(int)maxy-1][Z] = norms_array[i][(int)maxy-2][Z];
	}
	for (i = 0; i < maxx; i++) // make the last row the same
	{
/*		norms_array[(int)(maxx-1)][i].nx = norms_array[(int)(maxx-2)][i].nx;
		norms_array[(int)(maxx-1)][i].ny = norms_array[(int)(maxx-2)][i].ny;
		norms_array[(int)(maxx-1)][i].nz = norms_array[(int)(maxx-2)][i].nz;*/
		norms_array[(int)(maxx-1)][i][X] = norms_array[(int)(maxx-2)][i][X];
		norms_array[(int)(maxx-1)][i][Y] = norms_array[(int)(maxx-2)][i][Y];
		norms_array[(int)(maxx-1)][i][Z] = norms_array[(int)(maxx-2)][i][Z];
	}
}

/*-------------------------File-related Functions---------------------------*/
/*--------------------------------------------------------------------------*/
// documented in Object
int Dtm::readFrom(FILE *infile)
{
	char inln[144];
	int DummyInt;
	float DummyFloat;
	float LatB, LatT, LongL, LongR;
	char DummyChars[160];
	int i;
	int x, y;
	Point3d gr;

	float gridcell; // the dimensions of a cell

	cout << "Loading " << GetFilename().filename() << " ... \n";
	cout.flush();
	
	// area  description

	fscanf(infile, "%144c", inln);
	inln[143]=0;
	cout << inln << "\n";
	cout.flush();

	// DEM Level code
	fscanf(infile, "%6d", &DummyInt);

	// pattern code
	fscanf(infile, "%6d", &DummyInt);
	cerr << "pattern code " << DummyInt << "\n";

	// Planimetric reference system code
	fscanf(infile, "%6d", &DummyInt);

	// Zone code
	fscanf(infile, "%6d", &DummyInt);

	// 15 useless floats

	for (i=0; i<15; i++)
	{
		fscanf(infile, "%24c", DummyChars);
	}

	// Units codes
	fscanf(infile, "%6d", &DummyInt);
	cerr << "units code " << DummyInt << "\n";

	fscanf(infile, "%6d", &DummyInt);

	// Number (n) of sides in the polygon 
	fscanf(infile, "%6d", &DummyInt);

	// ground coordinates of the four corners of the DEM
	fscanf(infile, "%24c", DummyChars);
	DummyChars[20]='E';
	sscanf(DummyChars, "%24e",&LongL);

	fscanf(infile, "%24c", DummyChars);
	DummyChars[20]='E';
	sscanf(DummyChars, "%24e",&LatB);

	for (i=0; i<2; i++)
	{	
		fscanf(infile, "%24c", DummyChars);
		sscanf(DummyChars, "%24f",&DummyFloat);
	}

	fscanf(infile, "%24c", DummyChars);
	DummyChars[20]='E';
	sscanf(DummyChars,"%e", &LongR);

	fscanf(infile, "%24c", DummyChars);
	DummyChars[20]='E';
	sscanf(DummyChars, "%24e",&LatT);

	for (i=0; i<2; i++)
	{	
		fscanf(infile, "%24c", DummyChars);
		sscanf(DummyChars, "%24f",&DummyFloat);
	}
	
	cerr << "Long:  " << LongL << "  " << LongR << "\n";
	cerr << "Lat:   " << LatB  << "  " << LatT << "\n";

	// Min and Max elevations

		fscanf(infile, "%24c", DummyChars);
		DummyChars[24] = 0;
		DummyChars[20]='E';
		sscanf(DummyChars,"%f",&minheight);

		fscanf(infile, "%24c", DummyChars);
		DummyChars[24]=0;
		DummyChars[20]='E';
		sscanf(DummyChars,"%f",&maxheight);

		cout << "Max: " << maxheight << "  Min: " << minheight << "\n";

		bbox.t = maxheight;
		bbox.b = minheight;

	gr[0] = -LatB / VERT_SCALE;
	gr[1] = LongL / VERT_SCALE;
	gr[2] = 0;
	SetPosition(gr);


 	bbox.w = gr[0];
	bbox.e = gr[0] + LongR-LongL;
	bbox.n = gr[1] + LatT-LatB;
	bbox.s = gr[1];


	
	//kludge a geoOffset to make comparisons (mdp 3/14/2000)
//	geoOffset[0] = 540000.0 - LatB / VERT_SCALE;
//	geoOffset[1] = 5500000.0 + LongL / VERT_SCALE;
//	geoOffset[2] = 0;

	// Counterclockwise angle from the primary axis 

	fscanf(infile, "%24c", DummyChars);

	// Accuracy code

	fscanf(infile, "%1d", &DummyInt);

	// X resolution
	fscanf(infile, "%12c", DummyChars);
	DummyChars[12]=0;
	sscanf(DummyChars,"%f",&xres);

	// Y resolution
	fscanf(infile, "%12c", DummyChars);
	DummyChars[12]=0;
	sscanf(DummyChars,"%f",&yres);

	// Z resolution
	fscanf(infile, "%12c", DummyChars);
	DummyChars[12]=0;
	sscanf(DummyChars,"%f",&zres);

	cout << "xres: " << xres << " yres: " << yres << " zres: " << zres << "\n";

	gridcell = (float)(2.0*EARTH_RAD*PI/(360.0*60.0*60.0)*xres);  // xres is seconds

	cerr << "Gridcellsize  :" << gridcell << "\n";
	
	fscanf(infile, "%6d", &DummyInt);

	// Max X

	fscanf(infile, "%6d", &DummyInt);
	maxx = (float)DummyInt;

	// junk

	fscanf(infile, "%160c", DummyChars);

	// Done reading header...

	// Coordinates of strip
	fscanf(infile, "%6d", &DummyInt);

	fscanf(infile, "%6d", &DummyInt);

	// Max Y

	fscanf(infile, "%6d", &DummyInt);
	maxy = (float)DummyInt;

	// X's (1)

	fscanf(infile, "%6d", &DummyInt);

	// ground planimetric coordinates

	fscanf(infile, "%120c", DummyChars);

	// figure out the height scale
	
	heightscale = (float)(VERT_SCALE/gridcell);

	cout << "VERT_S: " << VERT_SCALE << ", gridcell: " << gridcell << "\n"; 
	cout << "Heightscale:  " << heightscale << "\n";

	maxheight*= heightscale; 
	minheight*= heightscale;

	// allocate arrays

	cout << "Allocating " << maxx << " x " << maxy << " array.\n";
	cout.flush();

	// allocate the vertex array

	norms_array = new vNorms*[(int)maxy];
	if (norms_array == NULL)
	{
		cerr << "Can't allocate Normals array! Buy more ram!\n";
		return 1;
	}

	for (i = 0; i < maxy; i++)
	{
		norms_array[i] = new vNorms[(int)maxx];
		if (norms_array[i] == NULL)
		{
			cerr << "Can't allocate Normals array! Buy more ram!\n";
			return 1;
		}

	}

	dem = new float*[(int)maxy];
	if ( dem == NULL)
	{
		cerr << "Can't allocate heights array! Buy more ram!\n";
		return 1;
	}

	for (i = 0; i < maxy; i++)
	{
		dem[i] = new float[(int)maxx];
		if ( dem[i] == NULL)
		{
			cerr << "Can't allocate heights array! Buy more ram!\n";
			return 1;
		}
	}

	// load the heights
	
	for (x = 0; x < maxx; x++)
	{
		if (x>0)
		{
			fscanf(infile, "%6d", &DummyInt);
			
			fscanf(infile, "%6d", &DummyInt);
			
			// Max Y
			
			fscanf(infile, "%6d", &DummyInt);
			
			// X's (1)
			
			fscanf(infile, "%6d", &DummyInt);
			
			// ground planimetric coordinates
			
			fscanf(infile, "%120c", DummyChars);
			
		}
		for (y = (int)(maxy-1); y >= 0; y--)
		{
			fscanf(infile, "%6d", &DummyInt);
			i = (int)(x*maxy + y);
			//dem[i] = DummyInt/heightscale;
			dem[x][y] = DummyInt*heightscale;
		}
	}
	cout.flush();
	// done reading file

	printf("\nDone loading.\n");
	
	// calculate normals
	
	printf("Calculating normals...");
	
	CalculateNormals();
	
	printf("Done.\n");
	
	// done
	
	return(0);
}

