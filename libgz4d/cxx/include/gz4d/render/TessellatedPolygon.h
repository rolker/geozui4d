// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_RENDER_TESSELLATEDPOLYGON_H
#define GZ4D_RENDER_TESSELLATEDPOLYGON_H


#include <gz4d/gz4d.h>
#include <gz4d/data/Polygon.h>
#include <gz4d/render/Tessellation.h>
#include <gz4d/render/DrawArrays.h>
#include <gz4d/geo/LocalENU.h>

namespace gz4d
{
    template <typename T> class PolygonTessellator;

    template <typename T = Point<double> > class TessellatedPolygon
    {
        friend class PolygonTessellator<T>;

        std::map<GLenum,std::vector<std::vector<T> > > data;

        public:
            TessellatedPolygon(){}
            TessellatedPolygon(TessellatedPolygon<T> const &tp):data(tp.data){}

            template <typename OT> TessellatedPolygon(TessellatedPolygon<OT> const &tp)
            {
                for(typename std::map<GLenum,std::vector<std::vector<OT> > >::const_iterator m = tp.getData().begin(); m != tp.getData().end(); ++m)
                    for(typename std::vector<std::vector<OT> >::const_iterator v = m->second.begin(); v != m->second.end(); ++v)
                    {
                        data[m->first].push_back(std::vector<T>());
                        for(typename std::vector<OT>::const_iterator p = v->begin(); p != v->end(); ++p)
                            data[m->first].back().push_back(T(*p));
                    }
            }

            void Draw() const
            {
                for(typename std::map<GLenum,std::vector<std::vector<T> > >::const_iterator m = data.begin(); m != data.end(); ++m)
                    for(typename std::vector<std::vector<T> >::const_iterator v = m->second.begin(); v != m->second.end(); ++v)
                        DrawArrays(*v,m->first);
            }

            template <typename ET> void Draw(geo::LocalENU<ET> const &georef) const
            {
                for(typename std::map<GLenum,std::vector<std::vector<T> > >::const_iterator m = data.begin(); m != data.end(); ++m)
                    for(typename std::vector<std::vector<T> >::const_iterator v = m->second.begin(); v != m->second.end(); ++v)
                        DrawArrays(georef(*v),m->first);
            }

            std::map<GLenum,std::vector<std::vector<T> > > const &getData() const
            {
                return data;
            }
    };


    template <typename T = Point<double> > class PolygonTessellator
    {
        friend class Tessellation;

        struct Data
        {
            typedef PolygonTessellator<T> tessellator_type;
            GLenum mode;
            TessellatedPolygon<T>* data;
        };

        static void Begin(GLenum m, void *d)
        {
            ((Data*)(d))->mode = m;
            ((Data*)(d))->data->data[m].push_back(std::vector<T>());
        }

        static void End(void *d)
        {
        }

        static void Vertex(GLdouble * v, void *d)
        {
            ((Data*)(d))->data->data[((Data*)(d))->mode].back().push_back(T(gz4d::Point<double>(v[0],v[1],v[2])));
        }

        public:
            template <typename PT> TessellatedPolygon<T> const &operator()(PT const &p, TessellatedPolygon<T> &tp)
            {
                Data data;
                data.data = &tp;
                Tessellation tess(&data);
                tess(p);
                return tp;
            }

            template <typename PT> TessellatedPolygon<T> operator()(PT const &p)
            {
                TessellatedPolygon<T> data;
                return operator()(p,data);
            }
    };
}

#endif
