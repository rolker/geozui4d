// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/gdal/saveGeotiff.h>
#include<gz4d/vector/operations.h>
#include<gz4d/vector/ios.h>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cerr << "usage: gt_corrolate geotiff1.tif geotiff2.tif" << std::endl;
    }
    
    gz4d::geo::Image<gz4d::RGBA8> img1 = gz4d::gdal::loadGeotiff(argv[1]);
    std::cout << argv[1] << " bbox: " << img1.getBoundingBox().getMin() << " - " << img1.getBoundingBox().getMax() << std::endl;
    std::cout << "pixel sizes: " << img1.getPixelSizes() << std::endl;
    
    gz4d::geo::Image<gz4d::RGBA8> img2 = gz4d::gdal::loadGeotiff(argv[2]);
    std::cout << argv[2] << " bbox: " << img2.getBoundingBox().getMin() << " - " << img2.getBoundingBox().getMax() << std::endl;
    std::cout << "pixel sizes: " << img2.getPixelSizes() << std::endl;
    
    std::cout << "displacement: " << img2.getBoundingBox().getMin()-img1.getBoundingBox().getMin() << std::endl;
    
    gz4d::Vector< double, 2 > offset = (img2.getBoundingBox().getMin()-img1.getBoundingBox().getMin())/img1.getPixelSizes();
    std::cout << "offset (pixels): " << offset << std::endl;

    for(int i = -2; i <= 2; ++i)
        for(int j = -2; j <= 2; ++j)
        {
            std::cout << i << "," << j << "\t";
            std::cout << img1.GetRaster().correlation(img2.GetRaster(),offset+gz4d::Vector<int,2>(i,j)) << std::endl;
        }
    
    return 0;
}