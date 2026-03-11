// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "SoundingBuffer.h"

SoundingBuffer::SoundingBuffer(int l)
{
	length = l;
	buffer = new Point3f[length];
	head = 0;
	filled = 0;
}

void SoundingBuffer::Draw(float deg)
{
	int i,l;
//	float intensity;

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
	glBegin(GL_LINES);
//	for (i=0; i < l; i+= 1+(deg*50))
	for (i=0; i < l; i+= 1)
	{
/*		intensity = (float)i-(float)head;
		if(intensity < 0.0)
		{
			intensity+=(float)l;
		}
		intensity/=(float)l;

		glColor4f(1.0,0.5,0.5,intensity);
*/
		glVertex3f(buffer[i][0],buffer[i][1],buffer[i][2]);
		glVertex3f(buffer[i][0],buffer[i][1],(float)(buffer[i][2]+5.0));
	}
	glEnd();
//	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

void SoundingBuffer::Add(Point3f s)
{
	head++;
	if (head >= length)
	{
		head = 0;
		filled = 1;
	}
	buffer[head][0] = s[0];
	buffer[head][1] = s[1];
	buffer[head][2] = s[2];
}


