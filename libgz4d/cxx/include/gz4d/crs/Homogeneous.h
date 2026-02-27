#ifndef GZ4D_CRS_HOMOGENEOUS_H
#define GZ4D_CRS_HOMOGENEOUS_H

#include "Cartesian.h"

namespace gz4d
{
    namespace crs
    {
        namespace dir
        {
            struct Homogeneous
            {
            };
        }
        
        namespace axis
        {
            template <typename B, typename DIR> struct HX : Axis<B,DIR,Unitless>
            {
                enum {hx_index=Axis<B,DIR,Unitless>::index};
            };
            
            template <typename B, typename DIR> struct HY : Axis<B,DIR,Unitless>
            {
                enum {hy_index=Axis<B,DIR,Unitless>::index};
            };
            
            template <typename B, typename DIR> struct HZ : Axis<B,DIR,Unitless>
            {
                enum {hz_index=Axis<B,DIR,Unitless>::index};
            };

            template <typename B> struct HW : Axis<B,dir::Homogeneous,Unitless>
            {
                enum {hw_index=Axis<B,dir::Homogeneous,Unitless>::index};
            };
            
            template <typename B, typename DIRX, typename DIRY> struct HXY: HY< HX<B,DIRX>, DIRY>
            {
            };
            
            template <typename B, typename DIRX, typename DIRY, typename DIRZ> struct HXYZ: HZ< HXY<B,DIRX,DIRY>, DIRZ>
            {
            };

            template <typename B, typename DIRX, typename DIRY> struct HXYW: HW< HXY<B,DIRX,DIRY>, DIRZ>
            {
            };

            template <typename B, typename DIRX, typename DIRY, typename DIRZ> struct HXYZW: HW< HXYZ<B,DIRX,DIRY,DIRZ> >
            {
            };
            
            
        }            
        
        namespace cs
        {
            template <typename A> struct Homogeneous: UserDefined<A>
            {
            };
            
            template <typename B=axis::InitAxis> struct RightHandHXYZ: Homogeneous<axis::HXYZ<B, dir::Right, dir::Up, dir::Near> >
            {
            };
        }
    }
}

#endif
