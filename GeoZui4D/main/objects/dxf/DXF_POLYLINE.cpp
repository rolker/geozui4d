/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include"DXF_POLYLINE.h"
#include"DXF_VERTEX.h"
#include<iostream>
#include<stdlib.h>
#include"utilities/Utilities.h"
#include <gz4d/vector/operations.h>
#include <gz4d/glo/Normal.h>
#include <gz4d/glo/Vertex.h>

using namespace std;

#define DXF_POLYLINE_CLOSED_M 1
#define DXF_POLYLINE_3DPOLYLINE 8
#define DXF_POLYLINE_3DPOLYGON 16
#define DXF_POLYLINE_POLYGON_CLOSED_N 32
#define DXF_POLYLINE_POLYFACE 64 

bool DXF_POLYLINE::warn = false;

DXF_POLYLINE::DXF_POLYLINE()
{
    elevation = 0.0;
    flags = 0;
    m=n=0;
    firstTime = 1;
    displayList = glGenLists(1);
}

// documented in Object
int DXF_POLYLINE::readFrom(FILE *file)
{
	//int i,j;

    readNext();
    while(groupCode!=0)
    {
        if(!processEntry())
        {
            switch(groupCode)
            {
                case 30:
                    elevation=(float)atof(valueString);
                    break;
                case 70:
                    flags = atoi(valueString);
                    break;
                case 71:
                    m = atoi(valueString);
                    break;
                case 72:
                    n = atoi(valueString);
                    break;
                default:
                    break;
            }
        }
        readNext();
		//cerr << "DXF_POLYLINE with flag: " << flags << endl;
        processVerticies();
    }
    if(flags & DXF_POLYLINE_POLYFACE)
        firstFace = m;
	
    if(!vertex.empty())
        bbox.set(gz4d::Point<double>(vertex[0].getPoint()));

    return 0;
}

void DXF_POLYLINE::Draw(float degradation, Window3D *win)
{
    int a1,a2,b1,b2;
    gz4d::Vector<float,3> a,b;
	
    if(flags & DXF_POLYLINE_3DPOLYGON)
    {
        if(firstTime)
        {
            firstTime = 0;
            normals.resize(n*m);
            for(int i=0;i<m;i++)
                for(int j=0;j<n;j++)
            {
                if(j==n-1)
                {
                    a1 = i*n+j-1;
                    a2 = i*n+j;
                }
                else if(j==0)
                {
                    a1 = i*n+j;
                    a2 = i*n+j+1;
                }
                else
                {
                    a1 = i*n+j-1;
                    a2 = i*n+j+1;
                }
                if(i==m-1)
                {
                    b1 = (i-1)*n+j;
                    b2 = (i)*n+j;
                }
                else if (i==0)
                {
                    b1 = (i)*n+j;
                    b2 = (i+1)*n+j;
                }
                else
                {
                    b1 = (i-1)*n+j;
                    b2 = (i+1)*n+j;
                }
					
                a = vertex[a2].getPoint() - vertex[a1].getPoint();
                b = vertex[b2].getPoint() - vertex[b1].getPoint();
                normals[i*n+j] = normalize(cross(b,a));
				
            }

            glNewList(displayList,GL_COMPILE);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

            for(int i=0;i<m-1;i++)
            {
                glBegin(GL_TRIANGLE_STRIP);
                for(int j=0;j<n;j++)
                {
                    gz4d::glo::Normal(normals[i*n+j]);
                    gz4d::glo::Vertex(vertex[i*n+j].getPoint());
                    gz4d::glo::Normal(normals[(i+1)*n+j]);
                    gz4d::glo::Vertex(vertex[(i+1)*n+j].getPoint());
                }
                if(flags&DXF_POLYLINE_POLYGON_CLOSED_N)
                {
                    gz4d::glo::Normal(normals[i*n]);
                    gz4d::glo::Vertex(vertex[i*n].getPoint());
                    gz4d::glo::Normal(normals[(i+1)*n]);
                    gz4d::glo::Vertex(vertex[(i+1)*n].getPoint());
                }
                glEnd();
            }
            if(flags&DXF_POLYLINE_CLOSED_M)
            {
                glBegin(GL_TRIANGLE_STRIP);
                for(int j=0;j<n;j++)
                {
                    gz4d::glo::Normal(normals[(m-1)*n+j]);
                    gz4d::glo::Vertex(vertex[(m-1)*n+j].getPoint());
                    gz4d::glo::Normal(normals[j]); 
                    gz4d::glo::Vertex(vertex[j].getPoint());
                }
                glEnd();
            }
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
            glEndList();
        }
        else
            glCallList(displayList);
    }
    else if (flags & DXF_POLYLINE_POLYFACE)
    {
        if(firstTime)
        {
            firstTime = 0;
            normals.resize(vertex.size()-getFirstFace());
            for(unsigned int i=getFirstFace();i<vertex.size();i++)
            {
                int i1 = vertex[i].getIndex(1);
                if(i1 < 0) i1 = -i1;
                int i2 = vertex[i].getIndex(2);
                if(i2 < 0) i2 = -i2;
                int i3 = vertex[i].getIndex(3);
                if(i3 < 0) i3 = -i3;

                a= vertex[i2-1].getPoint()-vertex[i1-1].getPoint();
                b= vertex[i3-1].getPoint()-vertex[i2-1].getPoint();
                normals[i-getFirstFace()] = normalize(cross(a,b));
            }
		
            glNewList(displayList,GL_COMPILE);
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
            std::cerr << "DXF_POLYLINE::Draw first face: " << getFirstFace() << " total vertecies: " << vertex.size() << std::endl;
            for(unsigned int i=getFirstFace();i<vertex.size();i++)
            {
                glBegin(GL_POLYGON);
                gz4d::glo::Normal(normals[i-getFirstFace()]);
                for(int j=1;j<=4;j++)
                {
                    if(vertex[i].getIndex(j)<0)
                        gz4d::glo::Vertex(vertex[-vertex[i].getIndex(j)-1].getPoint());
                    if(vertex[i].getIndex(j)>0)
                        gz4d::glo::Vertex(vertex[vertex[i].getIndex(j)-1].getPoint());
                }
                glEnd();
            }
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
            glEndList();
        }
        else
            glCallList(displayList);
    }
    else

    {
        if(!warn)
        {
            std::cerr << "Polyline with flag " << flags << " not supported" << std::endl;
            warn = true;
        }
    }
}

