/***************************************************************************** 
 ***               Interactive Visualization Systems Inc. (IVS)            *** 
 ***                                                                       *** 
 *** Copyright (C) 1996 Interactive Visualization Systems Inc.             *** 
 *****************************************************************************
 * CLASS          : Sv_Matrix4                                               * 
 * AUTHOR(S)      : Paton, Mark A.                                           * 
 * FILE           : Sv_Matrix4.c++                                           *
 *****************************************************************************/

// **********************  Internal Class Include Files  **********************

#include <math.h>
#include "widgets/Sv_Matrix4.h"

// ***********  Explicit Initialization of Static Member Variables  ***********
  

// ***********************  Constructors & Destructors  ***********************

////////////////////////////////////////////////////////////////////////////////
// Function: Sv_Matrix4
//    - Class constructor
//
Sv_Matrix4::Sv_Matrix4()
{
   for(GLint i=0; i<4; i++)
      for(GLint j=0; j<4; j++)
         _mat[i][j] = 0.0;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function: Sv_Matrix4
//    - Class constructor
//
Sv_Matrix4::Sv_Matrix4(const Sv_Matrix4 &matrix)
{
   for(GLint i=0; i<4; i++)
      for(GLint j=0; j<4; j++)
         _mat[i][j] = matrix._mat[i][j];
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function: ~Sv_Matrix4
//    - Default destructor.
//
Sv_Matrix4::~Sv_Matrix4(  ) 
{  
    ; // To be implemented    
} 
////////////////////////////////////////////////////////////////////////////////

// ***************************   Member Functions  ****************************

////////////////////////////////////////////////////////////////////////////////
// Function  : SetIdentity
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::SetIdentity()
{
   for(GLint i=0; i<4; i++)
      for(GLint j=0; j<4; j++)
         if( i == j )
            _mat[i][j] = 1.0;
         else
            _mat[i][j] = 0.0;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : GetGLMatrix
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::GetGLMatrix()
{
   GLint mm;
   GLfloat glmat[16];

   glGetIntegerv(GL_MATRIX_MODE, &mm);
   if (mm == GL_MODELVIEW)
	glGetFloatv(GL_MODELVIEW_MATRIX, glmat);
   else
	glGetFloatv(GL_PROJECTION_MATRIX, glmat);
   for(GLint i=0; i<4; i++)
      for(GLint j=0; j<4; j++)
         _mat[i][j] = glmat[i*4+j];
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : covalue
// Arguments : 
// Returns   : 
// Purpose   : 
//
GLdouble Sv_Matrix4::covalue(GLint a1, GLint a2, GLint a3, GLint b1, GLint b2, GLint b3)
{
    GLdouble cv;
    
    cv=_mat[a1][b1]*_mat[a2][b2]*_mat[a3][b3];
    cv+=_mat[a2][b1]*_mat[a3][b2]*_mat[a1][b3];
    cv+=_mat[a3][b1]*_mat[a1][b2]*_mat[a2][b3];
    cv-=_mat[a1][b3]*_mat[a2][b2]*_mat[a3][b1];
    cv-=_mat[a2][b3]*_mat[a3][b2]*_mat[a1][b1];
    cv-=_mat[a3][b3]*_mat[a1][b2]*_mat[a2][b1];
        
    return(cv);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : InvertMat
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::InvertMat()
{   
    Sv_Matrix4 *inv;
    GLdouble sign;
    GLdouble d;
    GLint i, j, k, l, m, a[3], b[3];
    
    inv = new Sv_Matrix4(*this);
    
    d=inv->Determinate();
    sign=1.;
    for(i=0;i<4;i++)
    {
	sign*=-1.;
	for(j=0;j<4;j++)
	{
	    sign*=-1.;
	    l=0;
	    m=0;
	    for(k=0;k<4;k++)
	    {
		if (k!=i)
		{
		    a[l]=k;
		    l++;
		}
		if (k!=j)
		{
		    b[m]=k;
		    m++;
		}
	    }
	    _mat[j][i]=(GLfloat) (sign*inv->covalue(a[0], a[1], a[2], b[0], b[1], b[2])/d);
	}
    }
    delete inv;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Determinate
// Arguments : 
// Returns   : 
// Purpose   : 
//
GLdouble Sv_Matrix4::Determinate()
{
    GLint i, j, k;
    GLdouble sign;
    GLdouble result;

    result=0.;
    sign=1.;
    for(i=0; i<4; i++)
    {
//	sign*=-1.;
	for(j=0; j<4; j++)
	{
	    if( i != j )
	    {
		sign*=-1.;
		for (k=0; k<4; k++)
		{
		    if( (k != i ) && ( k != j ) )
		    {
			sign*=-1.;
			result+=sign*(_mat[i][0]*_mat[j][1]*_mat[k][2]*_mat[6-i-j-k][3]);
		    }
		}
	    }
	}
    }
    return result;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Transpose
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Transpose()
{
    GLfloat	temp;

    temp = _mat[0][1];
    _mat[0][1] = _mat[1][0];
    _mat[1][0] = temp;
    temp = _mat[0][2];
    _mat[0][2] = _mat[2][0];
    _mat[2][0] = temp;
    temp = _mat[0][3];
    _mat[0][3] = _mat[3][0];
    _mat[3][0] = temp;
    temp = _mat[1][2];
    _mat[1][2] = _mat[2][1];
    _mat[2][1] = temp;
    temp = _mat[1][3];
    _mat[1][3] = _mat[3][1];
    _mat[3][1] = temp;
    temp = _mat[2][3];
    _mat[2][3] = _mat[3][2];
    _mat[3][2] = temp;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Copy
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Copy( Sv_Matrix4 *mat)
{
    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	    _mat[i][j] = mat->GetValue(i, j);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : operator+
// Arguments : 
// Returns   : 
// Purpose   : 
//
Sv_Matrix4 Sv_Matrix4::operator+(Sv_Matrix4 & mat)
{
   Sv_Matrix4 result;

    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	    result._mat[i][j] = _mat[i][j] + mat._mat[i][j];
    return result;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : operator-
// Arguments : 
// Returns   : 
// Purpose   : 
//
Sv_Matrix4 Sv_Matrix4::operator-(Sv_Matrix4 &mat)
{
    Sv_Matrix4 result;

    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	    result._mat[i][j] = _mat[i][j] - mat._mat[i][j];
    return result;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : operator-
// Arguments : 
// Returns   : 
// Purpose   : 
//
Sv_Matrix4 Sv_Matrix4::operator-()
{
    Sv_Matrix4 result;

    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	    result._mat[i][j] =  - _mat[i][j];
    return result;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : operator*
// Arguments : 
// Returns   : 
// Purpose   : 
//
Sv_Matrix4 Sv_Matrix4::operator*(Sv_Matrix4 & mat)
{
    Sv_Matrix4 result;

    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	{
	    result._mat[i][j] = 0.0;
	    for(GLint k=0; k<4; k++)
		result._mat[i][j] += _mat[i][k] * mat._mat[k][j];
	}	 
    return result;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : operator*
// Arguments : 
// Returns   : 
// Purpose   : 
//
Sv_Matrix4 Sv_Matrix4::operator*(GLfloat scalar)
{
    Sv_Matrix4 result;

    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	    result._mat[i][j] = _mat[i][j] * scalar;
    return result;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : operator<<(
// Arguments : 
// Returns   : 
// Purpose   : 
//
std::ostream& operator<<(std::ostream& s, Sv_Matrix4& m)
{
    s << "\t[ [" << m._mat[0][0] << ", " << m._mat[0][1]
      << ", "     << m._mat[0][2] << ", " << m._mat[0][3]
            << "]"      << std::endl;
    s << "\t  [" << m._mat[1][0] << ", " << m._mat[1][1]
      << ", "     << m._mat[1][2] << ", " << m._mat[1][3]
            << "]"      << std::endl;
    s << "\t  [" << m._mat[2][0] << ", " << m._mat[2][1]
      << ", "     << m._mat[2][2] << ", " << m._mat[2][3]
            << "]"      << std::endl;
    s << "\t  [" << m._mat[3][0] << ", " << m._mat[3][1]
      << ", "     << m._mat[3][2] << ", " << m._mat[3][3]
            << "] ]"  << std::endl;
    return s;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : VecMultMat
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::VecMultMat(GLfloat *vec, GLfloat *rv)
{
    GLfloat tempv[4];

    // Use a temporary array to hold the initial result just in	
    // case vec and rv are the same vector.

    tempv[0] = vec[0]*_mat[0][0] + vec[1]*_mat[1][0] + vec[2]*_mat[2][0] + vec[3]*_mat[3][0];
    tempv[1] = vec[0]*_mat[0][1] + vec[1]*_mat[1][1] + vec[2]*_mat[2][1] + vec[3]*_mat[3][1];
    tempv[2] = vec[0]*_mat[0][2] + vec[1]*_mat[1][2] + vec[2]*_mat[2][2] + vec[3]*_mat[3][2];
    tempv[3] = vec[0]*_mat[0][3] + vec[1]*_mat[1][3] + vec[2]*_mat[2][3] + vec[3]*_mat[3][3];
    rv[0] = tempv[0];
    rv[1] = tempv[1];
    rv[2] = tempv[2];
    rv[3] = tempv[3];
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Multiply
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Multiply(GLfloat vec[4], GLfloat r[4])
	{
	GLint   i,j;
	GLfloat val;
	
	for(i=0; i<4; i++)
		{
		val = 0.0;
		for(j=0; j<4; j++)
			val = val + _mat[i][j] * vec[j];
		r[i] = val;  
		}
	}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Interpolate
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Interpolate( Sv_Matrix4 &mat, GLfloat alpha, Sv_Matrix4 &result )
{
    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	    result._mat[i][j] = (float)(mat._mat[i][j]*alpha + (1.0 - alpha)*_mat[i][j]);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : SetTranslate
// Arguments : 
// Returns   : 
// Purpose   : 
//    NOTE:  REVESED ARRAY INDICES FOR DFLEET !!!!!!!!!!!
void Sv_Matrix4::SetTranslate(GLfloat x, GLfloat y, GLfloat z)
{
    SetIdentity();
    _mat[0][3] = x;
    _mat[1][3] = y;
    _mat[2][3] = z;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : SetRotation
// Arguments : 
// Returns   : 
// Purpose   : 
//    NOTE:  REVESED ARRAY INDICES FOR DFLEET !!!!!!!!!!!
//
void Sv_Matrix4::SetRotation(GLfloat angle, char axis)
{
    GLfloat c, s;

    GLfloat ang = (GLfloat) (angle * 3.14159 / 180.0);
    c = (float)cos( ang );
    s = (float)sin( ang );    
    SetIdentity();
    switch( axis )
    {
	case 'x':
	    _mat[1][1] = c;
	    _mat[1][2] = s;
	    _mat[2][1] = -s;
	    _mat[2][2] = c;
	    break;
	    
	case 'y':
	    _mat[0][0] = c;
	    _mat[0][2] = -s;
	    _mat[2][0] = s;
	    _mat[2][2] = c;
	    break;
	    
	case 'z':
	    _mat[0][0] = c;
	    _mat[0][1] = s;
	    _mat[1][0] = -s;
	    _mat[1][1] = c;
	    break;
	default:
            std::cerr << "UNKNOWN TRANSFORMATION MATRIX REQUESTED!\n";
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Write
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Write( char *fileName )
{
    FILE *out;
    
    if( (out = fopen( fileName, "w" )) == NULL )
    {
        std::cerr << "ERROR: Trying to write Sv_Matrix to file: " << fileName << std::endl;
	return;
    }
    
    for(GLint i=0; i<4; i++)
    {
	for(GLint j=0; j<4; j++)
	   fprintf( out, "%f ", _mat[i][j] );
	fprintf( out, "\n" );
    }
    fclose( out );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Write
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Write( FILE *out )
{
    int dummy;
    if( out != NULL )
    {
	for(GLint i=0; i<4; i++)
	    dummy = fwrite( _mat[i], sizeof(GLfloat), 4, out );
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Read
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Read( char *fileName )
{
    FILE *in;
    
    if( (in = fopen( fileName, "r" )) == NULL )
    {
        std::cerr << "ERROR: Trying to read Sv_Matrix to file: " << fileName << std::endl;
	return;
    }
    
    int dummy;
    for(GLint i=0; i<4; i++)
	for(GLint j=0; j<4; j++)
	   dummy = fscanf( in, "%f ", &(_mat[i][j]) );

    fclose( in );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : Read
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::Read( FILE *in )
{
    int dummy;
    if( in != NULL )
    {
	for(GLint i=0; i<4; i++)
	    dummy = fread( _mat[i], sizeof(GLfloat), 4, in );
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : GetMatrixPTR
// Arguments : 
// Returns   : 
// Purpose   : 
//
char *Sv_Matrix4::GetMatrixPTR()
{
    std::cerr << "Sv_Matrix4::GetMatrixPTR!\n";
    return( (char *)_mat );
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function  : SetMatrixFromPTR
// Arguments : 
// Returns   : 
// Purpose   : 
//
void Sv_Matrix4::SetMatrixFromPTR( char *data )
{
    unsigned int  i;
    char *ptr;

    std::cerr << "Sv_Matrix4::SetMatrixFromPTR!\n";
    
    ptr = (char *)_mat;
    for( i=0; i<sizeof(Matrix); i++ )
    {
    	*ptr = *data;
	ptr++;
	data++;
    }
}
////////////////////////////////////////////////////////////////////////////////
