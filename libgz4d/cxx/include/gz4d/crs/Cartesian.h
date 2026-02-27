#ifndef GZ4D_CRS_CARTESIAN_H
#define GZ4D_CRS_CARTESIAN_H

#include "Axis.h"
#include "CoordinateSystem.h"

namespace gz4d
{
    namespace crs
    {
        namespace axis
        {
            template <typename B, typename DIR, typename UNITS> struct X : Axis<B,DIR,UNITS>
            {
                enum {x_index=Axis<B,DIR,UNITS>::index};
            };
            
            template <typename B, typename DIR, typename UNITS> struct Y : Axis<B,DIR,UNITS>
            {
                enum {y_index=Axis<B,DIR,UNITS>::index};
            };
            
            template <typename B, typename DIR, typename UNITS> struct Z : Axis<B,DIR,UNITS>
            {
                enum {z_index=Axis<B,DIR,UNITS>::index};
            };
            
            template <typename B, typename DIRX, typename DIRY, typename UNITS> struct XY: Y< X<B,DIRX,UNITS>, DIRY , UNITS>
            {
            };
            
            template <typename B, typename DIRX, typename DIRY, typename DIRZ, typename UNITS> struct XYZ: Z< XY<B,DIRX,DIRY,UNITS>, DIRZ, UNITS>
            {
            };
        }            
        
        namespace cs
        {
            /// A 1-, 2-, or 3-dimensional coordinate system. In the 1-dimensional case, it contains a single straight coordinate axis. In the
            /// 2- and 3-dimensional cases it gives the position of points relative to orthogonal straight axes. In the multi-dimensional cases,
            /// all axes shall have the same length unit of measure. 
            template <typename A> struct Cartesian: CoordinateSystem<A>
            {
            };
            
            template <typename B=axis::InitAxis> struct RightHandXYZ: Cartesian<axis::XYZ<B, dir::Right, dir::Up, dir::Near,axis::Meter > >
            {
            };

            template <typename B=axis::InitAxis> struct RightHandXY: Cartesian<axis::XY<B, dir::Right, dir::Up, axis::Meter > >
            {
            };
            
        }
    }
}

#endif
