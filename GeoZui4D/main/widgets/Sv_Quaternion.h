/***************************************************************************** 
 ***               Interactive Visualization Systems Inc. (IVS)            *** 
 ***                                                                       *** 
 *** Copyright (C) 1996 Interactive Visualization Systems Inc.             *** 
 *****************************************************************************
 * CLASS          : Sv_Quaternion                                            *
 * FILE           : Sv_Quaternion.h                                          * 
 * AUTHOR(S)      : Paton, Mark A.                                           *  
 * DATE CREATED   : January  5, 1993                                         *  
 * LAST MOD. DATE : March   26, 1996                                         * 
 * PURPOSE        :                                                          * 
 *      This class implements the necessary data structure and function to   * 
 *   provide support for quaternion computations which are can be used       *
 *   as a method to specify rotations in 3D space.                           *
 * CHANGE LOG     :                                                          *
 * 	May 4, 1993 - David Hui                                              *
 * The member functions, Invert(), SetQuaternion() and overloaded operators, *
 * "*" and "=" are made inline and moved into this file for efficiency.      *
 * Also, definitions of the two overload operators are modified to use const *
 * arguments (to eliminate warning messages at compile time).                *
 * 	July 12, 1993 - Mark Paton                                           *
 * Added the function to interpolate between quaternions.                    *
 *****************************************************************************/

#ifndef SV_QUATERNION_H
#define SV_QUATERNION_H

// *************************  Required Include Files  *************************

// *** System Includes *** 
#include <math.h>
                      
// *** User Includes *** 
#include "widgets/Sv_Standards.h"
#include "widgets/Sv_Matrix4.h"

// ****************************  Class Definition  ****************************

class Sv_Quaternion 
{
    public:
        // **********  Constructors & Destructors  **********
	Sv_Quaternion(void);
	Sv_Quaternion(GLfloat w, GLfloat x, GLfloat y , GLfloat z );
	Sv_Quaternion(Sv_Quaternion &);
        ~Sv_Quaternion(  );
	
        // **********  Class Member Functions  **********
	void SetQuaternion(GLfloat w, GLfloat x, GLfloat y, GLfloat z);  
	void SetQuaternion(GLfloat vals[4]);
	void GetQuaternion(GLfloat vals[4]); 
	Sv_Quaternion operator*(const Sv_Quaternion &);   // Multiplication
	Sv_Quaternion operator=(const Sv_Quaternion &);   // Assignment
	void Invert();                                    // Invert Quaternion
	void Normalize(void);                                 // Normalize Quaternion
	Sv_Matrix4 ToRotationMatrix();                    // Quat --> Rot. matrix
        void FromRotationMatrix( Sv_Matrix4 *mat );       // Quat from a matrix
	void LoadGLQuaternion();
	void MultGLQuaternion();
	void MultGLTQuaternion();
	void EulerToQuaternion( GLfloat euler[3] );
	void EulerToQuaternion( GLfloat angleX, GLfloat angleY, GLfloat angleZ );
	void SphericalInterp( Sv_Quaternion &quat, GLdouble alpha, Sv_Quaternion &result );
        friend std::ostream& operator<<(std::ostream&, Sv_Quaternion&);

	// The quaternion values are directly accessible
	GLfloat _w;             // Twist about axis (real part)
	GLfloat _x, _y, _z;     // Axis (imaginary part of quaternion)		
		
    protected:
		
		
    private:
		
	
};

// ****************************  Inline Functions  ****************************

inline void Sv_Quaternion::SetQuaternion( GLfloat w, GLfloat x, GLfloat y, GLfloat z )
{
    _w = w;
    _x = x;
    _y = y;
    _z = z;
}

inline void Sv_Quaternion::SetQuaternion( GLfloat vals[4] )
{
    _w = vals[0];
    _x = vals[1];
    _y = vals[2];
    _z = vals[3];
}

inline void Sv_Quaternion::GetQuaternion( GLfloat vals[4] )
{
    vals[0] = _w;    
    vals[1] = _x;    
    vals[2] = _y;    
    vals[3] = _z;    
}

inline Sv_Quaternion Sv_Quaternion::operator*( const Sv_Quaternion &q )
{
    Sv_Quaternion rq;
    
    rq._w = (_w * q._w) - (_x * q._x) - (_y * q._y) - (_z * q._z);
    rq._x = (_x * q._w) + (_w * q._x) - (_z * q._y) + (_y * q._z);
    rq._y = (_y * q._w) + (_z * q._x) + (_w * q._y) - (_x * q._z);
    rq._z = (_z * q._w) - (_y * q._x) + (_x * q._y) + (_w * q._z);  
    
    return rq;
}

inline Sv_Quaternion Sv_Quaternion::operator=(const Sv_Quaternion &quaternion)
{
    _w = quaternion._w;
    _x = quaternion._x;
    _y = quaternion._y;
    _z = quaternion._z;
    return *this;
}

inline void Sv_Quaternion::Invert( void )
{
    _x = -_x;       // _w is unchanged
    _y = -_y;
    _z = -_z;
}

inline void Sv_Quaternion::EulerToQuaternion( GLfloat eX, GLfloat eY, GLfloat eZ )
{
    GLfloat cosroll, cospitch, cosyaw, sinroll, sinpitch, sinyaw;
    
    cosroll = (GLfloat)cos(DEGTORAD*eZ/2.0); 
    cospitch = (GLfloat)cos(DEGTORAD*eX/2.0); 
    cosyaw = (GLfloat)cos(DEGTORAD*eY/2.0);
    sinroll = (GLfloat)sin(DEGTORAD*eZ/2.0); 
    sinpitch = (GLfloat)sin(DEGTORAD*eX/2.0); 
    sinyaw = (GLfloat)sin(DEGTORAD*eY/2.0);
    
    _z = (sinroll * cospitch * cosyaw) - (cosroll * sinpitch * sinyaw);
    _x = (cosroll * sinpitch * cosyaw) + (sinroll * cospitch * sinyaw);
    _y = (cosroll * cospitch * sinyaw) - (sinroll * sinpitch * cosyaw);
    _w = (cosroll * cospitch * cosyaw) + (sinroll * sinpitch * sinyaw);
}

inline void Sv_Quaternion::EulerToQuaternion( GLfloat euler[3] )
{
    EulerToQuaternion( euler[X], euler[Y], euler[Z]);
}


#endif
