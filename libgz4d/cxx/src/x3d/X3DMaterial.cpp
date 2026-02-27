#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DMaterial.h>
//#include <gz4d/util/Xml.h>

gz4d::X3DMaterial::X3DMaterial()
    :ambientIntensity(0.2),diffuseColor(0.8),emissiveColor(0.0),shininess(0.2),specularColor(0.0),transparency(0.0),needCalcs(true)
{
    typeName = "Material";
}

void gz4d::X3DMaterial::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d, "ambientIntensity", ambientIntensity);
    LoadItem(x3d, "diffuseColor", diffuseColor);
    LoadItem(x3d, "emissiveColor", emissiveColor);
    LoadItem(x3d, "shininess", shininess);
    LoadItem(x3d, "specularColor", specularColor);
    LoadItem(x3d, "transparency", transparency);
    needCalcs = true;
    X3DMaterialNode::Load( x3d, root);
}

void gz4d::X3DMaterial::Draw()
{
    if (preDraw())
        return;

    glDisable(GL_COLOR_MATERIAL);

    if (needCalcs)
    {
        gl_ambient = Vector<float,4>(diffuseColor *ambientIntensity,0);
        gl_ambient[3] = 1.0-transparency;

        gl_diffuse = Vector<float,4>(diffuseColor,0);
        gl_diffuse[3] = 1.0 - transparency;

        gl_emission = Vector<float,4>(emissiveColor,0);
        gl_emission[3] = 1.0 - transparency;

        gl_shininess = shininess * 128.0;

        gl_specular = Vector<float,4>(specularColor,0);
        gl_specular[3] = 1.0 - transparency;

        needCalcs = false;
    }

    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,&gl_ambient[0]);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,&gl_diffuse[0]);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,&gl_emission[0]);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&gl_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,&gl_specular[0]);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0);

}
