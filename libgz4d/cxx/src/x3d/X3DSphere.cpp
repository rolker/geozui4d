#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DSphere.h>

#include <cmath>

gz4d::X3DSphere::X3DSphere()
{
    radius = 1.0;
    solid = true;
    typeName = "Sphere";
}

void gz4d::X3DSphere::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"solid",solid);
    LoadItem(x3d,"radius",radius);
    X3DGeometryNode::Load( x3d, root);
}

void gz4d::X3DSphere::Draw()
{
    if (preDraw())
        return;


    for(int lon = -180; lon < 180; lon += 5)
    {
        float rlon0 = Radians(float(lon));
        float rlon1 = Radians(float(lon+5));

        glBegin(GL_TRIANGLE_STRIP);
        for(int lat = 90; lat >= -90; lat -= 5)
        {
            float rlat = Radians(float(lat));

            float x0 = sin(rlon0)*cos(rlat);
            float x1 = sin(rlon1)*cos(rlat);
            float y = sin(rlat);
            float z0 = cos(rlon0)*cos(rlat);
            float z1 = cos(rlon1)*cos(rlat);

            glNormal3f(x1,y,z1);
            glTexCoord2f((lon+180+5)/360.0,(lat+90)/180.0);
            glVertex3f(x1*radius,y*radius,z1*radius);

            glNormal3f(x0,y,z0);
            glTexCoord2f((lon+180)/360.0,(lat+90)/180.0);
            glVertex3f(x0*radius,y*radius,z0*radius);
        }
        glEnd();
    }
}
