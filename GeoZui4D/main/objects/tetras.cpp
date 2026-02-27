/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// Tetras.cpp: implementation of the Points class.
//  (copied from Points.cpp for test purposes only)
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>
//#include<GL/glut.h>
#include "tetras.h"

#define PI	3.1415926536
//#define TETRA_BASE	500.0
#define TETRA_BASE	300.0

using namespace std;

extern int dominantEye;

REGISTER_OBJECT(Tetras, "tetra");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Tetras::Tetras()
{

//	HBITMAP hBitmap;
//	HPALETTE hPalette;
//	bool bmpresult;
//	LPTSTR zFileName ="D:/visualizer/testbitmap.bmp";

	
	pointCount = 0;

	coneNormals = new vNorms[CONE_SEGS];
	cone = new Point3f[CONE_SEGS+2];

//
//	GLfloat emission[] = {(float)0.9, (float)0.9, (float)0.9, 1.0};
//	
	makeCone();

//	palette[0][0] = 1.0f; 	palette[0][1] = 0.2f; 	palette[0][2] = 0.2f;
//	palette[1][0] = 0.0f; 	palette[1][1] = 0.3f; 	palette[1][2] = 1.0f;
//	palette[2][0] = 0.3f; 	palette[2][1] = 1.0f; 	palette[2][2] = 0.3f;
//	palette[3][0] = 0.1f; 	palette[3][1] = 1.0f; 	palette[3][2] = 1.0f;
//	palette[4][0] = 0.7f; 	palette[4][1] = 0.7f; 	palette[4][2] = 0.7f;
//	palette[5][0] = 0.8f; 	palette[4][1] = 0.3f; 	palette[4][2] = 1.0f;
			
//set up for texture map
	glEnable(GL_DEPTH_TEST);
//	makeCheckImage();

//	HBITMAP hBitmap;
//	HPALETTE hPalette;
//	bool bmpresult;
//	LPTSTR zFileName ="D:/visualizer/testbitmap.bmp";

//	bmpresult = LoadBitmapFromBMPFile(zFileName, &hBitmap, &hPalette);
	
	
//	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
//	glTexImage2D(GL_TEXTURE_2D,0,3,128,128,
//		0,GL_RGB,GL_UNSIGNED_BYTE,&bm2);
//	glTexImage2D(GL_TEXTURE_2D,0,3,64,64,
//		0,GL_RGB,GL_UNSIGNED_BYTE,&bm3);
//	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
//	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
//	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);


}

Tetras::~Tetras()
{
		
}

void Tetras::DrawLowRes(float degradation, Window3D *)
{
	int i;

	glShadeModel(GL_FLAT);

	for(i = 0; i < nPoints; i++)
	{	
		glPushMatrix();
		glTranslatef(positions[i][0], positions[i][1], positions[i][2]);
		//
		glColor3fv(palette[colors[i]]);
		DrawLoresCone();
		glPopMatrix(); 
	}

	doneFlag = pointCount = 0;
	glColor3f(1.0,1.0,1.0);
}


int Tetras::DrawHighRes(int eye, Window3D *win)
{
	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT);
	if (!doneFlag)
	{
		if (pointCount < nPoints)
		{
			glPushMatrix();
			glTranslatef(positions[pointCount][0], positions[pointCount][1], positions[pointCount][2]);
	
			glColor3fv(palette[colors[pointCount]]);
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
	glEnable(GL_LIGHTING);

	return doneFlag;
}

/*--------------------------------------------------------------------------*/
/**	This function resets this object to start over drawing high res the next
 *	time DrawHighRes is called.
 */
void Tetras::ResetHighRes()
{
	doneFlag = pointCount = 0;
}

void Tetras:: DrawCone()
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;

//	glEnable(GL_TEXTURE_2D);

	//do pairs of points
	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(cone[CONE_SEGS]);

//	
	//	GLfloat emission[] = {(float)0.3, (float)0.3, (float)0.3, 0.0};
//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
//
	
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

//add bottom triangle

	glBegin(GL_TRIANGLES);

//	glNormal3s(coneNormals[3].nx,coneNormals[3].ny,coneNormals[3].nz);

//	glTexCoord2f(0.0,1.0);
	glVertex3fv(cone[2]);

//	glTexCoord2f(0.0,0.0);
	glVertex3fv(cone[1]);

//	glTexCoord2f(1.0,0.0);
	glVertex3fv(cone[0]);

	glEnd();

//	glDisable(GL_TEXTURE_2D);
	
//	emission[0] = (float)0.0;
//	emission[1] = (float)0.0;
//	emission[2] = (float)0.0;
//	emission[3] = (float)1.0;
//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	
//wsl comment out to remove the stick base
//	glBegin(GL_LINES);
//	glVertex3fv(cone[CONE_SEGS]);
//	glVertex3fv(cone[CONE_SEGS+1]);
//	glEnd();
}

