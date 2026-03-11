// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DATA_POLYGON_H
#define GZ4D_DATA_POLYGON_H


#include <gz4d/gz4d.h>
#include <vector>
#include <gz4d/Box.h>

namespace gz4d
{
    /// \ingroup container
    /// Rings are counter-clockwise.
    template <typename T> class Polygon
    {
        public:
            typedef T point_type;
            typedef std::vector<T> ring_type;

            void insert(T const &p)
            {
                exteriorRing.push_back(p);
                boundingBox.expand(p);
            }

            void insert(std::size_t r, T const &p)
            {
                if(interiorRings.size() <= r)
                    interiorRings.resize(r+1);
                interiorRings[r].push_back(p);
                boundingBox.expand(p);
            }

            template<typename IT> void insert(IT begin, IT end)
            {
                exteriorRing.insert(exteriorRing.end(),begin,end);
                for(IT i = begin; i != end; ++i)
                    boundingBox.expand(*i);
            }

            template<typename IT> void insert(std::size_t r, IT begin, IT end)
            {
                if(interiorRings.size() <= r)
                    interiorRings.resize(r+1);
                interiorRings[r].insert(interiorRings[r].end(),begin,end);
                for(IT i = begin; i != end; ++i)
                    boundingBox.expand(*i);
            }

            ring_type const &getExteriorRing() const
            {
                return exteriorRing;
            }

            ring_type const &getInteriorRing(std::size_t i) const
            {
                assert(i<interiorRings.size());
                return interiorRings[i];
            }

            std::vector<ring_type> const &getInteriorRings() const
            {
                return interiorRings;
            }

            Box<T> const &getBoundingBox() const
            {
                return boundingBox;
            }

            T sw() const
            {
                T ret;
                ret[0] = boundingBox.getMin()[0]-(boundingBox.getSizes()[0]*0.01);
                ret[1] = boundingBox.getMin()[1]-(boundingBox.getSizes()[1]*0.01);
                return ret;
            }

            T nw() const
            {
                T ret;
                ret[0] = boundingBox.getMin()[0]-(boundingBox.getSizes()[0]*0.01);
                ret[1] = boundingBox.getMax()[1]+(boundingBox.getSizes()[1]*0.01);
                return ret;
            }

        private:
            ring_type exteriorRing;
            std::vector<ring_type> interiorRings;
            Box<T> boundingBox;
    };
}


#endif
