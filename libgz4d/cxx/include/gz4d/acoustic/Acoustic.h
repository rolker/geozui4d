// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_ACOUSTIC_H
#define GZ4D_ACOUSTIC_H

#include <gz4d/gz4d.h>
#include <gz4d/Vector.h>

namespace gz4d
{
    /// Acoustic and SONAR related classes.
    /// In a separate namsepace because of many terms similar
    /// to generic concepts (ex: Element, Array, etc.)
    /// \ingroup acoustic
    namespace acoustic
    {
        /// \defgroup acoustic SONAR and Acoustics
        /// \ingroup utilities
        /// \author Roland Arsenault
        ///
        /// Acoustic simulation and visualization.
        ///
        /// The classes in this module originate from the implementation of
        /// homework assignments for OE845/OE846 (Environmental Acoustics)
        /// taken by rja in 2007/2008.
        ///
        /// Array and element coordinates.
        ///
        /// Planar arrays and elements have their surface in the xy plane
        /// centered at 0,0,0 and radiate towards the positive Z axis.
        /// 
        /// Wave plane frequency (f) and sound speed (c) are abstracted by
        /// wavelength (lambda) and wave number (k).
        /// lambda = c/f
        /// k = 2pi/lambda
        ///
        /// Wave direction is specified with two angles, theta and phi.
        /// A direction vector K is derived from these angles, and is
        /// perpendicular to the wave plane.
        /// K = (kx,ky,kz)
        /// Kx = k sin(theta)cos(phi)
        /// Ky = k sin(theta)sin(phi)
        /// Kz = k cos(theta)
        ///
        /// In other words, theta is the angle between K and the Z axis
        /// and phi is the angle between the X axis and K's projection
        /// on the XY plane.


        /// 2D spacing type, usually expressed in wave lengths.
        /// \ingroup acoustic
        typedef Vector<double,2> Size;
/*        class GZ4DEXPORT Size: public Vector<double,2>
        {
            public:
                Size():Vector<double,2>(1.0){}
                Size(Size const &s):Vector<double,2>(s){}
                Size(double s):Vector<double,2>(s){}
                Size(double x, double y);

                using Vector<double,2>::operator=;
        };*/
    }
}

#endif

