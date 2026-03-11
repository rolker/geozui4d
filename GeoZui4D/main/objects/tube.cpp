// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
// tube.cpp: implementation of the tube class.
//
//////////////////////////////////////////////////////////////////////

#include "tube.h"
#include <math.h>
#include "utilities/CubicSpline.h"
#include "widgets/Sv_Standards.h"
#include "string.h"
#include <gz4d/glo/Vertex.h>

extern int dominantEye;

#define PI 3.1415926536
#define ZERO_TOLERANCE 0.00001

REGISTER_OBJECT(Tube, "tube");

const char *TubeNames[] = {OBJECT_NAMES, "N_RECORDS", "SPLINE", "NO_SPLINE", "DIA", "COLOR", "XYZ", ""};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Tube::Tube()
{
	SetTypeAndVersion("TUBE", 1, 0);
	SetKeywordTable(TubeNames);

	spline = GL_FALSE;
	drawseg = 0;

	nFacets = 20;
	maxDiameter = 2.0;

	size = NULL;
	discs = NULL;
	norms_array = NULL;
	Colors = NULL;
	drawLines = GL_FALSE;
}

Tube::~Tube()
{
	if (size)
		delete size;
	if (discs)
		delete [] discs;
	if (norms_array)
		delete [] norms_array;
	if (Colors)
		delete [] Colors;
}

void Tube::DrawLowRes(float degradation, Window3D *)
{
	int i,gap;
	
	if (renderingForPicking)
		gap = 1;
	else
 		gap =  1 + int(6.0*degradation);
 
	glShadeModel(GL_FLAT);

	//for(i = 0; i < nPoints-1; i++)
	for(i = 0; i < nPoints-gap; i +=gap)
	{
		DrawShadedRing(i,gap);
	}

	doneFlag = drawseg = 0;
}


int Tube::DrawHighRes(int eye, Window3D *win)
{
	int j;
	
	if (doneFlag == 0)
	{
		glShadeModel(GL_FLAT);

		//draw 10 segments at once
		for (j=0;j<10;++j) 
		{
			if ((drawseg+j) < (nPoints -1)) 
			{
					DrawShadedRing(drawseg+j,1);
			}
			else
				break; //setting of doneflag changed by mdp 9/11/2000 (was premature in one eye)
		}
		if (eye == dominantEye || eye == MONO_EYE)
			drawseg=drawseg+10;

		if (drawseg >= nPoints)
			doneFlag = 1;
	}
	else
		doneFlag = 1;
	
	return doneFlag;
}

void Tube:: DrawShadedRing(int seg, int gap)
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;
	vNorms *norm1, *norm2;
	Point3f *disc1, *disc2;
	disc1 = discs[seg];
	disc2 = discs[seg+gap];// gap was 1
	
	norm1 = norms_array[seg];
	norm2 = norms_array[seg+gap]; // gap was 1

	if (nParms > 0)
		glColor3fv(Colors[seg]);
	else
	{
		if (seg%2 == 1) 	
			glColor3fv(Color1);
		else 
			glColor3fv(Color2);
	}

	//Draw line to make visible from far away.
	if (drawLines)
	{
		glBegin(GL_LINES);
		gz4d::glo::Vertex(Path[seg]);
                gz4d::glo::Vertex(Path[seg + gap]);
		glEnd();
	}

	//do pairs of points
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < nFacets; i+=gap)
	{
		if (!renderingForPicking)
			//glNormal3s(norm1[i].nx, norm1[i].ny, norm1[i].nz);
			glNormal3sv(norm1[i]);
		glVertex3fv(disc1[i]);
		if (!renderingForPicking)
			//glNormal3s(norm2[i].nx, norm2[i].ny, norm2[i].nz);
			glNormal3sv(norm2[i]);
		glVertex3fv(disc2[i]);
	} 
	// finish the ring
	if (!renderingForPicking)
		//glNormal3s(norm1[0].nx, norm1[0].ny, norm1[0].nz);
		glNormal3sv(norm1[0]);
	glVertex3fv(disc1[0]);
	if (!renderingForPicking)
		//glNormal3s(norm2[0].nx, norm2[0].ny, norm2[0].nz);
		glNormal3sv(norm2[0]);
	glVertex3fv(disc2[0]);
	glEnd();
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void Tube::ResetHighRes()
{
	doneFlag = drawseg = 0;
}