void Tetras:: DrawLoresCone()
{
	// draw a single shaded triangle strip
	// need to change to row major order
	int i;
	glDisable(GL_LIGHTING);
//	glEnable(GL_TEXTURE_2D);

	//do pairs of points
	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(cone[CONE_SEGS]);
//wsl patch
//	for (i = 0; i < CONE_SEGS; i=i+8)
//	for (i = 0; i < CONE_SEGS; i=i+2)
	for (i = 0; i < CONE_SEGS; i++)  //NO DIFFERENCE FROM HIGH RES!!
	{
		//glNormal3s(coneNormals[i].nx, coneNormals[i].ny, coneNormals[i].nz);
		glNormal3sv(coneNormals[i]);
		glVertex3fv(cone[i]);
	}
	//glNormal3s(coneNormals[0].nx, coneNormals[0].ny, coneNormals[0].nz);
	glNormal3sv(coneNormals[0]);
	glVertex3fv(cone[0]);
	glEnd();

//add bottom triangle
	glBegin(GL_TRIANGLES);

//	glNormal3s(coneNormals[3].nx,coneNormals[3].ny,coneNormals[3].nz);

//	glVertex3fv(cone[2]);
//	glVertex3fv(cone[1]);
//	glVertex3fv(cone[0]);

//	glTexCoord2f(0.0,1.0);
	glVertex3fv(cone[2]);

//	glTexCoord2f(0.0,0.0);
	glVertex3fv(cone[1]);

//	glTexCoord2f(1.0,0.0);
	glVertex3fv(cone[0]);

	glEnd();

//	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

//wsl comment out
//	glBegin(GL_LINES);
//	glVertex3fv(cone[CONE_SEGS]);
//	glVertex3fv(cone[CONE_SEGS+1]);
//	glEnd();
}

// documented in Object
int Tetras::readFrom(FILE *infile)
{
	char DummyChars[160];
//wsl 2/21/01
//	char animal[10];
	int colorcode;
	int i;
	Point3d Position;

//wsl 2/9/01
	int iii;
	float pal0, pal1, pal2;
//wsl end

//attempt to read in a bmp file:

//	HBITMAP hBitmap;
//	HPALETTE hPalette;
//	bool bmpresult;
//	LPTSTR zFileName ="D:/visualizer/testbitmap.bmp";
//	bmpresult = LoadBitmapFromBMPFile(zFileName, &hBitmap, &hPalette);

//
        int dummy = fscanf(infile, "%s", DummyChars);// get first string

	cout << DummyChars << "\n";
	cout.flush();
	
//wsl 2/9/01
//read a 64 X 3 colormap	
	for (iii=0;iii<64;iii++)
	{

            dummy = fscanf(infile,"%f %f %f",&pal0, &pal1, &pal2);
		printf("pal0 %d %f\n",iii,pal0);
		palette[iii][0]=pal0;
		palette[iii][1]=pal1;
		palette[iii][2]=pal2;
		printf("palette[iii][0] %d %f\n",iii,palette[iii][0]);
		printf("palette[iii][1] %d %f\n",iii,palette[iii][1]);
		printf("palette[iii][2] %d %f\n",iii,palette[iii][2]);
	}

//	cout << "palette0x0  " << palette[0][0]  << "\n";		
//	cout << "palette63x2 " << palette[63][2] << "\n";
//	cout.flush();
//wsl end

        dummy = fscanf(infile, "%d", &nPoints);

	cout <<"Points " <<  nPoints << "\n";
	cout.flush();

// to test
//	nPoints = 8;

	//get the X,Y,Z position of the object
        dummy = fscanf(infile, "%lf %lf %lf", &Position[0],&Position[1],&Position[2]); 
	
	SetPosition(Position);

// Get two opposite corners of the object box

	// create the normals data structure

	positions = new Point3f[nPoints];
	colors = new int[nPoints];

	// read points one at a time
	for (i = 0; i < nPoints; i++)
	{
//		fscanf(infile, "%f %f %f %s", &PositionX, &PositionY, &PositionZ, animal ); 
            dummy = fscanf(infile, "%lf %lf %lf %d", &Position[0],&Position[1],&Position[2], &colorcode ); 

//	if (i<100) printf("colorcode= %d\n",colorcode);
		
		positions[i][0] = Position[0];
		positions[i][1] = Position[1];
		positions[i][2] = Position[2];

//wsl 2/21/01

		colors[i]=colorcode;				

		if (i<66) printf("i=, colors[i]= %d %d\n",i,colors[i]);
		
//		switch(animal[0])
//		{
//			case 'A': colors[i] = 0; break;
//			case 'B': colors[i] = 1; break;
//			case 'C': colors[i] = 2; break;
//			case 'D': colors[i] = 3; break;
//			case 'E': colors[i] = 4; break;
//wsl 2/21/01
//			default: colors[i] = 5;
//			default: colors[i] = 6;
//		}

	}
//
//	printf("image handle= %d\n",bmpresult);
//
	cout.flush();
	
// done reading file
	printf("\nDone loading.\n");
	
	return(0);
}


