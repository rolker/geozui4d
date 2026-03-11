// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_GLO_MATERIAL_H
#define GZ4D_GLO_MATERIAL_H

#include <gz4d/glo.h>
#include <gz4d/Color.h>


namespace gz4d
{
    namespace glo
    {
        /// \ingroup glo
        class Material
        {
            GLenum face;
            public:
                Material(GLenum face=GL_FRONT)
                :face(face)
                {
                }

                void Ambient(gz4d::Color const &c)
                {
                    glMaterialfv(face,GL_AMBIENT,&c.front());
                }
                
                void Diffuse(gz4d::Color const &c)
                {
                    glMaterialfv(face,GL_DIFFUSE,&c.front());
                }
                
                void AmbientAndDiffuse(gz4d::Color const &c)
                {
                    glMaterialfv(face,GL_AMBIENT_AND_DIFFUSE,&c.front());
                }
                

                void Specular(gz4d::Color const &c)
                {
                    glMaterialfv(face,GL_SPECULAR,&c.front());
                }
                
                void Shininess(float s)
                {
                    glMaterialf(face,GL_SHININESS,s);
                }
                
                void Emission(gz4d::Color const &c)
                {
                    glMaterialfv(face,GL_EMISSION,&c.front());
                }
        };
    }
}

#endif
