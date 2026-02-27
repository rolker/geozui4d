#ifndef GZ4D_VIEWPORT_H
#define GZ4D_VIEWPORT_H

#include <gz4d/gz4d.h>
#include <gz4d/glo.h>
#include <gz4d/Box.h>
#include <gz4d/glo/Error.h>


namespace gz4d
{
    namespace glo
    {
        /**
        Describes the portion of a window or canvas in which to render.
        Values are in screen coordinates.
        \ingroup glo
        */
        class Viewport
        {
            union
            {
                GLint viewport[4];
                struct
                {
                    GLint x,y;
                    GLuint width,height;
                };
            };

            public:
                Viewport()
                :x(0),y(0),width(1),height(1)
                {
                }
                
                Viewport(unsigned int w, unsigned int h)
                :x(0),y(0),width(w),height(h)
                
                {
                }
                
                Viewport(int x, int y, unsigned int w, unsigned int h)
                :x(x),y(y),width(w),height(h)
                {
                }
                
                template <typename T> Viewport(Box<T,2> const &b);

                int GetX() const {return x;}
                int GetY() const {return y;}
                unsigned int GetWidth() const {return width;}
                unsigned int GetHeight() const {return height;}

                void Apply() const
                {
                    glViewport(x,y,width,height);
                    glo::Error();
                }
                
                void ApplyPick(double x, double y, double width, double height)
                {
                    gluPickMatrix(x, y, width, height, viewport);
                    glo::Error();
                }
                

                void Query()
                {
                    glGetIntegerv(GL_VIEWPORT,viewport);
                    glo::Error();
                }
                
                bool Contains(int x, int y) const
                {
                    return (xp >= x && xp <= x+width && yp >= y && yp <= y+height);
                }
                
                float AspectRatio() const { return width/static_cast<float>(height);}
        };

        template <typename T> inline Viewport::Viewport(Box<T,2> const &b)
        {
            x = b.getMin()[0];
            y = b.getMin()[1];
            width = b.getSizes()[0];
            height = b.getSizes()[1];
        }
    }
}

#endif
