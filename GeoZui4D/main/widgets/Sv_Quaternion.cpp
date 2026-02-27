/***************************************************************************** 
 ***               Interactive Visualization Systems Inc. (IVS)            *** 
 ***                                                                       *** 
 *** Copyright (C) 1996 Interactive Visualization Systems Inc.             *** 
 *****************************************************************************
 * CLASS          : Sv_Quaternion                                            * 
 * AUTHOR(S)      : Paton, Mark A.                                           * 
 * FILE           : Sv_Quaternion.c++                                        *
 *****************************************************************************/

// **********************  Internal Class Include Files  **********************

#include <math.h>
#include "Sv_Quaternion.h"

// ***********  Explicit Initialization of Static Member Variables  ***********

// ***********************  Constructors & Destructors  ***********************

////////////////////////////////////////////////////////////////////////////////
// Function: Sv_Quaternion
//    - Class constructor
//
Sv_Quaternion::Sv_Quaternion(  ) 
{  
    _w = 1.0;       // Set a default quaternion that is equivalent to
    _x = 0.0;       // the identity rotation matrix.
    _y = 0.0;
    _z = 0.0;
} 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function: Sv_Quaternion
//    - Class constructor
//
Sv_Quaternion::Sv_Quaternion(GLfloat w, GLfloat x, GLfloat y, GLfloat z )
{  
    _w = w;          // Assign the quaternion values
    _x = x;
    _y = y;
    _z = z;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function: Sv_Quaternion
//    - Class constructor
//
Sv_Quaternion::Sv_Quaternion(Sv_Quaternion & quat)
{
    // Initialize quaternion with the info in quat.
    _w = quat._w;
    _x = quat._x;
    _y = quat._y;
    _z = quat._z;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function: ~Sv_Quaternion
//    - Default destructor.
//
Sv_Quaternion::~Sv_Quaternion(  ) 
{  
    ; // To be implemented    
} 
////////////////////////////////////////////////////////////////////////////////

// ***************************   Member Functions  ****************************

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Quaternion::Normalize(void)
{
    GLfloat factor;
    
    factor = (float)sqrt( _w * _w + _x * _x + _y * _y + _z * _z );
    if( factor != 0.0 )
    {
	_w = _w / factor;
	_x = _x / factor;
	_y = _y / factor;
	_z = _z / factor;
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
Sv_Matrix4 Sv_Quaternion::ToRotationMatrix()
{
    Sv_Matrix4 rmat;
    GLfloat dx, dy, dz;
    GLfloat dwx, dwy, dwz, dxx, dxy, dxz, dyy, dyz, dzz;
    
    dx  = _x + _x;
    dy  = _y + _y;
    dz  = _z + _z;
    dwx = _w * dx;
    dwy = _w * dy;
    dwz = _w * dz;
    dxx = _x * dx;
    dxy = _x * dy;
    dxz = _x * dz;
    dyy = _y * dy;
    dyz = _y * dz;
    dzz = _z * dz;
    
    rmat.SetValue(0,0,1 - dyy -dzz);
    rmat.SetValue(0,1,dxy + dwz);
    rmat.SetValue(0,2,dxz - dwy);
    rmat.SetValue(0,3,0.0);
    rmat.SetValue(1,0,dxy - dwz);
    rmat.SetValue(1,1,1 - dxx - dzz );
    rmat.SetValue(1,2,dyz + dwx);
    rmat.SetValue(1,3,0.0);
    rmat.SetValue(2,0, dxz + dwy );
    rmat.SetValue(2,1, dyz - dwx );
    rmat.SetValue(2,2, 1 - dxx -dyy );
    rmat.SetValue(2,3,0.0);
    rmat.SetValue(3,0, 0.0);
    rmat.SetValue(3,1, 0.0);
    rmat.SetValue(3,2, 0.0);
    rmat.SetValue(3,3,1.0);
    
    return rmat;     
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Quaternion::FromRotationMatrix( Sv_Matrix4 *mat )
{
    GLfloat w, x, y, z;
    GLfloat w2, x2, y2;

    w2 = (float)(0.25 * ( 1 + mat->GetValue(0, 0) + 
                 mat->GetValue(1,1) + mat->GetValue(2,2) ));
    if( fabs(w2) > 0.000000001 )
    {
        w = (float)sqrt( w2 );
        x = ( mat->GetValue(1,2) - mat->GetValue(2,1) ) / ( 4 * w );
        y = ( mat->GetValue(2,0) - mat->GetValue(0,2) ) / ( 4 * w );
        z = ( mat->GetValue(0,1) - mat->GetValue(1,0) ) / ( 4 * w );
    }
    else
    {
        w = 0.0;
        x2 = (float)(-0.5 * ( mat->GetValue(1,1) + mat->GetValue(2,2) ));
        if( fabs( x2 ) > 0.000000001 )
        {
            x = (float)sqrt( x2 );
            y = (float)(mat->GetValue(0,1) / ( 2.0 * x ));
            z = (float)(mat->GetValue(0,2) / ( 2.0 * x ));
        }
        else
        {
            x = 0.0;
            y2 = (float)(0.5 * ( 1.0 - mat->GetValue(2,2) ));
            if( y2 > 0.000000001 )
            {
                y = (float)sqrt( y2 );
                z = (float)(mat->GetValue(1,2) / ( 2.0 * y ));
            }
            else
            {
                y = 0.0;
                z = 1.0;
            }
        }
    }
    _w = w;
    _x = x;
    _y = y;
    _z = z;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
//void Sv_Quaternion::LoadGLQuaternion()
//{
 //   Sv_Matrix4 rotMatrix;
//    
//    rotMatrix = this->ToRotationMatrix();
//    rotMatrix.LoadGLMatrix();
//}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
//void Sv_Quaternion::MultGLQuaternion()
//{
//   Sv_Matrix4 rotMatrix;
//    
//   rotMatrix = this->ToRotationMatrix();
//    rotMatrix.MultGLMatrix();
//}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
//void Sv_Quaternion::MultGLTQuaternion()
//{
 //   Sv_Matrix4 rotMatrix;
//    
//    rotMatrix = this->ToRotationMatrix();
//    rotMatrix.Transpose();
//    rotMatrix.MultGLMatrix();
//}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Quaternion::SphericalInterp( Sv_Quaternion &quat, 
                                     GLdouble alpha, Sv_Quaternion &result )
{
    GLdouble sum;
    GLdouble beta1, beta2, theta;
    
    sum = _w*quat._w + _x*quat._x + _y*quat._y + _z*quat._z;
    if( sum < 1.0 )
        theta = acos(sum);
    else
	theta = 0;
    if( theta <= 1E-10 )    // Less then epsilion
    {
	beta1 = 1.0 - alpha;
	beta2 = alpha;
    }
    else
    {
	beta1 = sin( (1.0 - alpha)*theta) / sin(theta);
	beta2 = sin( alpha*theta ) / sin(theta);
    }
    result._w = (float)(beta1*_w  + beta2*quat._w);    
    result._x = (float)(beta1*_x  + beta2*quat._x);    
    result._y = (float)(beta1*_y  + beta2*quat._y);    
    result._z = (float)(beta1*_z  + beta2*quat._z);    
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : 
// Arguments : 
// Returns   : 
// Purpose   : 
//
std::ostream &operator<<(std::ostream& s, Sv_Quaternion& quat)
{
    s << "{"  << quat._w 
      << ", " << quat._x 
      << ", " << quat._y
      << ", " << quat._z << "}";
    return s;
}
////////////////////////////////////////////////////////////////////////////////
