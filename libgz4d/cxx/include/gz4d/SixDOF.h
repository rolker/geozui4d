#ifndef GZ4D_SIXDOF_H
#define GZ4D_SIXDOF_H

#include <gz4d/gz4d.h>
#include <gz4d/Point.h>
#include <gz4d/Rotation.h>

namespace gz4d
{
    /// \ingroup base
    template<typename T> struct SixDOF
    {
        Point<T> position;
        Rotation<T> orientation;

        SixDOF()
        {
        }

        SixDOF(SixDOF<T> const &sd)
            :position(sd.position),orientation(sd.orientation)
        {
        }

        SixDOF(Point<T> const &p, Rotation<T> const &r)
            :position(p),orientation(r)
        {
        }

        SixDOF(Point<T> const &p)
            :position(p)
        {
        }

        SixDOF(Rotation<T> const &r)
            :orientation(r)
        {
        }

        Point<T> operator()(Point<T> const &p) const
        {
            return position + orientation(p);
        }

        Point<T> reverse(Point<T> const &p) const
        {
            return orientation.Inverse()(p-position);
        }

        SixDOF<T> operator()(SixDOF<T> const &s) const
        {
            SixDOF<T> ret;
            ret.position = (*this)(s.position);
            ret.orientation = orientation*s.orientation;
            return ret;
        }

        
    };
}


#endif
