#ifndef GZ4D_SPHERICALPOINT_H
#define GZ4D_SPHERICALPOINT_H

#include <gz4d/gz4d.h>
#include <gz4d/Point.h>
#include "Angle.h"
#include <cmath>

namespace gz4d
{
    /// A point represented in spherical coordinates (r,theta,phi).
    /// Radius r is distance from origin.
    /// Theta is 0 when along the Z axis, and PI/2 when along the XY plane.
    /// Phi is the ccw rotaion around the Z axis of the point projected on the XY plane,
    /// with 0 being along the X axis.
    /// x = sin(theta) cos(phi)
    /// y = sin(theta) sin(phi)
    /// z = cos(theta)
    template<typename T> class SphericalPoint
    {
        T radius;

        typedef Angle<T,Radian> theta_type;
        theta_type theta;
        
        typedef Angle<T,Radian> phi_type;
        phi_type phi;

        public:
            SphericalPoint(){}
            SphericalPoint(SphericalPoint const &o):radius(o.radius),theta(o.theta),phi(o.phi){}
            SphericalPoint(T radius, theta_type theta, phi_type phi):radius(radius),theta(theta),phi(phi)
            {
                if (!(radius > 0.0)) this->radius = 0.0;
            }
            operator Point<T>()
            {
                return Point<T>(sin(theta)*cos(phi)*radius,sin(theta)*sin(phi)*radius,cos(theta)*radius);
            }

            SphericalPoint<T> const &operator+=(SphericalPoint<T> const &o)
            {
                radius += o.radius;
                theta += o.theta;
                phi += o.phi;
                return *this;
            }
    };
}

#endif
