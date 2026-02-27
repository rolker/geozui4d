/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/


#include"objects/Segment.h"

#include<iostream>
#include<math.h>
#include "utilities/Utilities.h"
#include"engine/Window3D.h"
#include <gz4d/glo/Color.h>

using namespace std;

Point3f * Segment::circle = NULL;
Point3f * Segment::cap = NULL;

#define SEGMENT_NUM_FACES 60
#define SEGMENT_NUM_CAP_SLICES 10


Segment::Segment()
{
	int i,j;
	float step = (float)((2.0*M_PI)/(float)(SEGMENT_NUM_FACES));
	float sliceStep = (float)((0.5*M_PI)/(float)(SEGMENT_NUM_CAP_SLICES));
	
	if(!circle)
	{
		circle = new Point3f[SEGMENT_NUM_FACES];
		for(i=0;i<SEGMENT_NUM_FACES;i++)
		{
			circle[i][0]=float(sin(i*step));
			circle[i][1]=0.0;
			circle[i][2]=float(cos(i*step));
		}
		circle[SEGMENT_NUM_FACES-1][0]=circle[0][0];
		circle[SEGMENT_NUM_FACES-1][1]=circle[0][1];
		circle[SEGMENT_NUM_FACES-1][2]=circle[0][2];
	}
	
	if(!cap)
	{
		cap = new Point3f[SEGMENT_NUM_FACES*SEGMENT_NUM_CAP_SLICES];
		for(i=0;i<SEGMENT_NUM_FACES;i++)
		{
			for(j=0;j<SEGMENT_NUM_CAP_SLICES;j++)
			{
				cap[i*SEGMENT_NUM_CAP_SLICES+j][0]=float(sin(i*step)*cos(j*sliceStep));
				cap[i*SEGMENT_NUM_CAP_SLICES+j][1]=-float(sin(j*sliceStep));
				cap[i*SEGMENT_NUM_CAP_SLICES+j][2]=float(cos(i*step)*cos(j*sliceStep));
			}
		}
	}
	
	startWidthScale = NULL;
	startHeightScale = NULL;
	endWidthScale = NULL;
	endHeightScale = NULL;

}

Segment::~Segment()
{
}

void Segment::DrawPoints(Window3D *win, float customStartColor[4], float customEndColor[4])
{
    DrawStartPoint(win,customStartColor);
    DrawEndPoint(win,customEndColor);
}

void Segment::DrawStartPoint(Window3D *win, float customStartColor[4])
{
    if (customStartColor)
        glColor4fv(customStartColor);
    else
        gz4d::glo::Color(*startColor);
    glVertex3f(start[X],start[Y],start[Z]);
}

void Segment::DrawEndPoint(Window3D *win, float customEndColor[4])
{
    if(customEndColor)
        glColor4fv(customEndColor);
    else
        gz4d::glo::Color(*endColor);

    glVertex3f(end[X], end[Y], end[Z]);
}

void Segment::Draw(float deg, Window3D *win)
{
    int i,lasti;

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
        DrawPoints(win);
    glEnd();
    glEnable(GL_LIGHTING);

    if(deg >= 0.5)
            return;

	int stepsize = 1 + (int)(2.0*deg*(SEGMENT_NUM_FACES/6.0-1));
		
	glPushMatrix();
	glTranslatef(start[X],start[Y],start[Z]);
	
	glRotatef(heading,0.0,0.0,1.0);
	glRotatef(elevation,1.0,0.0,0.0);
	
	gz4d::glo::Color(*startColor);
	drawCap(deg,startCap,startWidthScale,startHeightScale);
	
	glScalef(1.0,length,1.0);
	
	glBegin(GL_QUAD_STRIP);
		for(i=0;i<SEGMENT_NUM_FACES;i+=stepsize)
		{
			glNormal3f(circle[i][0]**startWidthScale,circle[i][1],circle[i][2]**startHeightScale);
			gz4d::glo::Color(*startColor);
			glVertex3f(circle[i][0]**startWidthScale,circle[i][1],circle[i][2]**startHeightScale);
			glNormal3f(circle[i][0]**endWidthScale,circle[i][1],circle[i][2]**endHeightScale);
			gz4d::glo::Color(*endColor);
			glVertex3f((float)(circle[i][0]**endWidthScale),(float)(circle[i][1]+1.0),(float)(circle[i][2]**endHeightScale));
			lasti = i;
		}
		if(lasti < SEGMENT_NUM_FACES-1)
		{
			i = SEGMENT_NUM_FACES-1;
			glNormal3f(circle[i][0]**startWidthScale,circle[i][1],circle[i][2]**startHeightScale);
			gz4d::glo::Color(*startColor);
			glVertex3f(circle[i][0]**startWidthScale,circle[i][1],circle[i][2]**startHeightScale);
			glNormal3f(circle[i][0]**endWidthScale,circle[i][1],circle[i][2]**endHeightScale);
			gz4d::glo::Color(*endColor);
			glVertex3f((float)(circle[i][0]**endWidthScale),(float)(circle[i][1]+1.0),(float)(circle[i][2]**endHeightScale));
		}
	glEnd();
	
	glPopMatrix();

	glPushMatrix();
	glTranslatef(end[X], end[Y], end[Z]);
	glRotatef(heading,0.0,0.0,1.0);
	glRotatef(elevation,1.0,0.0,0.0);
	glRotatef(180,0.0,0.0,1.0);
	
	gz4d::glo::Color(*endColor);
	drawCap(deg,endCap,endWidthScale,endHeightScale);

	glPopMatrix();
	
}

