// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0


#include <math.h>
#include<gz4d/glo.h>
#include <GL/glut.h>
#include "utilities/disc.h"

// a set of concentric color bands - blends with background to show accoustics.


disc::disc(int n,float dia)
{
	int i,j,c,r,g,b,t;
	float angle,rad;
	diam = dia;
	nPts = n+1;
	Xpts = new float[nPts];
	Ypts = new float[nPts];
	for(i=0;i<nPts;++i)
	{
		angle = 2.0*3.14159*float(i)/n;
		Xpts[i] = sin(angle);
		Ypts[i] = cos(angle);
	}


	// make a sine texture for sound waves

	for (i=0;i<32;++i) //radial color and transparncy
		for(j=0;j<32;++j)  // sine pattern
		{
			c = (int)(128.0*(sin(j*3.14159/31.0)+1.0));
			rad = i/31.0;

			r = 0;
			g =0;
			b = 80;

			if(rad < 0.707)
			{
				r = 0;
				g = 128;
				b = 200;
			}
			if(rad < 0.5)
			{
				r = 0;
				g = 255;
				b = 255;
			}
			if(rad < 0.3535)
			{
				r = 64;
				g = 255;
				b = 128;
			}
			if(rad < 0.177)
			{
				r = 200;
				g = 200;
				b = 64;
			}
			if(rad < 0.09)
			{
				r = 255;
				g = 64;
				b = 0;
			}




			// transparency from 0.5 in center down
			t = (int)((1.0-rad*.88)*60.0*(sin(j*3.14159/31.0)+1.0));
		
			waves[i][j][0] = GLubyte(r);
			waves[i][j][1] = GLubyte(g);
			waves[i][j][2] = GLubyte(b);
			waves[i][j][3] = GLubyte(t);
		}

		mov = 0.0;

		initTex();


}

void disc::initTex()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &waveTex);
	glBindTexture(GL_TEXTURE_2D,waveTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,waves);

	step = 0.1f;
}





void disc::draw()
{
	int i;
	float d,d2;

	float r2,alpha;

	alpha = 0.4f;

	mov = mov+0.10001;
	if(mov > 1.0) mov = 0.0;


	r2 = 1.0/sqrt(2.0);

	d = diam;
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_2D,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D,waveTex);


	d = diam;
	d2 = d*r2;

	glColor4f(1.0,1.0,1.0,0.5);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(1.0-mov,0.0);
	glVertex3f(0.0,0.0,0.0);
	//glColor3f(0.0,0.0,0.0);
	for(i=0;i<nPts;++i)
	{
		glTexCoord2f(40.0-mov,1.0);
		glVertex3f(Xpts[i]*d,Ypts[i]*d,0.0);
	}
	glEnd();

	glEnable(GL_LIGHTING);
//	glDisable(GL_BLEND); //commented out by mdp 8/31/2004 (we always blend)
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