void Tetras::makeCone()
{
	Point3f normal;
	int i;
	float x,y,z,rad,deg;

	rad = TETRA_BASE;

	cone[CONE_SEGS][0] = 0.0;
	cone[CONE_SEGS][1] = 0.0;
//wsl patch
//	cone[CONE_SEGS][2] = 1.5*rad;
//	cone[CONE_SEGS][2] = 0.5*rad;
//	cone[CONE_SEGS][2] = -0.0;      /*flat disk*/
//	cone[CONE_SEGS][2] = -0.5*rad;  /*inverted cone*/
	cone[CONE_SEGS][2] = rad;       /*for tetrahedrons centered on coords*/

	cone[CONE_SEGS+1][0] = 0.0;   /*not used with tetrahedrons*/
	cone[CONE_SEGS+1][1] = 0.0;   /*not used with tetrahedrons*/
	cone[CONE_SEGS+1][2] = -50.0; /*not used with tetrahedrons*/
	
	for ( i = 0; i < CONE_SEGS; i++) // nStrips
	{
		deg = float(i*360)/CONE_SEGS;
		x = rad*sin(deg*PI/180.0);
		y = rad*cos(deg*PI/180.0);
//wsl patch
//		z = -1.5*rad;
//		z = -0.5*rad;
//		z = 0.0;       /*flat disk*/
//		z = 0.5*rad;   /*inverted cone*/
		z = -0.5*rad;  /*for tetrahedrons*/

		cone[i][0] = x;
		cone[i][1] = y;
		cone[i][2] = z;
	}

//replaced for tetrahedrons
/*
	for ( i = 0; i < (CONE_SEGS-1); i++) // nStrips
	{
		CalcNormal(cone[CONE_SEGS],cone[i],cone[i+1],normal);

		coneNormals[i].nx = (GLshort)(normal[0]*32767);
		coneNormals[i].ny = (GLshort)(normal[1]*32767);
		coneNormals[i].nz = (GLshort)(normal[2]*32767);
	
	}
	// now do the last one
	CalcNormal(cone[CONE_SEGS],cone[CONE_SEGS-1],cone[0],normal);
	coneNormals[CONE_SEGS-1].nx = (GLshort)(normal[0]*32767);
	coneNormals[CONE_SEGS-1].ny = (GLshort)(normal[1]*32767);
	coneNormals[CONE_SEGS-1].nz = (GLshort)(normal[2]*32767);
*/

// 	CalcNormal(cone[3],cone[0],cone[1],normal);
// 	coneNormals[0].nx = (GLshort)(normal[0]*32767);
// 	coneNormals[0].ny = (GLshort)(normal[1]*32767);
// 	coneNormals[0].nz = (GLshort)(normal[2]*32767);
// 
// 	CalcNormal(cone[3],cone[1],cone[2],normal);
// 	coneNormals[1].nx = (GLshort)(normal[0]*32767);
// 	coneNormals[1].ny = (GLshort)(normal[1]*32767);
// 	coneNormals[0].nz = (GLshort)(normal[2]*32767);
// 
// 	CalcNormal(cone[3],cone[2],cone[0],normal);
// 	coneNormals[2].nx = (GLshort)(normal[0]*32767);
// 	coneNormals[2].ny = (GLshort)(normal[1]*32767);
// 	coneNormals[2].nz = (GLshort)(normal[2]*32767);
// 
// 	CalcNormal(cone[2],cone[1],cone[0],normal);
// 	coneNormals[3].nx = (GLshort)(normal[0]*32767);
// 	coneNormals[3].ny = (GLshort)(normal[1]*32767);
// 	coneNormals[3].nz = (GLshort)(normal[2]*32767);

	CalcNormal(cone[3],cone[0],cone[1],normal);
	coneNormals[0][X] = (GLshort)(normal[0]*32767);
	coneNormals[0][Y] = (GLshort)(normal[1]*32767);
	coneNormals[0][Z] = (GLshort)(normal[2]*32767);

	CalcNormal(cone[3],cone[1],cone[2],normal);
	coneNormals[1][X] = (GLshort)(normal[0]*32767);
	coneNormals[1][Y] = (GLshort)(normal[1]*32767);
	coneNormals[0][Z] = (GLshort)(normal[2]*32767);

	CalcNormal(cone[3],cone[2],cone[0],normal);
	coneNormals[2][X] = (GLshort)(normal[0]*32767);
	coneNormals[2][Y] = (GLshort)(normal[1]*32767);
	coneNormals[2][Z] = (GLshort)(normal[2]*32767);

	CalcNormal(cone[2],cone[1],cone[0],normal);
	coneNormals[3][X] = (GLshort)(normal[0]*32767);
	coneNormals[3][Y] = (GLshort)(normal[1]*32767);
	coneNormals[3][Z] = (GLshort)(normal[2]*32767);
}


