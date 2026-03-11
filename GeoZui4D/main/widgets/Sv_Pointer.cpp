// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

//
// Implementation of Sv_Pointer class
//
#include <gz4d/gz4d.h>
#include <math.h>
#include "Sv_Pointer.h"

//const GLdouble ONE_DIV_ROOT_THREE = 0.57735027;
//#ifndef M_PI
//	const GLdouble M_PI = 3.141592;
//#endif

Sv_Pointer::Sv_Pointer( GLfloat radius)
{
	
	SetVertices(radius);
}

Sv_Pointer::~Sv_Pointer( void )
{
	;
}


/*----------------------------------------------------------------------------*/
/**	SetSize alters the size of the slider pointer(s).  added by mdp 10/5/2000.
 */
void Sv_Pointer::SetSize(GLfloat theLength)
{
	SetVertices(theLength);
}


void Sv_Pointer::SetPointerType( GLshort theType )	
{
	itsType = theType; 
}

void Sv_Pointer::DrawPointer()
	{
	GLint i;

		glBegin(GL_TRIANGLE_STRIP);
		for(i=0;i<6;++i)
		{
			glNormal3fv(CursorRimNormals[i]);
			glVertex3fv(CursorRim[i]);  // draw the cursor here
		}
		glEnd();

		// draw the top
		glBegin(GL_TRIANGLE_STRIP);
		for(i=0;i<3;++i)
		{
			glNormal3f(0.0,1.0,0.0);
			glVertex3fv(CursorUpperRim[i]);
			glVertex3fv(CursorRim[i*2+1]);  
		}
		glEnd();
		// draw the bottom
		glBegin(GL_TRIANGLE_STRIP);
		for(i=0;i<3;++i)
		{
			glNormal3f(0.0,-1.0,0.0);
			glVertex3fv(CursorLowerRim[i]);
			glVertex3fv(CursorRim[i*2]);  
		}
		glEnd();

		// left hand side
		glBegin(GL_TRIANGLE_STRIP);
		for(i=0;i<3;++i)
		{
			glNormal3f(-0.7f,0.0,0.7f);
			glVertex3fv(CursorLowerRim[0]);
			glVertex3fv(CursorUpperRim[0]);
			glVertex3fv(CursorRim[0]);  
			glVertex3fv(CursorRim[1]); 
		}
		glEnd();

		glBegin(GL_TRIANGLE_STRIP);
		for(i=0;i<3;++i)
		{
			glNormal3f(0.7f,0.0,0.7f);
			glVertex3fv(CursorLowerRim[2]);
			glVertex3fv(CursorUpperRim[2]);
			glVertex3fv(CursorRim[4]);  
			glVertex3fv(CursorRim[5]); 
		}
		glEnd();

        glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0,1.0f,0.0f);
//			glNormal3f(1.0,0.0,0.0);
            glVertex3fv(CursorArrow[0]);
			glNormal3f(0.0,1.0f,0.0f);
            glVertex3fv(CursorArrow[1]);
			glNormal3f(0.0,-1.0f,0.0f);
            glVertex3fv(CursorArrow[2]);
			glNormal3f(0.0,-1.0f,0.0f);
            glVertex3fv(CursorArrow[3]);
			glNormal3f(0.0,1.0f,0.0f);
            glVertex3fv(CursorArrow[4]);
			glNormal3f(0.0,1.0f,0.0f);
            glVertex3fv(CursorArrow[1]);
        glEnd();

        glPushMatrix();
        glScalef(-1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0.0,1.0f,0.0f);
//			glNormal3f(1.0,0.0,0.0);
            glVertex3fv(CursorArrow[0]);
			glNormal3f(0.0,1.0f,0.0f);
            glVertex3fv(CursorArrow[1]);
			glNormal3f(0.0,-1.0f,0.0f);
            glVertex3fv(CursorArrow[2]);
			glNormal3f(0.0,-1.0f,0.0f);
            glVertex3fv(CursorArrow[3]);
			glNormal3f(0.0,1.0f,0.0f);
            glVertex3fv(CursorArrow[4]);
			glNormal3f(0.0,1.0f,0.0f);
            glVertex3fv(CursorArrow[1]);
        glEnd();
        glPopMatrix();
	}