/*--------------------------------------------------------------------------*/
int Tube::processField(int option, char **fields)
{
	int i = 1;

	switch(option)
	{
		case N_RECORDS:
			nPoints = ConvertInt(i);
			break;
		case SPLINE:
			spline = GL_TRUE;
			break;
		case NO_SPLINE:
			spline = GL_FALSE;
			break;
		case DIA:
			maxDiameter = ConvertFloat(i);
			break;
		case COLOR:
			ConvertFloatPoint(Color1, i);
			ConvertFloatPoint(Color2, i);
			break;
		case XYZ:
			nParms = ConvertInt(i);
			break;
		default:
			return Object::processField(option, fields);
			break;
	}
	return 0;
}

/*--------------------------------------------------------------------------*/
// documented in Object
int Tube::readFrom(FILE *infile)
{
	int i;
	float x, y, z;
	float v1,v2,p;

	//parse header
	if (Object::readFrom(infile))
		return 1;

	if (nParms > 0)
		Colors = new Point3f[nPoints];
	if (nParms > 1)
		size = new float[nPoints];
	
	// reate the normals data structure
	Path.resize(nPoints);

	//cerr << "DIM " << nParms << "\n";

	// read points one at a time
	for (i = 0; i < nPoints; i++)
	{
		//if(strcmp("XYZ",XYorXYZ)==0)
		{
                    int ret = 0;
			if (reversedCoordinates)
				ret = fscanf(infile, "%f %f %f", &y, &x, &z);
			else
				ret = fscanf(infile, "%f %f %f", &x, &y, &z);
                        if(ret == 3)
                        {
			Path[i][X] = x;
			Path[i][Y] = y;
			Path[i][Z] = z;
                        }
			//cerr << x << " " << y << " " << z << "\n";
		}

		if (nParms > 0) 
		{
			if(fscanf(infile, "%f",&v1)==1)
                        {
			p = v1/* / float(16.0)*/;
			Colors[i][0] = Color2[0]*((float)1-p) + Color1[0]*p;
			Colors[i][1] = Color2[1]*((float)1-p) + Color1[1]*p;
			Colors[i][2] = Color2[2]*((float)1-p) + Color1[2]*p;			
                        }
		}
		if (nParms > 1)
		{
			if(fscanf(infile, "%f",&v2)==1)
			 size[i] = v2*maxDiameter;
		}
	}
	
	projectXY(Path);
	
	bbox.set(Path[0]);
	for (i = 1; i < nPoints; i++)
	{
		bbox.expand(Path[i]);
	}
	//cerr << "****** " << nParms << "\n";

	if (spline)  // resample points
		MakeSpline();

	norms_array = new vNorms*[nPoints];
	discs = new Point3f*[nPoints];

	//allocate memory for normals

	for (i = 0; i < nPoints; i++)
	{
		norms_array[i] = new vNorms[nFacets];
	}
	
	// allocate memory for disks
	for (i = 0; i < nPoints; i++)
	{
		discs[i] = new Point3f[nFacets];
	}

	//cout.flush();
	// done reading file

	//printf("\nDone loading.\n");
	
	// calculate normals
	
	std::cout << "Calculating normals..." << std::endl;
	
	CalcAllNormals();
	
	//printf("Done.\n");
	
	// done
	editedFlag = GL_FALSE;
	
	return(0);
}


