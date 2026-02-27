#ifndef GZ4D_OGR_GEOMETRY_COLLECTION_H
#define GZ4D_OGR_GEOMETRY_COLLECTION_H

#include <gz4d/ogr/common.h>
#include <gz4d/ogr/Geometry.h>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/reverse_iterator.hpp>

class OGRGeometryCollection;

namespace gz4d
{
    namespace ogr
    {
        class GeometryCollection: public Geometry
        {
            std::shared_ptr<OGRGeometryCollection> geometryCollection;
            public:
                template <typename T, typename RT, typename OT> class iterator_base: public boost::iterator_facade<iterator_base<T,RT,OT>, RT,boost::random_access_traversal_tag>
                {
                    std::shared_ptr<OT> ogrObject;
                    int index;
                    T gz4dObject;

                    friend class boost::iterator_core_access;

                    void increment()
                    {
                        gz4dObject = T(ogrObject,++index);
                    }

                    void decrement()
                    {
                        gz4dObject = T(ogrObject,--index);
                    }

                    void advance(std::ptrdiff_t n)
                    {
                        index += n;
                        gz4dObject = T(ogrObject,index);
                    }

                    std::ptrdiff_t distance_to(iterator_base<T,RT,OT> const &other) const
                    {
                        return other.index - index;
                    }

                    bool equal (iterator_base<T,RT,OT> const& other) const
                    {
                        return ogrObject && ogrObject == other.ogrObject && index == other.index;
                    }

                    RT &dereference() const
                    {
                        return gz4dObject;
                    }

                    public:
                        iterator_base(std::shared_ptr<OT> o, int i)
                            :ogrObject(o),index(i)
                        {
                            gz4dObject = T(ogrObject,index);
                        }

                        iterator_base()
                        :index(0)
                        {}
                };

                typedef iterator_base<Geometry,Geometry const,OGRGeometryCollection> const_iterator;
                typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

                inline GeometryCollection(Geometry const &g);
                inline operator bool() const {return bool(geometryCollection);}
                inline std::size_t size() const;
                inline const_iterator begin();
                inline const_iterator end();
                inline const_reverse_iterator rbegin();
                inline const_reverse_iterator rend();

        };
        
        template <typename T, typename OT> class DerivedGeometryCollection: public GeometryCollection
        {
            protected:
                std::shared_ptr<OT> ogrObject;
                DerivedGeometryCollection(Geometry const &g)
                :GeometryCollection(g){}
            public:
                typedef iterator_base<T, T const, OT> const_iterator;
                typedef boost::reverse_iterator<const_iterator> const_reverse_iterator;

                operator bool() const {return bool(ogrObject);}

                const_iterator begin(){ return const_iterator(ogrObject,0);}
                const_iterator end(){return const_iterator(ogrObject,size());}
                const_reverse_iterator rbegin(){return const_reverse_iterator(end()-1);}
                const_reverse_iterator rend(){return const_reverse_iterator(begin()-1);}
        };

    }
}

#include <ogr_geometry.h>

inline gz4d::ogr::GeometryCollection::GeometryCollection(Geometry const &other)
:Geometry(other)
{
    geometryCollection = std::dynamic_pointer_cast<OGRGeometryCollection>(geometry);
}

inline std::size_t gz4d::ogr::GeometryCollection::size() const
{
    return geometryCollection->getNumGeometries();
}


inline gz4d::ogr::GeometryCollection::const_iterator gz4d::ogr::GeometryCollection::begin()
{
    return const_iterator(geometryCollection,0);
}

inline gz4d::ogr::GeometryCollection::const_iterator gz4d::ogr::GeometryCollection::end()
{
    return const_iterator(geometryCollection,geometryCollection->getNumGeometries());
}

inline gz4d::ogr::GeometryCollection::const_reverse_iterator gz4d::ogr::GeometryCollection::rbegin()
{
    return const_reverse_iterator(const_iterator(geometryCollection,geometryCollection->getNumGeometries()-1));
}

inline gz4d::ogr::GeometryCollection::const_reverse_iterator gz4d::ogr::GeometryCollection::rend()
{
    return const_reverse_iterator(const_iterator(geometryCollection,-1));
}


#endif