void Tetras::CalcNormal(Point3f v1, Point3f v2, Point3f v3, Point3f result)
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
	length = sqrt(result[0]*result[0] 
		+ result[1]*result[1] + result[2]*result[2]);

	result[0] /= length;
	result[1] /= length;
	result[2] /= length;

}

/*//added to read bitmap file
bool Tetras::LoadBitmapFromBMPFile(LPTSTR szFileName,
								  HBITMAP *phBitmap,
								  HPALETTE *phPalette)
{
	BITMAP bm;
	int iii, intout;
//	char bm2[128*128*3];
	char* bptr;
	FILE *fp;

	*phBitmap = NULL;
	*phPalette = NULL;

	*phBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (*phBitmap == NULL)
		return FALSE;
	GetObject(*phBitmap, sizeof(BITMAP), &bm);

	fp=fopen("d:/visualizer/scratchdata.txt","w");
	
	for (iii=0;iii<128*128*3;iii++)
	{
		bptr=(char*)bm.bmBits;
		bm2[iii]=bptr[iii];
		intout=(int)bptr[iii];
		if (iii<128*3)		{
//			fprintf(fp,"bm2= %x\n",bm2[iii]);
//			fprintf(fp,"bmBits= %x\n",bptr[iii]);
//			fprintf(fp," %d\n",intout+128);
			fprintf(fp," %x\n",bm2[iii]);
//			fprintf(fp," %d\n",(int)bm2[iii]+256);
		}
	}

	fclose(fp);
	
	return TRUE;

}

void Tetras::makeCheckImage(void)
{
	int i,j;
	unsigned char cc[64][64];
	//	int iii,jjj;

	for (i=0;i<64;i++)
	{
		for (j=0;j<64;j++)
		{
			if ( (i%2==0) && (j%2==0) || (i%2==1) && (j%2==1) )
				cc[i][j]=0xff;
			else
				cc[i][j]=0x00;	

			if (i%8==0)
				bm3[i][j][0]=(GLubyte)cc[i][j];
			else if (j%8==0)
				bm3[i][j][1]=(GLubyte)cc[i][j];
			else 
				bm3[i][j][2]=(GLubyte)cc[i][j];
		}
	}

}

*/