void Tube::CalcAllNormals()
{
	Point3f N = {0.0,0.0,0.0},S = {0.0,0.0,0.0},F = {0.0,0.0,0.0};
	int i, j;

	float * circX;
	float * circY;
	float normX, normY, normZ;
	float dia;

    dia = 0.0;
    
	CalculateCircles(circX, circY);
	// we compute the normals to the disks one at a time

	// Need to prime the pump so as to compute the first disc
	// This is simply a permuted version of the first forward
	// vector
	N[0] = Path[1][2] - Path[0][2];
	N[1] = Path[1][0] - Path[0][0];
	N[2] = Path[1][1] - Path[0][1];

	for ( i = 0; i < nPoints-1; i++) // 
	{
		//First we calculate the forward vector
		if(i==0)
		{
			F[0] = Path[i+1][0] - Path[i][0];
			F[1] = Path[i+1][1] - Path[i][1];
			F[2] = Path[i+1][2] - Path[i][2];
		}
		else 
		{
			F[0] = Path[i+1][0] - Path[i-1][0];
			F[1] = Path[i+1][1] - Path[i-1][1];
			F[2] = Path[i+1][2] - Path[i-1][2];
		}

		CalcNormal(F,N,S);  // S is the resulting side vector
		CalcNormal(S,F,N);  // This is the resulting orthogonal vector
		//now rotate the disk into the S and N vectors  

		//cerr << " I  " << i << "\n";

		dia = maxDiameter;
		if(nParms == 2) dia = maxDiameter*size[i];

		for (j = 0; j < nFacets; j++)
		{	
			// rotate it into place
			normX = (N[0]*circX[j] + S[0]*circY[j]);
			normY = (N[1]*circX[j] + S[1]*circY[j]);
			normZ = (N[2]*circX[j] + S[2]*circY[j]);

/*			norms_array[i][j].nx = short(normX*32767);
			norms_array[i][j].ny = short(normY*32767);
			norms_array[i][j].nz = short(normZ*32767);*/
			norms_array[i][j][X] = short(normX*32767);
			norms_array[i][j][Y] = short(normY*32767);
			norms_array[i][j][Z] = short(normZ*32767);

			discs[i][j][0] = Path[i][0] + normX*dia;		
			discs[i][j][1] = Path[i][1] + normY*dia;		
			discs[i][j][2] = Path[i][2] + normZ*dia;		
		}

	}
	if(nParms == 2)
		dia = maxDiameter*size[nPoints-1];
	// do the last disk
	for (j = 0; j < (nFacets); j++)
	{	
/*		norms_array[nPoints-1][j].nx = norms_array[nPoints-2][j].nx;
		norms_array[nPoints-1][j].ny = norms_array[nPoints-2][j].ny;
		norms_array[nPoints-1][j].nz = norms_array[nPoints-2][j].nz;*/
		norms_array[nPoints-1][j][X] = norms_array[nPoints-2][j][X];
		norms_array[nPoints-1][j][Y] = norms_array[nPoints-2][j][Y];
		norms_array[nPoints-1][j][Z] = norms_array[nPoints-2][j][Z];

		normX = (N[0]*circX[j] + S[0]*circY[j]);
		normY = (N[1]*circX[j] + S[1]*circY[j]);
		normZ = (N[2]*circX[j] + S[2]*circY[j]);

		discs[nPoints-1][j][0] = normX*dia + Path[nPoints-1][0];
		discs[nPoints-1][j][1] = normY*dia + Path[nPoints-1][1];
		discs[nPoints-1][j][2] = normZ*dia + Path[nPoints-1][2];
	}	

}

int Tube::CalcNormal(Point3f v1, Point3f v2, Point3f result)
{
	// do the cross product defined by the vectors
	// v1 - v2

	float length;

	// calculate the cross product

	result[X] = v1[Y]*v2[Z] - v1[Z]*v2[Y];
	result[Y] = v1[Z]*v2[X] - v1[X]*v2[Z];
	result[Z] = v1[X]*v2[Y] - v1[Y]*v2[X];

	// and normalize to a unit vector
	length = (float)sqrt(result[0]*result[0] 
		+ result[1]*result[1] + result[2]*result[2]);

	if (length < ZERO_TOLERANCE)
		return GL_FALSE;

	result[X] /= length;
	result[Y] /= length;
	result[Z] /= length;

	return GL_TRUE;
}

/*---------------------------------------------------------------------------*/
void Tube :: MakeSpline()
{
    CubicSpline sp(Path);

    nPoints *= 9;
    std::vector<gz4d::Point<double> > newPath(nPoints);
    sp.resample(newPath);
    Path = newPath;
}

/*---------------------------------------------------------------------------*/
/** This procedure calculates the circle points based on nFacets, and stores
 *	the x and y values in new arrays, passed back through the x and y
 *	argument pointers.
 */
void Tube::CalculateCircles(float *&circX, float *&circY)
{
	float angleIncrement = (float)(2.0 * PI / (float)nFacets);
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
