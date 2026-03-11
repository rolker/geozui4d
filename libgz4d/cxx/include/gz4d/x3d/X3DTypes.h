// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_X3DTYPES_H
#define GZ4D_X3DTYPES_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>

#include <vector>
#include <gz4d/Vector.h>
#include <gz4d/Matrix.h>
#include <gz4d/time.h>
#include <memory>

namespace gz4d
{

    //class X3DNode;

    // See...
    // http://www.web3d.org/x3d/specifications/ISO-IEC-19775-X3DAbstractSpecification/

    typedef bool SFBool;
    typedef std::vector<SFBool> MFBool;

    typedef Vector<float,3> SFColor;
    typedef std::vector<SFColor> MFColor;

    typedef Vector<float,4> SFColorRGBA;
    typedef std::vector<SFColorRGBA> MFColorRGBA;

    typedef double SFDouble;
    typedef std::vector<SFDouble> MFDouble;

    typedef float SFFloat;
    typedef std::vector<SFFloat> MFFloat;

    struct SFImage
    {
        unsigned int width;
        unsigned int height;
        unsigned char components;
        std::vector<unsigned int> pixels;
    };
    typedef std::vector<SFImage> MFImage;

    typedef int SFInt32;
    typedef std::vector<SFInt32> MFInt32;


    typedef std::string SFString;
    typedef std::vector<SFString> MFString;

    typedef Time SFTime;
    typedef std::vector<SFTime> MFTime;

    typedef Vector<double,2> SFVec2d;
    typedef std::vector<SFVec2d> MFVec2d;

    typedef Vector<float,2> SFVec2f;
    typedef std::vector<SFVec2f> MFVec2f;

    typedef Vector<double,3> SFVec3d;
    typedef std::vector<SFVec3d> MFVec3d;

    typedef Vector<float,3> SFVec3f;
    typedef std::vector<SFVec3f> MFVec3f;

    typedef Vector<float,4> SFRotation;
    typedef std::vector<SFRotation> MFRotation;
    
    typedef Vector<double,4> SFVec4d;
    typedef std::vector<SFVec4d> MFVec4d;

    typedef Vector<float,4> SFVec4f;
    typedef std::vector<SFVec4f> MFVec4f;

    typedef Matrix<float,3,3> SFMatrix3f;
    typedef std::vector<SFMatrix3f> MFMatrix3f;

    typedef Matrix<float,4,4> SFMatrix4f;
    typedef std::vector<SFMatrix4f> MFMatrix4f;

};

#endif