void Segment::drawCap(float deg, SegmentCapType type, float * ws, float * hs)
{
	int i,j;
	
	float ys;
	
	if(*hs<*ws)
		ys = *hs;
	else
		ys = *ws;
	
	int stepsize = 1 + (int)(2.0*deg*(SEGMENT_NUM_FACES/6.0-1));
	int slicestepsize = 1 + (int)(2.0*deg*(SEGMENT_NUM_CAP_SLICES/3.0-1));
	
	switch(type)
	{
	case Open:
		break;
	case Sphere:
		for(i=0;i<SEGMENT_NUM_FACES-stepsize;i+=stepsize)
		{
			glBegin(GL_QUAD_STRIP);
			for(j=0;	j<SEGMENT_NUM_CAP_SLICES; j+=slicestepsize)
			{
				glNormal3f(cap[i*SEGMENT_NUM_CAP_SLICES+j][0]**ws,cap[i*SEGMENT_NUM_CAP_SLICES+j][1]*ys,cap[i*SEGMENT_NUM_CAP_SLICES+j][2]**hs);
				glVertex3f(cap[i*SEGMENT_NUM_CAP_SLICES+j][0]**ws,cap[i*SEGMENT_NUM_CAP_SLICES+j][1]*ys,cap[i*SEGMENT_NUM_CAP_SLICES+j][2]**hs);
				glNormal3f(cap[(i+stepsize)*SEGMENT_NUM_CAP_SLICES+j][0]**ws,cap[(i+stepsize)*SEGMENT_NUM_CAP_SLICES+j][1]*ys,cap[(i+stepsize)*SEGMENT_NUM_CAP_SLICES+j][2]**hs);
				glVertex3f(cap[(i+stepsize)*SEGMENT_NUM_CAP_SLICES+j][0]**ws,cap[(i+stepsize)*SEGMENT_NUM_CAP_SLICES+j][1]*ys,cap[(i+stepsize)*SEGMENT_NUM_CAP_SLICES+j][2]**hs);
			}
			glNormal3f(0.0,(float)(-1.0*ys),0.0);
			glVertex3f(0.0,(float)(-1.0*ys),0.0);
			glVertex3f(0.0,(float)(-1.0*ys),0.0);
			glEnd();
		}
		glBegin(GL_QUAD_STRIP);
		for(j=0;	j<SEGMENT_NUM_CAP_SLICES; j+=slicestepsize)
		{
			glNormal3f(cap[i*SEGMENT_NUM_CAP_SLICES+j][0]**ws,cap[i*SEGMENT_NUM_CAP_SLICES+j][1]*ys,cap[i*SEGMENT_NUM_CAP_SLICES+j][2]**hs);
			glVertex3f(cap[i*SEGMENT_NUM_CAP_SLICES+j][0]**ws,cap[i*SEGMENT_NUM_CAP_SLICES+j][1]*ys,cap[i*SEGMENT_NUM_CAP_SLICES+j][2]**hs);
			glNormal3f(cap[j][0]**ws,cap[j][1]*ys,cap[j][2]**hs);
			glVertex3f(cap[j][0]**ws,cap[j][1]*ys,cap[j][2]**hs);
		}
		glNormal3f(0.0,(float)(-1.0*ys),0.0);
		glVertex3f(0.0,(float)(-1.0*ys),0.0);
		glVertex3f(0.0,(float)(-1.0*ys),0.0);
		glEnd();
		break;
	case Disc:
		glNormal3f(0.0,-1.0,0.0);
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0.0,0.0,0.0);
		for(i=0;i<SEGMENT_NUM_FACES-stepsize;i+=stepsize)
		{
			glVertex3f(circle[i][0]**ws,circle[i][1],circle[i][2]**hs);
		}
		glVertex3f(circle[0][0]**ws,circle[0][1],circle[0][2]**hs);
		glEnd();
		break;
	}
}

void Segment::calculateTransforms()
{
	float dx,dy,dz;

	dx = end[0]-start[0];
	dy = end[1]-start[1];
	dz = end[2]-start[2];
	
	length = (float)sqrt(dx*dx+dy*dy+dz*dz);
	
	if(dx!= 0.0 || dy!=0.0)
		heading = (float)((atan2(dy,dx)/(2*M_PI))*360.0-90.0);
	else
		heading = 0.0;
	
	elevation = (float)((asin(dz/length)/(2*M_PI))*360.0);
}


void Segment::setPositions(gz4d::Point<float> const &s, gz4d::Point<float> const &e)
{
    start = s;
    end = e;
    calculateTransforms();
}
	
void Segment::setColor(std::shared_ptr<gz4d::Color> &s, std::shared_ptr<gz4d::Color> &e)
{
    if(s)
        startColor = s;
    if(e)
        endColor = e;
}

void Segment::setCapTypes(SegmentCapType s,SegmentCapType e)
{
    startCap = s;
    endCap = e;
}

void Segment::setScales(float *sw, float *sh, float *ew, float *eh)
{
    if(sw)
        startWidthScale = sw;
    if(sh)
        startHeightScale = sh;
    if(ew)
        endWidthScale = ew;
    if(eh)
        endHeightScale = eh;
}

