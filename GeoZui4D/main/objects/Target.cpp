// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"Target.h"
#include<gz4d/glo.h>
#include<GL/glut.h>
#include <gz4d/glo/PushAttrib.h>

REGISTER_OBJECT(Target, "");

Target::Target()
{
	size = 1.0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

Target::~Target()
{
}

void Target::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT);
    glColor3fv(color);
    glutSolidSphere(size,15,15);
    glPointSize(2.0);
    glBegin(GL_POINTS);
    glVertex3f(0.0,0.0,0.0);
    glEnd();
}

void Target::SetColor(float r, float g, float b)
{
	color[0] = r;
	color[1] = g;
	color[2] = b;
}

void Target::SetSize(float s)
{
	size = s;
}
