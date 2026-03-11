// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_TRIANGLE_H
#define GZ4D_DATA_TRIANGLE_H

#include <gz4d/gz4d.h>
#include <vector>
#include "Segment.h"
#include "Circle.h"
#include <gz4d/Matrix.h>
#include <gz4d/matrix/operations.h>

namespace gz4d
{
    /// \ingroup container
    template <typename T> struct Triangle
    {
        T a,b,c;
        Triangle(){}
        Triangle(Triangle<T> const &t):a(t.a),b(t.b),c(t.c){}
        Triangle(T const &a, T const &b, T const &c):a(a),b(b),c(c){}

        bool valid() const {return norm2(cross(b-a,c-a))>0.0;}

        /// For 2D triangles or 3D triangles with front face up.
        bool clockwise() const {return cross(b-a,c-b)[2] < 0.0;}

        typename T::value_type area() const{ return norm(cross(b-a,c-a))/2.0; }

        /// For 3D triangles, works on the 2D projection on to the XY plane of triangles facing up.
        bool contains(T const &p) const 
        {
            if(p == a || p == b || p == c)
                return true;
            if(clockwise())
                return Triangle<T>(a,b,p).clockwise() && Triangle<T>(b,c,p).clockwise() && Triangle<T>(c,a,p).clockwise();
            else
                return !Triangle<T>(a,b,p).clockwise() && !Triangle<T>(b,c,p).clockwise() && !Triangle<T>(c,a,p).clockwise();
        }

        Triangle<T> extend(T const &p) const
        {
            bool cw = clockwise();
            if(Triangle<T>(a,p,b).clockwise() == cw)
                return Triangle<T>(a,p,b);
            if(Triangle<T>(b,p,c).clockwise() == cw)
                return Triangle<T>(b,p,c);
            if(Triangle<T>(c,p,a).clockwise() == cw)
                return Triangle<T>(c,p,a);
            return Triangle<T>();
        }

        std::vector<Triangle<T> > split(T const &p) const
        {
            std::vector<Triangle<T> > ret;
            if(!contains(p))
                return ret;
            ret.push_back(Triangle<T>(a,b,p));
            ret.push_back(Triangle<T>(b,c,p));
            ret.push_back(Triangle<T>(c,a,p));
            return ret;
        }
        
        Circle<T> circumcircle() const
        {
            T ca = a-c;
            T cb = b-c;

            typename T::value_type nca = norm(ca);
            typename T::value_type ncb = norm(cb);
            typename T::value_type nacb = norm(cross(ca,cb));

            typename T::value_type r = nca*ncb*norm(ca-cb)/(2.0*nacb);
            T p = cross((cb*pow(nca,2.0)-ca*pow(ncb,2.0)),cross(ca,cb))/(2.0*pow(nacb,2.0))+c;
            return Circle<T>(p,r);
        }

        typename T::value_type delaunay(T const &p) const
        {
            Matrix<typename T::value_type,3,3> mat;
            int br = 1;
            int cr = 2;
            if(clockwise())
            {
                br = 2;
                cr = 1;
            }
            mat(0,0) = a[0] - p[0];
            mat(0,1) = a[1] - p[1];
            mat(0,2) = ((a[0]*a[0])-(p[0]*p[0]))+((a[1]*a[1])-(p[1]*p[1]));
            mat(br,0) = b[0] - p[0];
            mat(br,1) = b[1] - p[1];
            mat(br,2) = ((b[0]*b[0])-(p[0]*p[0]))+((b[1]*b[1])-(p[1]*p[1]));
            mat(cr,0) = c[0] - p[0];
            mat(cr,1) = c[1] - p[1];
            mat(cr,2) = ((c[0]*c[0])-(p[0]*p[0]))+((c[1]*c[1])-(p[1]*p[1]));
            return determinant(mat);
        }

    };
}


#endif
