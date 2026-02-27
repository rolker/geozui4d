/***************************************************************************** 
 ***               Interactive Visualization Systems Inc. (IVS)            *** 
 ***                                                                       *** 
 *** Copyright (C) 1996 Interactive Visualization Systems Inc.             *** 
 *****************************************************************************
 * CLASS          : Sv_Matrix4                                               *
 * FILE           : Sv_Matrix4.h                                             * 
 * AUTHOR(S)      : Paton, Mark A.                                           *  
 * DATE CREATED   : January  5, 1993                                         *  
 * LAST MOD. DATE : March   26, 1996                                         * 
 * PURPOSE        :                                                          * 
 *     This class was generated from MPCoder as a template to work from...   * 
 *****************************************************************************/

#ifndef SV_MATRIX4_H
#define SV_MATRIX4_H

// *************************  Required Include Files  *************************

// *** System Includes *** 
#include <iostream>
#include <stdio.h>

#include <tcl.h>
#include <tk.h>
#include<gz4d/glo.h>
#include <GL/glu.h>
                      
// *** User Includes *** 

// *********************  User Visible Constants/Defines  *********************

// ************************  Support Types/Structures  ************************

typedef GLfloat Matrix[4][4];

// ***********************  Forward Class Declarations  ***********************

// ****************************  Class Definition  ****************************


class Sv_Matrix4 
{
    public:
        // **********  Constructors & Destructors  **********
	Sv_Matrix4();       
	Sv_Matrix4(const Sv_Matrix4&);
	~Sv_Matrix4(  );
	
        // **********  Class Member Functions  **********
	void SetIdentity();                  // Set Matrix to identity
	GLdouble Determinate();              // Compute determinate (ONLY FOR GL MATRICES)
	GLdouble covalue(GLint a1, GLint a2, GLint a3, GLint b1, GLint b2, GLint b3);	// Gets Cofactors of the matrix
	void  InvertMat();			     // Invert Matrix();
	void  Transpose();                   // Transpose matrix();
	void  Copy( Sv_Matrix4 * );           // Copy
	Sv_Matrix4 operator+(Sv_Matrix4 &);  // Matrix addition
	Sv_Matrix4 operator-(Sv_Matrix4 &);  // Matrix subtraction
	Sv_Matrix4 operator-();              // Matrix negation
	Sv_Matrix4 operator*(Sv_Matrix4 &);  // Matrix multiplication
	Sv_Matrix4 operator*(GLfloat);         // Matrix mult. by scalar      
        friend std::ostream& operator<<(std::ostream&, Sv_Matrix4&);      
	GLfloat GetValue(GLint i, GLint j)            { return _mat[i][j]; }
	void  SetValue(GLint i, GLint j, GLfloat val) { _mat[i][j] = val;  }
	void  VecMultMat(GLfloat *vec, GLfloat *rv);
	void  Multiply(GLfloat vec[4], GLfloat r[4]);    // Mult pt. by matrix
	void  GetGLMatrix();                  // Get the current GL matrix

	char  *GetMatrixPTR();
	void  SetMatrixFromPTR( char *data );
	
	void  LoadGLMatrix() { glLoadMatrixf( (GLfloat *)_mat ); }
	void  MultGLMatrix() { glMultMatrixf( (GLfloat *)_mat ); }
	void  Interpolate( Sv_Matrix4 &mat, GLfloat alpha, Sv_Matrix4 &result );
	
	// ** Functions for setting GL compatible transformation matrices **
	void  SetTranslate(GLfloat x, GLfloat y, GLfloat z);
	void  SetRotation(GLfloat angle, char axis); //changed to GLfloat by mdp 5/19/2005
	
	void  Write( char *fileName );
	void  Write( FILE *out );
	void  Read( char *fileName );		
	void  Read( FILE *in );	

	Matrix _mat;                      // Public storage for the matrix		
		
    protected:
		
    private:
	
};

// ****************************  Inline Functions  ****************************


#endif
