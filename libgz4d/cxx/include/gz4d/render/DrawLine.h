#ifndef GZ4D_RENDER_DRAWLINE_H
#define GZ4D_RENDER_DRAWLINE_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>

namespace gz4d
{
    void DrawLine(std::vector<Point<double> > const &data)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3,GL_DOUBLE,0,&data.front());
        glDrawArrays(GL_LINE_STRIP,0,data.size());
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

#endif
