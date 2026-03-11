// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/geocam/Map.h>
#include <gz4d/gdal/saveGeotiff.h>


int main(int argc, char * argv[])
{
    if(argc != 10)
    {
        std::cout << "usage: geocam_animator config.xml navfile image_dir image_pattern range resolution width height outfile" << std::endl;
        exit(1);
    }
    
    //gz4d::geo::Projection proj("+proj=stere +lat_0=90 +lat_ts=67.5 +lon_0=-155 +units=m");
    gz4d::geo::Projection proj;
    
    gz4d::geocam::Platform::Ptr geocam = gz4d::geocam::Platform::create(argv[1]);
    
    std::cout << "loading nav " << argv[2] << "..." << std::endl;
    geocam->setNavigation(argv[2]);
    std::cout << "done" << std::endl;
    
    gz4d::geocam::ImageSets is;
    is["default"] = gz4d::geocam::ImageSet::Ptr(new gz4d::geocam::ImageSet);
    is["default"]->add(argv[3],argv[4]);
    
    gz4d::geocam::Map m(geocam,is,proj);
    m.setResolution(atof(argv[6]));
    m.setRange(atof(argv[5]));
    
    m.animate(argv[9],geocam->getNavigation(),atoi(argv[7]),atoi(argv[8]));
    
    return 0;
}
