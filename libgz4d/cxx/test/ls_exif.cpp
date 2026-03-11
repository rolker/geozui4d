// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/Exif.h>
#include <iostream>
#include <gz4d/util/Utilities.h>
#include <gz4d/vector/ios.h>

int main(int argc, char * argv[])
{
    gz4d::Exif e(argv[1]);

    float f = e.GetFloat("exif","FocalLength");
    std::cout << "Focal length: " << f << std::endl;

    uint16_t x = e.GetShort("exif","PixelXDimension");
    uint16_t y = e.GetShort("exif","PixelYDimension");
    std::cout << "Plane dimensions (pixels): " << x << ", " << y << std::endl;

    float xri = e.GetFloat("exif","FocalPlaneXResolution");
    float yri = e.GetFloat("exif","FocalPlaneYResolution");
    std::cout << "Focal plane resolution (pixels/inche): " << xri << ", " << yri << std::endl;

    float xrm = xri / 0.0254;
    float yrm = yri / 0.0254;
    std::cout << "Focal plane resolution (pixels/meter): " << xrm << ", " << yrm << std::endl;

    float ipx = x/xrm;
    float ipy = y/yrm;
    std::cout << "Image plane size (meters): " << ipx << ", " << ipy << std::endl;

    float xa = gz4d::Degrees(2.0*atan((ipx/2.0)/(f/1000.0)));
    float ya = gz4d::Degrees(2.0*atan((ipy/2.0)/(f/1000.0)));
    std::cout << "Field of view: " << xa << ", " << ya << std::endl;

    std::cout << e.GetFOV() << std::endl;
    return 0;
}
