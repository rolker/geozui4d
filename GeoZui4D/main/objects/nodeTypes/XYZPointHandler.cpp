/* Copyright (c) 2000 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "objects/nodeTypes/XYZPointHandler.h"
#include <stdlib.h>
#include <typeinfo>
#include <iostream>
#include <gz4d/util/Utilities.h>

using namespace std;

XYZPointHandler::XYZPointHandler()
{
}

void XYZPointHandler::Add(SDV_Data &s)
{
/*    SoundingNode * temp;
    temp = new SoundingNode;*/

//     temp->sounding[0] = ((XYZPoint*)s)->sounding[0];
//     temp->sounding[1] = ((XYZPoint*)s)->sounding[1];
//     temp->sounding[2] = ((XYZPoint*)s)->sounding[2];
    std::list<float> l(1,s.sounding[0]);
    pointList.merge(l);
    std::list<float>::iterator c = pointList.begin();
    for (unsigned int i = 1; i < pointList.size()/2;i++, c++);
    value = *c;
    if(!gz4d::IsNan(s.alternateData))
    {
        std::list<float> al(1,s.alternateData);
        altList.merge(al);
        c = altList.begin();
        for (unsigned int i = 1; i < altList.size()/2;i++, c++);
        alternateData = *c;
    }
//	AddNode(temp);
}

// void XYZPointHandler::AddNode(SoundingNode * s)
// {
// 	int i;
// 	SoundingNode * cursor;
// 
//  	cursor = list;
// 	if (list != NULL)
// 	{
// 		while((cursor->next != NULL) && (cursor->sounding[2] > s->sounding[2]))
// 		{
// 			cursor = cursor->next;
// 			if (cursor == NULL)
// 			{
// 				cerr << "Cursor is NULL!\n";
// 			}
// 		}
// 		s->next = cursor->next;
// 		cursor->next = s;
// 	}
// 	else
// 	{
// 		list = s;
// 		list->next = NULL;
// 	}
//     
//     count++;
// 	cursor = list;
// 	for (i = 0; i < (count/2); i++)
// 	{
// 		cursor = cursor->next;
// 	}
// 	value = cursor->sounding[2];
// 
// }

// double XYZPointHandler::GetAverage()
// {
//     return value;
// }

// SoundingNode * XYZPointHandler::Pop()
// {
//     SoundingNode * ret;
//     
//     if (list == NULL)
//     {
//         return NULL;
//     }
//     else
//     {
//         ret = list;
//         list = list->next;
//         return ret;
//     }
//     return NULL;
// }

void XYZPointHandler::Draw()
{
/*    SoundingNode * s;
    
    s = list;
    glBegin(GL_POINTS);
    while(s != NULL)
    {
        glVertex3f(s->sounding[0],s->sounding[1],s->sounding[2]);
        s = s->next;
    }
    glEnd();*/
}

void XYZPointHandler::Draw(double minx, double maxx, double miny, double maxy)
{
/*    SoundingNode * s;
    
    s = list;
    glColor3f(1.0,0.0,0.0);
    glBegin(GL_LINES);
    while(s != NULL)
    {
        glVertex3f(s->sounding[0],s->sounding[1],s->sounding[2]);
        glVertex3f((minx+maxx)/2.0,(miny+maxy)/2.0,0.0);
        s = s->next;
    }
    glEnd();*/
}

void XYZPointHandler::writeTo(FILE * outfile)
{
/*    SoundingNode * s;
    
    s = list;
    while(s != NULL)
    {
		fprintf(outfile, "%f %f %f\n", s->sounding[0],s->sounding[1],s->sounding[2]);
        s = s->next;
    }
    glEnd();*/
}
