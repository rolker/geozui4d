#ifndef GZ4D_X3DMATERIAL_H
#define GZ4D_X3DMATERIAL_H

#include "X3DMaterialNode.h"

namespace gz4d
{
    class X3DMaterial : public X3DMaterialNode
    {
        public:
            X3DMaterial();
            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
        protected:
            SFFloat ambientIntensity;
            SFColor diffuseColor;
            SFColor emissiveColor;
            SFFloat shininess;
            SFColor specularColor;
            SFFloat transparency;

            bool needCalcs;
            Vector<float,4> gl_ambient;
            Vector<float,4> gl_diffuse;
            Vector<float,4> gl_emission;
            Vector<float,4> gl_specular;
            GLfloat gl_shininess;
    };
};

#endif
