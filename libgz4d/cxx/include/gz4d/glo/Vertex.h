#ifndef GZ4D_GLO_VERTEX_H
#define GZ4D_GLO_VERTEX_H

#include <gz4d/glo.h>
#include <gz4d/Vector.h>


namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        template <typename T, std::size_t N> void Vertex(Vector<T,N> const &v);

        template <> inline void Vertex(Vector<float,2> const &v)
        {
            glVertex2fv(&v.front());
        }

        template <> inline void Vertex(Vector<float,3> const &v)
        {
            glVertex3fv(&v.front());
        }

        template <> inline void Vertex(Vector<double,2> const &v)
        {
            glVertex2dv(&v.front());
        }

        template <> inline void Vertex(Vector<double,3> const &v)
        {
            glVertex3dv(&v.front());
        }
    }
}

#endif

