// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef DISC_H
#define DISC_H
class disc
{
public:
	disc(int n,float dia);
	void initTex();

	void draw();
private:
	float *Xpts;
	float *Ypts;
	int nPts;
	float diam;

	GLubyte waves[32][32][4];
	GLuint waveTex; //the texture handle
	float mov;
	float step;
};

#endif
