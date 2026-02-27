#include<gz4d/gdal/saveGeotiff.h>
#include<gz4d/vector/operations.h>
#include<gz4d/vector/ios.h>
#include <gz4d/cv.h>
#include <gz4d/image/io.h>
#include <gz4d/util/Histogram.h>
#include <boost/foreach.hpp>

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cerr << "usage: gt_optical_flow geotiff1.tif geotiff2.tif" << std::endl;
    }
    
    
    gz4d::geo::Image<gz4d::RGBA8> img1 = gz4d::gdal::loadGeotiff(argv[1]);
    std::cout << argv[1] << " bbox: " << img1.getBoundingBox().getMin() << " - " << img1.getBoundingBox().getMax() << std::endl;
    std::cout << "pixel sizes: " << img1.getPixelSizes() << std::endl;
    
    
    gz4d::geo::Image<gz4d::RGBA8> img1rect = img1.nonNullRectangle();
    std::cout << "rectangle bbox: " << img1rect.getBoundingBox().getMin() << " - " << img1rect.getBoundingBox().getMax() << std::endl;
    gz4d::gdal::saveGeotiff("test_rec1.tif",img1rect);
    
    gz4d::geo::Image<gz4d::RGBA8> img2 = gz4d::gdal::loadGeotiff(argv[2]);
    std::cout << argv[2] << " bbox: " << img2.getBoundingBox().getMin() << " - " << img2.getBoundingBox().getMax() << std::endl;
    std::cout << "pixel sizes: " << img2.getPixelSizes() << std::endl;

    gz4d::geo::Image<gz4d::RGBA8> img2rect = img2.nonNullRectangle();
    std::cout << "rectangle bbox: " << img2rect.getBoundingBox().getMin() << " - " << img2rect.getBoundingBox().getMax() << std::endl;
    gz4d::gdal::saveGeotiff("test_rec2.tif",img2rect);
    
    std::cout << "displacement: " << img2rect.getBoundingBox().getMin()-img1rect.getBoundingBox().getMin() << std::endl;
    
    std::pair< gz4d::geo::Image< gz4d::RGBA8 >, gz4d::geo::Image< gz4d::RGBA8 > > overlap = img1rect.overlap(img2rect);
    
    std::cout << "overlap bbox: " << overlap.first.getBoundingBox().getMin() << " - " << overlap.first.getBoundingBox().getMax() << std::endl;
    std::cout << "overlap bbox: " << overlap.second.getBoundingBox().getMin() << " - " << overlap.second.getBoundingBox().getMax() << std::endl;
    
    gz4d::ImageL32 i1 = gz4d::ImageL32(overlap.first.GetRaster());
    i1.Save("test_1.tif");
                    
    gz4d::ImageL32 i2 = gz4d::ImageL32(overlap.second.GetRaster());
    i2.Save("test_2.tif");
                    
    gz4d::cv::Mat<gz4d::Pixel<gz4d::L32> > cvimg1(i1.GetRasterPtr());
    gz4d::cv::Mat<gz4d::Pixel<gz4d::L32> > cvimg2(i2.GetRasterPtr());
    
    gz4d::cv::Point2d flow = cv::phaseCorrelate(cvimg1.mat,cvimg2.mat);
    gz4d::Vector<double,2> ret = flow;
    
    std::cout << "flow: " << ret << std::endl;
    
    return 0;
}
