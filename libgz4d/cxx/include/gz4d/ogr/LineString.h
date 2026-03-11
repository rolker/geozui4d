// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_OGR_LINESTRING_H
#define GZ4D_OGR_LINESTRING_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/Curve.h>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <gz4d/ogr/Point.h>
#include <ogr_geometry.h>

namespace gz4d
{
    namespace ogr
    {
        class LineString: public Curve
        {

            std::shared_ptr<OGRLineString> lineString;
            public:
                class const_iterator: public boost::iterator_facade<const_iterator,gz4d::Point<double> const,boost::random_access_traversal_tag>
                {
                    std::shared_ptr<OGRLineString> lineString;
                    int index;
                    gz4d::Point<double> point;

                    friend class boost::iterator_core_access;

                    void update();
                    void increment();
                    void decrement();
                    void advance(std::ptrdiff_t n);
                    std::ptrdiff_t distance_to(const_iterator const &other) const;
                    bool equal (const_iterator const& other) const;
                    gz4d::Point<double> const &dereference() const {return point;}
                    

                    friend class LineString;
                    const_iterator(std::shared_ptr<OGRLineString> ls, int i);
                        
                    public:
                        const_iterator();
                };

                typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

                LineString(Geometry const &g);
                operator bool() const {return bool(lineString);}

                std::size_t size() const;
                const_iterator begin();
                const_iterator end();
                const_reverse_iterator rbegin();
                const_reverse_iterator rend();
        };
    }
}


inline gz4d::ogr::LineString::LineString(Geometry const &other)
:Curve(other)
{
    lineString = std::dynamic_pointer_cast<OGRLineString>(geometry);
}

inline std::size_t gz4d::ogr::LineString::size() const
{
    return lineString->getNumPoints();
}


inline gz4d::ogr::LineString::const_iterator gz4d::ogr::LineString::begin()
{
    return const_iterator(lineString,0);
}

inline gz4d::ogr::LineString::const_iterator gz4d::ogr::LineString::end()
{
    return const_iterator(lineString,lineString->getNumPoints());
}

inline gz4d::ogr::LineString::const_reverse_iterator gz4d::ogr::LineString::rbegin()
{
    return const_reverse_iterator(const_iterator(lineString,lineString->getNumPoints()-1));
}

inline gz4d::ogr::LineString::const_reverse_iterator gz4d::ogr::LineString::rend()
{
    return const_reverse_iterator(const_iterator(lineString,-1));
}

inline gz4d::ogr::LineString::const_iterator::const_iterator()
:index(0)
{
}

inline gz4d::ogr::LineString::const_iterator::const_iterator(std::shared_ptr<OGRLineString> ls, int i)
:lineString(ls),index(i)
{
    update();
}

inline void gz4d::ogr::LineString::const_iterator::update()
{
    if(index >= 0 && index < lineString->getNumPoints())
    {
        Point p;
        lineString->getPoint(index,p.point.get());
        point = p;
    }
}

inline void gz4d::ogr::LineString::const_iterator::increment()
{
    ++index;
    update();
}

inline void gz4d::ogr::LineString::const_iterator::decrement()
{
    --index;
    update();
}

inline bool gz4d::ogr::LineString::const_iterator::equal (const_iterator const &other) const
{
    return lineString && lineString == other.lineString && index == other.index;
}


inline void gz4d::ogr::LineString::const_iterator::advance(std::ptrdiff_t n)
{
    index += n;
    update();
}

inline std::ptrdiff_t gz4d::ogr::LineString::const_iterator::distance_to(const_iterator const &other) const
{
    return other.index - index;
}

#endif
