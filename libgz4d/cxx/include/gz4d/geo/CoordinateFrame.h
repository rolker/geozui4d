#ifndef GZ4D_GEO_COORDINATEFRAME_H
#define GZ4D_GEO_COORDINATEFRAME_H

#include <gz4d/gz4d.h>

namespace gz4d
{
    namespace geo
    {

        struct LatLon
        {
            enum Coordinates
            {
                Latitude = 0, Longitude = 1, Height = 2
            };
        };

        struct LonLat
        {
            enum Coordinates
            {
                Longitude = 0, Latitude = 1, Height = 2
            };
        };

        struct XYZ
        {
            enum Coordinates
            {
                X = 0, Y = 1, Z = 2
            };
        };
    }
}

#endif
