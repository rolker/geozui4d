#ifndef GZ4D_X3DGEOLOCATION_H
#define GZ4D_X3DGEOLOCATION_H

#include "X3DGroupingNode.h"

namespace gz4d
{
    class GZ4DEXPORT X3DGeoLocation: public X3DGroupingNode
    {
        public:
            X3DGeoLocation();

            void Load(const boost::property_tree::ptree &x3d, X3D* root);
            void Draw();
            MFString const &getGeoSystem() const {return geoSystem;}
            SFVec3d const &getGeoCoords() const {return geoCoords;}
                    
        protected:
            SFVec3d geoCoords;
            SFNode geoOrigin;
            MFString geoSystem;
    };
};

#endif
