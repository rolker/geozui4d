// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GEO_TILESET_H
#define GZ4D_GEO_TILESET_H

#include <gz4d/Box.h>
#include <gz4d/SubdivisionTree.h>
#include <math.h>

namespace gz4d
{
    namespace geo
    {
        template<uint8_t NL=27> struct EarthTileSpecs
        {
            static Vector<double,2> base_origin(){ return Vector<double,2>(-180.0,-90.0);}
            static Vector<double,2> base_units_per_pixel(){ return Vector<double,2>(0.703125,0.703125);}
            static Vector<std::size_t,2> tile_size(){ return Vector<std::size_t,2>(256,256);}
            static const std::size_t base_scale = 1<<NL;
        };

        template<typename TS=EarthTileSpecs<> > struct Index2D: public subtree::Index<Index2D<TS>,2>
        {
            typedef subtree::Index<Index2D<TS>,2> base_type;
            
            Index2D(typename base_type::level_type level, typename base_type::normalized_coordinates_type const &coordinates):base_type(level,(coordinates-TS::base_origin())/(TS::base_units_per_pixel()*TS::tile_size())){}

            uint32_t scale() const
            {
                return TS::base_scale>>base_type::_level;
            }
        };
        
        template<typename T, typename TS=EarthTileSpecs<> > class TileSet: public SubdivisionTree<TileSet<T,TS>,  T,Index2D<TS> >
        {
        public:
            typedef TileSet<T,TS> self_type;
            typedef SubdivisionTree<self_type,T,Index2D<TS> > base_type;
            typedef TS tile_specs;
        
            static typename base_type::index_type::level_type level(std::size_t scale)
            {
                double l = tile_specs::base_scale/double(scale);
#ifdef BOOST_MSVC
				return log(l)/log(2.0);
#else
                return log2(l);
#endif

            }
        };
    }
}

#endif
