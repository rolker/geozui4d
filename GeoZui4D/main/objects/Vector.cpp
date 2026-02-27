/* Copyright (c) 2002 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Vector.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class displays vector fields
 *
 *	Created by rja Jan 4th, 2002
 */
/*---------------------------------------------------------------------------*/
#include "Vector.h"
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "engine/PosableStateRelationships/RelationshipManager.h"
#include "engine/PosableStateRelationships/RelationshipQueryResult.h"
#include "engine/Window3D.h"
#include <iostream>
#include <string.h>
#include <math.h>
#include "engine/Scene3D.h"
#include "engine/InputMediator.h"
#include <gz4d/geo/Projection.h>
#include "MainWindow.h"

extern MainWindow * mainWindow;

using namespace std;

#define N_SEGS	18
#define RADIUS	0.1f

extern int dominantEye;

REGISTER_OBJECT(Vector, "vector");

const char *VectorNames[] = {OBJECT_NAMES, "UTM","LATLONG","STATIC","DYNAMIC",
        					 "SCALE", ""};

/*---------------------------------------------------------------------------*/
Vector::Vector()
: Object(GL_FALSE)
{
	SetTypeAndVersion("VECTOR", 1, 0);
	SetKeywordTable(VectorNames);
    dynamic = 0;
    scale = 1.0;
	pose = new ObjectPoseState(this);
    
    timestep = 0;
}

