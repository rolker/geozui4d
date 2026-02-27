#ifndef GZ4D_RENDER_TESSELATION_H
#define GZ4D_RENDER_TESSELATION_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>
#include <gz4d/data/Polygon.h>
#include <GL/glu.h>

#ifdef BOOST_MSVC
    typedef void (__stdcall *_GLUfuncptr)();
#endif

namespace gz4d
{
    class Tessellation
    {
        std::shared_ptr<GLUtesselator> tess;
        void * data;
        public:
            Tessellation()
            :tess(gluNewTess(),gluDeleteTess),data(NULL)
            {
                gluTessCallback(tess.get(), GLU_TESS_VERTEX, (_GLUfuncptr)glVertex3dv);
                gluTessCallback(tess.get(), GLU_TESS_BEGIN, (_GLUfuncptr)glBegin);
                gluTessCallback(tess.get(), GLU_TESS_END, glEnd);
            }

            template <typename T> Tessellation(T *t)
            :tess(gluNewTess(),gluDeleteTess),data(t)
            {
                gluTessCallback(tess.get(), GLU_TESS_VERTEX_DATA,(_GLUfuncptr)T::tessellator_type::Vertex);
                gluTessCallback(tess.get(), GLU_TESS_BEGIN_DATA,(_GLUfuncptr)T::tessellator_type::Begin);
                gluTessCallback(tess.get(), GLU_TESS_END_DATA,(_GLUfuncptr)T::tessellator_type::End);
            }

            template <typename CT> void Contour(CT const &c)
            {
                gluTessBeginContour(tess.get());
                for(typename CT::const_iterator v = c.begin(); v != c.end(); ++v)
                        gluTessVertex(tess.get(), const_cast<double*>(&(v->front())), const_cast<double*>(&(v->front())));
                gluTessEndContour(tess.get());
            }

            template <typename CT> void Contours(CT const &cl)
            {
                for(typename CT::const_iterator c = cl.begin(); c != cl.end(); ++c)
                    Contour(*c);
            }

            template <typename T> void operator()(T const &cl);
            template <typename PT> void operator()(Polygon<PT> const &cl);
    };

    template <typename PT> inline void Tessellation::operator()(Polygon<PT> const &p)
    {
        gluTessBeginPolygon(tess.get(),data);
        Contour(p.getExteriorRing());
        Contours(p.getInteriorRings());
        gluTessEndPolygon(tess.get());
    }

    template <typename CT> inline void Tessellation::operator()(CT const &cl)
    {
        for(typename CT::const_iterator c = cl.begin(); c != cl.end(); ++c)
            (*this)(*c);
    }
}

#endif

