#ifndef GZ4D_RENDER_DRAWARRAYS_H
#define GZ4D_RENDER_DRAWARRAYS_H

#include <gz4d/gz4d.h>
#include <gz4d/glo/VertexPointer.h>

namespace gz4d
{
    template <typename T> void DrawArrays(std::vector<T> const &data, GLenum mode, GLint first = 0, std::size_t count = 0)
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glo::VertexPointer(data);
        if(count == 0)
            count = data.size() - first;
        glDrawArrays(mode,first,count);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

#endif