/*--------------------------------------------------------------------------*/
int Vector::processField(int option, char **fields)
{
    int i = 1;
    
	switch(option)
	{
		case STATIC:
			break;
        case DYNAMIC:
            dynamic = 1;
            break;
        case SCALE:
            scale = ConvertDouble(i);
            break;
		default:
		  return Object::processField(option, fields);
		  break;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
// documented in Object
int Vector::readFrom(FILE *infile)
{
    int i, j, retval, firstpoint;
//    PointVect tempvect;
    char ** fields;
        
        
        //parse header
    if (Object::readFrom(infile))
        return 1;


        
        // temp list to store the vects, sice we don't know yet how many we have
    struct templist
    {
        templist * next;
        PointVect * vect;
    };
        
    struct templol
    {
        double time;
        templol * next;
        templist * list;
        int count;
    };
        
    templol * lol = NULL;
    templol * lolitem = NULL;
    templist * listitem = NULL;
    
    timecount = 0;  
            
    gz4d::geo::Projection proj;
    firstpoint = 1;
    for(fields=NextLine(GL_FALSE); fields!=NULL; fields=NextLine(GL_FALSE))
    {
        i=0;
        if(strcmp("TIME",fields[0])==0)
        {
            lolitem = new templol;
            lolitem->next = lol;
            lolitem->time = ConvertDouble(++i);
            lolitem->list = NULL;
            lolitem->count = 0;
            lol = lolitem;
            timecount++;
            if((bbox.start == 0) && (bbox.end == 0))
            {
                    bbox.start = bbox.end = lolitem->time;
            }
            else
            {
                    if(lolitem->time < bbox.start)
                    {
                            bbox.start = lolitem->time;
                    }
                    if(lolitem->time > bbox.end)
                    {
                            bbox.end = lolitem->time;
                    }
            }
        }
        else
        {
            if(lol == NULL)
            {
                lolitem = new templol;
                lolitem->next = lol;
                lolitem->time = 0.0;
                lolitem->list = NULL;
                lolitem->count = 0;
                lol = lolitem;
                timecount++;
            }
            listitem = new templist;
            listitem->next = lolitem->list;
            lolitem->list = listitem;
            lolitem->count++;
            listitem->vect = new PointVect;
            ConvertDoublePoint(listitem->vect->position,i,reversedCoordinates);
            ConvertDoublePoint(listitem->vect->v,i);
            listitem->vect->v[0]*=scale;
            listitem->vect->v[1]*=scale;
            listitem->vect->v[2]*=scale;
            if(firstpoint)
            {
                if(!centerGiven && projectionRequired)
                {
                    retval = autoProjection(listitem->vect->position[X]);
                    if (retval)
                        return 1;
                    if (!centerGiven)
                    {
                        proj = gz4d::geo::Projection(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                        gz4d::Point<double> gr = proj(listitem->vect->position[Y], listitem->vect->position[X]);
                        bbox.e = bbox.w = gr[X];
                        bbox.n = bbox.s = gr[Y];
                        bbox.t = bbox.b = listitem->vect->position[Z];
                        gr[Z] = 0.0f;
                    }
                }
                firstpoint = 0;
            }
            if (projectionRequired)
            {
                gz4d::Point<double> xy = proj(listitem->vect->position[Y], listitem->vect->position[X]);

                listitem->vect->position[X] = xy[X];
                listitem->vect->position[Y] = xy[Y];
                bbox.expand(listitem->vect->position);
                listitem->vect->position[X]-=GetPosition(X);
                listitem->vect->position[Y]-=GetPosition(Y);
                listitem->vect->position[Z]-=GetPosition(Z);
            }
        }
    }
    
    // now that all the vectors are loaded in a list, move them to 
    // the array.
    
    vects = new Vects[timecount];
    
    double earliest;
    templol * earlylist;

    
    i=0;
    
    //sort the list of lists of vectors in chrono order by plucking
    //out, one at a time, the earliest list.
    while(lol != NULL)
    {
        lolitem = lol;
        earliest = lol->time;
        earlylist = lol;
        while(lolitem != NULL)
        {
                if(earliest > lolitem->time)
                {
                        earliest = lolitem->time;
                        earlylist = lolitem;
                }
                lolitem = lolitem->next;
        }
        // now that we found it, take it out of the list
        if(earlylist == lol)
        {
                lol=lol->next;
        }
        else
        {
                lolitem=lol;
                while(lolitem->next != earlylist)
                {
                        lolitem = lolitem->next;
                }
                lolitem->next = earlylist->next;
        }
        
        vects[i].count = earlylist->count;
        vects[i].vect = new PointVect[vects[i].count];
        vects[i].arrow = new Arrow[vects[i].count];
        vects[i].time = earlylist->time;
        
        for(j=vects[i].count-1; j>=0 ;j--)
        {
                //  vects.vect[i].position[0]=list->vect->position[0];
                //  vects.vect[i].position[1]=list->vect->position[1];
                //  vects.vect[i].position[2]=list->vect->position[2];
                //  vects.vect[i].vector[0]=list->vect->vector[0];
                //  vects.vect[i].vector[1]=list->vect->vector[1];
                //  vects.vect[i].vector[2]=list->vect->vector[2];
                makeArrow(earlylist->list->vect->position,earlylist->list->vect->v,vects[i].arrow[j]);
                listitem = earlylist->list;
                earlylist->list = earlylist->list->next;
                delete listitem->vect;
                delete listitem;
        }
        delete earlylist;
        i++;
    }
    
    return 0; //returns 0 if ok.
}

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
 *	parameter informs the object to render itself more or less quickly.  A
 *	degradation of 0 indicates no degradation (high-res, for instance), while
 *  a degradation of 1 indicates maximum degradation.  The Window3D parameter
 *	can be used to determine information such as pixel-size-in-meters, etc.
 */
void Vector::DrawLowRes(float degradation, Window3D *win)
{
    int i,j, step;
    Arrow * arrow;        
    
    
    doneFlag = 0;
//	const GLfloat *pos;

/*    int i;
    
    glColor3f(1.0,1.0,1.0);
    glBegin(GL_LINES);
    for(i=0; i<vects.count; i++)
    {
        glVertex3d(vects.vect[i].position[0],vects.vect[i].position[1],
                   vects.vect[i].position[2]);
        glVertex3d(vects.vect[i].position[0]+(vects.vect[i].v[0]*scale),
                   vects.vect[i].position[1]+(vects.vect[i].v[1]*scale),
                   vects.vect[i].position[2]+(vects.vect[i].v[2]*scale));
    }
	glEnd();        
    
    */


    step = 1 + (int)(8*degradation);  // results in a triangle when deg = 0.8;
                       
    for(i=0; i<vects[timestep].count; i++)
    {
        arrow = &(vects[timestep].arrow[i]);
        
       	if (degradation < 0.8)
	    {

		    glBegin(GL_TRIANGLE_STRIP);
		    for (j = 0; j < N_SEGS; j+=step)
		    {
			    glColor3f(1.0,0.0,0.0);
			    //glNormal3s(arrow->Normals[j].nx, arrow->Normals[j].ny, arrow->Normals[j].nz);
			    glNormal3sv(arrow->Normals[j]);
			    glVertex3fv(arrow->Base[j]);
			    glColor3f(1.0,1.0,0.0);
			    glVertex3fv(arrow->end);
		    }
		    //glNormal3s(arrow->Normals[0].nx, arrow->Normals[0].ny, arrow->Normals[0].nz);
		    glNormal3sv(arrow->Normals[0]);
		    glColor3f(1.0,0.0,0.0);
		    glVertex3fv(arrow->Base[0]);
		    glColor3f(1.0,1.0,0.0);
		    glVertex3fv(arrow->end);
		    glEnd();
	    }
	    glBegin(GL_LINES);  // at the highest level of degradation we get this.
		    glColor3f(1.0,0.0,0.0);
	    glVertex3fv(arrow->start);
		    glColor3f(1.0,1.0,0.0);
	    glVertex3fv(arrow->end);
	    glEnd();

	}       
    
}

/*---------------------------------------------------------------------------*/
int Vector::DrawHighRes(int eye, Window3D *win)
{
	DrawLowRes(0, NULL);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
	    doneFlag = 1;

	return 1;
}

/*---------------------------------------------------------------------------*/

void Vector::makeArrow(Point3d start, Point3d dir, Arrow & arrow)
{
	int i;
	float x,y,rad,angle;
	float len;
	float dx, dy, dz;
	float ux,uy, uz, vx, vy, vz;
	float nx, ny, nz;

	arrow.Normals = new vNorms[N_SEGS];
	arrow.Base = new Point3f[N_SEGS];

    for(i=0;i<3;++i)
	{
		arrow.start[i] = start[i];
		arrow.end[i] = start[i]+dir[i];
	}
    
	// we need to construct two vectors orthogonal to dir: u and v
	dx = dir[0]; dy = dir[1]; dz = dir[2];
	len = sqrt(dx*dx + dy*dy + dz*dz);
	dx /=len; dy/=len; dz/=len;
	ux = -dy; uy =  dx; uz = 0;

	vx = -dx*dz;
	vy = -dz*dy;
	vz = dx*dx + dy*dy;	

	rad = RADIUS*len;
	
	for ( i = 0; i < N_SEGS; i++) // nStrips
	{
		angle = float(i)/float(N_SEGS);
		x = float(sin(2.0*angle*M_PI));
		y = float(cos(2.0*angle*M_PI));
	
		nx = x*ux+ y*vx;	
	    ny = x*uy + y*vy;
		nz = x*uz + y*vz;

		arrow.Base[i][0] 	= nx*rad +start[0];
		arrow.Base[i][1] = ny*rad + start[1];
		arrow.Base[i][2] = nz*rad + start[2];
/*		arrow.Normals[i].nx = (GLshort)(nx*32767);
		arrow.Normals[i].ny = (GLshort)(ny*32767);
		arrow.Normals[i].nz = (GLshort)(nz*32767);*/
		arrow.Normals[i][X] = (GLshort)(nx*32767);
		arrow.Normals[i][Y] = (GLshort)(ny*32767);
		arrow.Normals[i][Z] = (GLshort)(nz*32767);

	}
	//end[0] = 0.0; end[1] = 0.0; end[2] = 2.0;
}

/*---------------------------------------------------------------------------*/

void Vector::Idle(double time)
{
	Object::Idle(time);
    int i;
    if((time>=bbox.start) && (time <= bbox.end))
    {
	    if( (time < qstart) || (time > qend))
        {
            i=0;
            while(time > vects[i].time)
            {
                i++;
                if(i>=timecount)
                    break;
            }
            if(i>0) i--;
		    timestep = i;
            qstart = vects[i].time;
            if (i<(timecount-1))
            {
                qend = vects[i+1].time;
            }
            else
            {
                qend = qstart;
            }
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
        }
	}
    else
    {
        if(time < bbox.start)
            timestep = 0;
        else
            timestep = timecount - 1;
    }
}
