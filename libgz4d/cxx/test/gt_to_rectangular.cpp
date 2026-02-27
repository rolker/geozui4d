#include<gz4d/gdal/saveGeotiff.h>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cerr << "usage: gt_to_rectangular in_geotiff.tif out_geotiff.tif" << std::endl;
        return 1;
    }
    
    gz4d::geo::Image<gz4d::RGBA8> in_img = gz4d::gdal::loadGeotiff(argv[1]);
    
    gz4d::geo::Image< gz4d::RGBA8 > out_img = in_img.nonNullRectangle(gz4d::Pixel<gz4d::RGBA8>(gz4d::Color(0,0,0,0)));
    gz4d::gdal::saveGeotiff(argv[2],out_img);
    
}
    