void Sv_Pointer::SetVertex( GLfloat *theVertexArray, GLfloat theX, GLfloat theY, GLfloat theZ )
	{
	theVertexArray[0] = theX;
	theVertexArray[1] = theY;
	theVertexArray[2] = theZ;
	}


void Sv_Pointer::SetVertices(float radius )
	{
	GLint i;
	float outerRadius, midRadius;

	// we should paramterize the width

	outerRadius = radius*1.12f;
    midRadius = ((outerRadius + radius) / 2.0) * 1.05f;

	for(i=0;i<3;++i)
	{		
		CursorRim[i*2][0] = outerRadius*(GLfloat)sin(((i-1.0)/20.0)*M_PI);
		CursorRim[i*2][1] = -20.0; // this is the width
		CursorRim[i*2][2] = outerRadius*(GLfloat)cos(((i-1.0)/20.0)*M_PI);

		CursorRim[i*2+1][0] = outerRadius*(GLfloat)sin(((i-1.0)/20.0)*M_PI);
		CursorRim[i*2+1][1] = 0.0;
		CursorRim[i*2+1][2] = outerRadius*(GLfloat)cos(((i-1.0)/20.0)*M_PI);
		
		CursorRimNormals[i*2][0] = (GLfloat)sin(((i-1.0)/20.0)*M_PI);
		CursorRimNormals[i*2][1] = -0.3f;
		CursorRimNormals[i*2][2] = (GLfloat)cos(((i-1.0)/20.0)*M_PI);

		CursorRimNormals[i*2+1][0] = outerRadius*(GLfloat)sin(((i-1.0)/20.0)*M_PI);
		CursorRimNormals[i*2+1][1] = 0.3f;
		CursorRimNormals[i*2+1][2] = outerRadius*(GLfloat)cos(((i-1.0)/20.0)*M_PI);


		CursorUpperRim[i][0] = radius*(GLfloat)sin(((i-1.0)/10.0)*M_PI);
		CursorUpperRim[i][1] = 0.0;
		CursorUpperRim[i][2] = radius*(GLfloat)cos(((i-1.0)/10.0)*M_PI);

		CursorLowerRim[i][0] = radius*(GLfloat)sin(((i-1.0)/10.0)*M_PI);
		CursorLowerRim[i][1] = -20.0;
		CursorLowerRim[i][2] = radius*(GLfloat)cos(((i-1.0)/20.0)*M_PI);
	}

    CursorArrow[0][0] = midRadius *(GLfloat)sin((0.18)*M_PI);
    CursorArrow[0][1] = -10.0;
    CursorArrow[0][2] = midRadius *(GLfloat)cos((0.18)*M_PI);

    CursorArrow[1][0] = radius *(GLfloat)sin((0.12)*M_PI);
    CursorArrow[1][1] = 0.0;
    CursorArrow[1][2] = radius *(GLfloat)cos((0.12)*M_PI);

    CursorArrow[2][0] = radius *(GLfloat)sin((0.12)*M_PI);
    CursorArrow[2][1] = -20.0;
    CursorArrow[2][2] = radius *(GLfloat)cos((0.12)*M_PI);

    CursorArrow[3][0] = outerRadius *(GLfloat)sin((0.11)*M_PI);
    CursorArrow[3][1] = -20.0;
    CursorArrow[3][2] = outerRadius *(GLfloat)cos((0.11)*M_PI);

    CursorArrow[4][0] = outerRadius *(GLfloat)sin((0.11)*M_PI);
    CursorArrow[4][1] = 0.0;
    CursorArrow[4][2] = outerRadius *(GLfloat)cos((0.11)*M_PI);

}
	

