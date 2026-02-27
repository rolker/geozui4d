#define GZ4D_INTERNAL
#include <gz4d/x3d/X3DGeoLocation.h>
#include <gz4d/x3d/X3DGeoOrigin.h>

gz4d::X3DGeoLocation::X3DGeoLocation()
{
    typeName = "GeoLocation";
}

void gz4d::X3DGeoLocation::Load(const boost::property_tree::ptree &x3d, X3D* root)
{
    LoadItem(x3d,"geoCoords",geoCoords);
    LoadArray( x3d, "geoSystem", geoSystem);

    boost::property_tree::ptree::const_assoc_iterator n = x3d.find("GeoOrigin");
    if(n != x3d.not_found())
        geoOrigin = SFNode(new X3DGeoOrigin(),n->second, root);

    X3DGroupingNode::Load(x3d, root);
}

void gz4d::X3DGeoLocation::Draw()
{
    if (preDraw())
        return;

    glPushMatrix();


    X3DGroupingNode::Draw();

    glPopMatrix();
}

