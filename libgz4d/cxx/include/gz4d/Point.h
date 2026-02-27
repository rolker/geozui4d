#ifndef GZ4D_POINT_H
#define GZ4D_POINT_H

#include <gz4d/Vector.h>
#include <gz4d/util/Utilities.h>

namespace gz4d
{
    /// \ingroup base
    template<typename T> class Point : public Vector<T,3>
    {
        public:
            Point():Vector<T,3>(0.0){}
            Point(Vector<T,3> const &v):Vector<T,3>(v){}
            Point(Vector<T,4> const &v):Vector<T,3>(v[0]/v[3],v[1]/v[3],v[2]/v[3]){}
            Point(T x, T y, T z):Vector<T,3>(x, y, z){}
            using Vector<T, 3>::operator=;
            static Point Invalid(){return Vector<T,3>(Nan<T>());}
            bool IsValid() const {return !(IsNan(Vector<T,3>::values[0])||IsNan(Vector<T,3>::values[1])||IsNan(Vector<T,3>::values[2]));}
            operator Vector<T, 4>() const {return Vector<T,4>(Vector<T,3>::values[0],Vector<T,3>::values[1],Vector<T,3>::values[2],1);}
    };
}


#endif
