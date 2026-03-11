// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0



#ifndef SV_POINTER_H
#define SV_POINTER_H

#include <tcl.h>
#include <tk.h>
#include<gz4d/glo.h>

/*#ifdef WIN32
#include "Togl/togl.h" //needed to help set up for glu.h
#endif*/
#include <GL/glu.h>

//Pointer types
const GLshort  SV_POINTER_IS_TRIANGLE = 0,	// isosceles triangle
					SV_POINTER_EQ_TRIANGLE = 1,	// equilateral triangle
					SV_POINTER_ARROW = 2;		// regular arrow

//Pointer styles
const GLshort	SV_POINTER_SOLID = 0, 	    // filled pointer
					SV_POINTER_OUTLINE = 1;	    // outline only
	
/** This class defines an object, in an arrow-shaped form, which may be 
 *  used to point to things in a scene. It's primary purpose is	     
 *  to provide graphical primitives for Sv_Slider3.		             
 */
//class declaration
class Sv_Pointer 
	{
   public:
		//Ctor and dtor
		//Create pointer with specified length (from tip to end,
		// the width is calculated automatically from this), type and style (as defined above)
		Sv_Pointer( GLfloat theLength );
		~Sv_Pointer( void );

		//Member functions
		//Specify a new length, type and style of the pointer
		void DrawPointer( void );

		void SetPointerType( GLshort theType );
		void SetSize(GLfloat theLength); //added by mdp 10/5/2000
		
   protected:
		void SetVertex( GLfloat* theVertexArray, GLfloat theX, GLfloat theY, GLfloat theZ );
		void SetVertices( float radius );
		GLfloat itsLength;
		GLshort itsType;
		GLshort itsStyle;

		GLfloat CursorRim[6][3];
		GLfloat CursorRimNormals[6][3];

		GLfloat CursorUpperRim[3][3];
		GLfloat CursorLowerRim[3][3];

        GLfloat CursorArrow[5][3];

		GLshort itsGLDrawingShape;//GL_POLYGON, GL_TRIANGLES, etc.
		GLint itsNumVertices;//The number of points for the pointer (different pointers has different number
		            // of vertices
	};

#endif

