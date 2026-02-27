#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DGeoOrigin.h>

gz4d::X3DGeoOrigin::X3DGeoOrigin()
{
    typeName = "GeoOrigin";
}

void gz4d::X3DGeoOrigin::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadArray( x3d, "geoSystem", geoSystem);
    LoadItem( x3d, "rotateYUp", rotateYUp);

    X3DNode::Load(x3d, root);
}

