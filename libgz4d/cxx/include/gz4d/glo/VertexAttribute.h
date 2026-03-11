// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_VERTEXATTRIBUTE_H
#define GZ4D_GLO_VERTEXATTRIBUTE_H

#include <gz4d/glo.h>
#include <vector>
#include <gz4d/Vector.h>

namespace gz4d
{
    namespace glo
    {
        class Program;
        
        class VertexAttribute
        {
            friend class Program;
            
            int location;
            VertexAttribute(int location):location(location){}
        public:
            VertexAttribute():location(-1){}

            void operator()(float v) const
            {
                glVertexAttrib1f(location,v);
            }

            void operator()(double v) const
            {
                glVertexAttrib1d(location,v);
            }


            void operator()(std::vector<float> const &v) const
            {
                switch (v.size())
                {
                case 1:
                    glVertexAttrib1fv(location,&v.front());
                    break;
                case 2:
                    glVertexAttrib2fv(location,&v.front());
                    break;
                case 3:
                    glVertexAttrib3fv(location,&v.front());
                    break;
                case 4:
                    glVertexAttrib4fv(location,&v.front());
                    break;
                }
            }

            void operator()(Vector<float,2> const &v) const
            {
                glVertexAttrib2f(location,v[0],v[1]);
            }

            operator bool()
            {
                return location != -1;
            }

        };
    }
}

#endif
