// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
#ifndef SDV_TYPES
#define SDV_TYPES

#ifndef NDEBUG
#define glError() { \
        GLenum err = glGetError(); \
        while (err != GL_NO_ERROR) { \
                fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
                err = glGetError(); \
} \
}
#else
#define glError()
#endif

#include<gz4d/glo.h>

#ifdef WIN32
#include "Togl/togl.h" //this does a good job of setting up includes properly for glu.h
#endif

#include <tcl.h>
#include <tk.h>

// X11/Xlib.h defines 'Complex' as a macro (= 0), which conflicts with
// Boost.Math template parameter names. Undefine it here.
#ifdef Complex
#undef Complex
#endif

#include "widgets/Sv_Standards.h"


#define LEFT_EYE 0
#define RIGHT_EYE 1
#define MONO_EYE 2

#define SDV_LEFT_BUTTON 1
#define SDV_MIDDLE_BUTTON 2
#define SDV_RIGHT_BUTTON 3

#define SDV_DOWN 0
#define SDV_UP 1

#define SDV_MONO 0
#define SDV_STEREO 1

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define RAD2DEG(x)  ((x)*180.0/M_PI)
#define DEG2RAD(x)  ((x)*M_PI/180.0)

#ifdef WIN32

#include "float.h"
#else

#include "math.h"  

#endif
        
// 3D point


typedef float Point3f[3];
typedef float Point2f[2];
typedef double Point3d[3];
typedef double Point2d[2];
typedef float Point4f[4];

#define COPY_POINT(dest, src) {\
            (dest)[X] = (src)[X]; \
            (dest)[Y] = (src)[Y]; \
            (dest)[Z] = (src)[Z];}

#define SET_POINT(dest, x, y, z) {\
            (dest)[X] = (float)(x); \
            (dest)[Y] = (float)(y); \
            (dest)[Z] = (float)(z);}
            
#define CERR_POINT(p) p[X] << ", " << p[Y] << ", " << p[Z]

#define DIFF_POINT(a,b,c) {\
            (c)[X] = (a)[X]-(b)[X]; \
            (c)[Y] = (a)[Y]-(b)[Y]; \
            (c)[Z] = (a)[Z]-(b)[Z];}
            
#define ADD_POINT(a,b,c) {\
            (c)[X] = (a)[X]+(b)[X]; \
            (c)[Y] = (a)[Y]+(b)[Y]; \
            (c)[Z] = (a)[Z]+(b)[Z];}

typedef int Point3i[3];

typedef GLubyte Point4ub[4];

typedef GLubyte Color3ub[3];

//typedef GLubyte CLUTType[1000][4]

typedef GLshort vNorms[3];
//struct vNorms
//{
//  GLshort nx, ny, nz;  // separate out these from height
//  GLubyte color;
//};

#include "utilities/BBox.h"
#include <typeinfo>
        
#define DYNAMIC_CAST(targetVar, type, sourceVar) \
    try { \
        targetVar = dynamic_cast<type>(sourceVar); \
    } catch(...) { \
        targetVar = NULL; \
    }

/* gcc and VC++ have different ideas of what typeid().name() should return*/
/* usage:  GET_CLASS_NAME(<class_pointer>, <const char pointer>); */
#ifdef WIN32
#define GET_CLASS_NAME(ob, result) result = std::string(&(typeid(*ob).name()[6]))
#else
#define GET_CLASS_NAME(ob, result) \
{   int indexToName = 0; \
    const char *GET_CLASS_NAME_STRING = typeid(*ob).name(); \
    while (GET_CLASS_NAME_STRING[indexToName] && isdigit(GET_CLASS_NAME_STRING[indexToName])) \
        indexToName++; \
                 result = std::string(&GET_CLASS_NAME_STRING[indexToName]); \
}
#endif
    
#endif
