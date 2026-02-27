#ifndef GZ4D_DATA_SEGMENT_H
#define GZ4D_DATA_SEGMENT_H

#include <gz4d/gz4d.h>
#include <gz4d/Box.h>
#include "Triangle.h"

namespace gz4d
{
    /// \ingroup container
    template <typename T> struct Segment
    {
        T a,b;

        Segment(){}
        Segment(Segment<T> const &s):a(s.a),b(s.b){}
        Segment(T const &a, T const &b):a(a),b(b){}

        typename T::value_type length() const {return norm(b-a);}

        /// Only takes 2D case into account.
        bool intersects(Segment<T> const &s) const
        {
            // I is intersection point
            //
            // (1)  I = a+p(b-a)
            // (2)  I = s.a + sp(s.b-s.a)
            // (3)  a+p(b-a) = s.a + sp(s.b-s.a)
            //
            // split into 2 eq with 2 unknowns
            //
            // (4)  a[0]+p(b[0]-a[0]) = s.a[0] + sp(s.b[0]-s.a[0])
            // (5)  a[1]+p(b[1]-a[1]) = s.a[1] + sp(s.b[1]-s.a[1])
            // 
            // solve for p and sp
            // 
            // (6)       (a[0]+p(b[0]-a[0])-s.a[0])   (a[1]+p(b[1]-a[1])-s.a[1])
            //      sp = -------------------------- = --------------------------
            //                 (s.b[0]-s.a[0])             (s.b[1]-s.a[1])
            //
            // (7)   p(b[0]-a[0])    p(b[1]-a[1])    a[1]-s.a[1]     a[0]-s.a[0]
            //       ------------- - ------------- = ------------- - -------------
            //       s.b[0]-s.a[0]   s.b[1]-s.a[1]   s.b[1]-s.a[1]   s.b[0]-s.a[0]
            //
            // (8)      (s.b[0]-s.a[0])(a[1]-s.a[1])-(s.b[1]-s.a[1])(a[0]-s.a[0])
            //      p = ---------------------------------------------------------
            //          (s.b[1]-s.a[1])(b[0]-a[0]) - (s.b[0]-s.a[0])(b[1]-a[1])
            //
            // (9)       (b[0]-a[0])(s.a[1]-a[1])-(b[1]-a[1])(s.a[0]-a[0])
            //      sp = -------------------------------------------------------
            //           (b[1]-a[1])(s.b[0]-s.a[0]) - (b[0]-a[0])(s.b[1]-s.a[1])
            // 
            // flip a few signs and rearange 9 to match denominator of 8
            //
            // (10)        (b[0]-a[0])(a[1]-s.a[1])-(b[1]-a[1])(a[0]-s.a[0])
            //      sp = -------------------------------------------------------
            //           (s.b[1]-s.a[1])(b[0]-a[0]) - (s.b[0]-s.a[0])(b[1]-a[1])

            T diff = b-a;
            T sdiff = s.b -s.a;
            typename T::value_type d = sdiff[1]*diff[0]-sdiff[0]*diff[1];
            typename T::value_type n = sdiff[0]*(a[1]-s.a[1]) - sdiff[1]*(a[0]-s.a[0]);
            typename T::value_type sn = diff[0]*(a[1]-s.a[1]) - diff[1]*(a[0]-s.a[0]);
            if (d == 0.0)//parallel
            {
                if(n == 0 && sn == 0) // coincident
                {
                    Box<T> bbox;
                    T p;
                    p[0] = a[0];
                    p[1] = a[1];
                    bbox.expand(p);
                    p[0] = b[0];
                    p[1] = b[1];
                    bbox.expand(p);

                    Box<T> sbbox;
                    p[0] = s.a[0];
                    p[1] = s.a[1];
                    sbbox.expand(p);
                    p[0] = s.b[0];
                    p[1] = s.b[1];
                    sbbox.expand(p);
                    return bbox.intersects(sbbox);
                }
                return false;
            }
            typename T::value_type p = n/d;
            typename T::value_type sp = sn/d;
            if(p>0.0 && p<1.0 && sp > 0.0 && sp < 1.0)
                return true;
            return false;
        }

    };
}

#endif
