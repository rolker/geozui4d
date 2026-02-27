#ifndef GZ4D_ROTATION_H
#define GZ4D_ROTATION_H

#include <gz4d/gz4d.h>
#include <boost/math/quaternion.hpp>
#include <gz4d/Point.h>
#include <gz4d/Units.h>
#include <gz4d/Matrix.h>
#include <gz4d/matrix/operations.h>
#include <gz4d/vector/operations.h>
#include "Angle.h"

namespace gz4d
{
    /// \ingroup base
    template<typename T>
    class Rotation: public boost::math::quaternion<T>
    {
        public:
            Rotation()
            :boost::math::quaternion<T>(1)
            {}

            template<typename T2> Rotation(Rotation<T2> const &r)
            :boost::math::quaternion<T>(r)
            {}

            template<typename T2> Rotation(boost::math::quaternion<T2> const &q)
            :boost::math::quaternion<T>(q)
            {}

            template<typename AT, typename RT> Rotation(Angle<AT,Degree,RT> angle, Point<T> const &axis)
            {
                Rotation<T>::Set(Angle<AT,Radian,RT>(angle),axis);
            }

            template<typename AT, typename RT> Rotation(Angle<AT,Radian,RT> angle, Point<T> const &axis)
            {
                Rotation<T>::Set(angle ,axis);
            }
            
            
            template<typename T2> Rotation(Point<T2> const &v1, Point<T2> const &v2)
            :boost::math::quaternion<T>(1)
            {
                T2 n = norm(v1)*norm(v2);
                if(n > 0.0)
                {
                    Angle<T2,Radian> angle  = acos(v1.dot(v2)/n);
                    Point<T2> axis = cross(v1,v2);
                    Rotation<T>::Set(angle,axis);
                }
            }


            Angle<T,Radian,ZeroCenteredPeriod> angle() const
            {
                return acos(boost::math::quaternion<T>::real())*2.0;
            }

            /// Sets rotation using angle around axis
            /// angle is in radians
            template<typename T2, typename RT> void Set(Angle<T2,Radian,RT> angle, Point<T2> const &axis)
            {
                if(norm2(axis) > 0.0)
                {
                    Point<T2> a = normalize(axis) * sin(angle/2.0);
                    *this = boost::math::quaternion<T>(cos(angle/2.0),a[0],a[1],a[2]);
                }
            }

            Point<T> operator()(Point<T> const &vector) const
            {
                boost::math::quaternion<T> v(0.0,vector[0],vector[1],vector[2]);
                v = (*this) * v * conj(*this);
                return Point<T>(v.R_component_2(),v.R_component_3(),v.R_component_4());
            }

            Rotation<T> Inverse() const
            {
                return conj(*this);
            }

            Matrix<T,4,4> GetMatrix() const
            {
                Matrix<T,4,4> ret = Matrix<T,4,4>::Identity();

                T a = this->R_component_1();
                T b = this->R_component_2();
                T c = this->R_component_3();
                T d = this->R_component_4();
                T a2 = a*a;
                T b2 = b*b;
                T c2 = c*c;
                T d2 = d*d;

                ret(0,0) = a2 + b2 - c2 - d2;
                ret(1,0) = 2.0*(a*d + b*c);
                ret(2,0) = 2.0*(b*d - a*c);

                ret(0,1) = 2.0*(b*c - a*d);
                ret(1,1) = a2 - b2 + c2 - d2;
                ret(2,1) = 2.0*(a*b + c*d);

                ret(0,2) = 2.0*(a*c + b*d);
                ret(1,2) = 2.0*(c*d - a*b);
                ret(2,2) = a2 - b2 - c2 + d2;
                return ret;
            }

            Matrix<T,4,4> GetInverseMatrix() const
            {
                return Inverse().GetMatrix();
            }
    };

}

#endif
