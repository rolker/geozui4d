// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include<gz4d/glo.h>
#include <GL/glut.h>
#include "SwathBuffer.h"

using namespace std;

SwathBuffer::SwathBuffer(gz4d::Point<double> const &objoffset, int l)
{
    length = l;
    buffer = new SwathType[length];
    head = 0;
    filled = 0;
    offset = objoffset;
    cerr << "Creating SwathBuffer of length " << l << "\n";
}

void SwathBuffer::Draw(float deg)
{
//    mutex.lock();
//    cerr << "SwathBuffer::Draw Enter\n";
	int i,l,j;
//	float intensity;

//    cerr << "SwathBuffer: draw\n";
    
    if(head == 0 && filled == 0)
    {
        return;
    }
    
	if(filled == 1)
	{
		l = length;
	}
	else
	{
		l = head+1;
	}
	glColor3f(1.0,0.0,0.5);
//	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	for (i=0; i < l; i+= 1+(deg*50))
	for (i=0; i < l; i+= 1)
	{
//        cerr << "Drawing element: " << i << "\n";
       	glBegin(GL_POINTS);
        for (j=0;j<buffer[i].soundingCount; j++)
        {
			glVertex3f(buffer[i].soundings[j][0]+offset[0],buffer[i].soundings[j][1]+offset[1],buffer[i].soundings[j][2]+offset[2]);
	    }
        glEnd();
	}
	glBegin(GL_LINE_STRIP);
	glVertex3f(buffer[head].soundings[0][0]+offset[0],buffer[head].soundings[0][1]+offset[1],buffer[head].soundings[0][2]+offset[2]);
	glVertex3f(buffer[head].east+offset[0],buffer[head].north+offset[1],buffer[head].heave+offset[2]);
	glVertex3f(buffer[head].soundings[buffer[head].soundingCount-1][0]+offset[0],buffer[head].soundings[buffer[head].soundingCount-1][1]+offset[1],buffer[head].soundings[buffer[head].soundingCount-1][2]+offset[2]);
	glEnd();

//	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
//    mutex.unlock();
//    cerr << "SwathBuffer::Draw Exit\n";
}

void SwathBuffer::Add(SwathType s)
{
//    cerr << "SwathBuffer::Add Enter\n";
//    mutex.lock();
    int i;
    
	head++;
	if (head >= length)
	{
		head = 0;
		filled = 1;
	}

//    cerr << "SwathBuffer: swath added at position:" << head << "\n";

    buffer[head].soundings.resize(s.soundingCount);

    for(i = 0; i<s.soundingCount; i++)
    {
        buffer[head].soundings[i][0] = s.soundings[i][0];
        buffer[head].soundings[i][1] = s.soundings[i][1];
        buffer[head].soundings[i][2] = s.soundings[i][2];
    }
	buffer[head].soundingCount = s.soundingCount;
    buffer[head].east = s.east;
    buffer[head].north = s.north;
    buffer[head].roll = s.roll;
    buffer[head].pitch = s.pitch;
    buffer[head].heading = s.heading;
    buffer[head].heave = s.heave;
//    mutex.unlock();
//    cerr << "SwathBuffer::Add Exit\n";
}


