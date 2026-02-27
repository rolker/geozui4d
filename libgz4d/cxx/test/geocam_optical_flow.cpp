#include <gz4d/geocam/Image.h>
#include <gz4d/geocam/Platform.h>
#include <gz4d/geocam/Map.h>
#include <gz4d/vector/ios.h>
#include <gz4d/cv/OpticalFlow.h>
#include <gz4d/image/io.h>
#include <gz4d/gdal/saveGeotiff.h>

int main(int argc, char * argv[])
{
    if(argc != 4 && argc != 5)
    {
        std::cout << "usage: geocam_optical_flow config.xml navfile image_dir image_pattern" << std::endl;
        exit(1);
    }

    gz4d::geo::Projection proj("+proj=stere +lat_0=90 +lat_ts=67.5 +lon_0=-155 +units=m");
    
    namespace gc = gz4d::geocam;
    
    gc::Platform::Ptr geocam = gc::Platform::create(argv[1]);
    
    std::cout << "loading nav " << argv[2] << "..." << std::endl;
    geocam->setNavigation(argv[2]);
    std::cout << "done" << std::endl;
    
    gc::ImageSet::Ptr is(new gc::ImageSet);
    is->add(argv[3],argv[4]);
    
    std::cout << is->size() << std::endl;
    
    gz4d::Vector<double,2> resolution(.5,.5);

    struct PrevData
    {
        gz4d::Time timestamp;
        gz4d::geocam::ProjectedImage pi;
        gz4d::geo::Image<gz4d::RGBA8> proj_img;
    };
    
    std::map<std::string,PrevData> prev_data;
    
    for(gz4d::TimeSequence< gc::Image >::iterator next = is->getImages().begin(); next != is->getImages().end(); ++next)
    {
        std::string cam = next->second.getCameraLabel();
        //if(prev_data[cam].timestamp.valid())
            //std::cout << boost::filesystem::path(prev_data[cam].pi.getFilename()).filename().string() << " -> " << boost::filesystem::path(next->second.getFilename()).filename().string() << std::endl;
        gz4d::geocam::ProjectedImage next_pi(next->second,*geocam,proj);
        gz4d::geo::Image<gz4d::RGBA8> next_proj_img = next_pi.generate(500,resolution);
        gz4d::Time next_timestamp = next->first;
        if(!next_proj_img.empty() && !prev_data[cam].proj_img.empty())
        {
            std::pair< gz4d::geo::Image< gz4d::RGBA8 >, gz4d::geo::Image< gz4d::RGBA8 > > overlap = prev_data[cam].proj_img.nonNullRectangleOverlap(next_proj_img);
            
            if(overlap.first.empty() || overlap.second.empty())
            {
                std::cerr << "not overlapping" << std::endl;
            }
            else
            {
                //overlap.first.Save((boost::filesystem::path("debug")/(boost::filesystem::path(prev_data[cam].pi.getFilename()).filename().string()+"_overlap1.tif")).string());
                //overlap.second.Save((boost::filesystem::path("debug")/(boost::filesystem::path(prev_data[cam].pi.getFilename()).filename().string()+"_overlap2.tif")).string());
                gz4d::ImageL8 i1 = gz4d::ImageL8(overlap.first.GetRaster());
                gz4d::ImageL8 i2 = gz4d::ImageL8(overlap.second.GetRaster());
                
                gz4d::cv::OpticalFlow of(i1,i2);
                
                double dt = (next_timestamp-prev_data[cam].timestamp).asSeconds();
                //std::cout << of.transform << std::endl;
                
                double x = of.transform.at<double>(0,2);
                double y = of.transform.at<double>(1,2);
                double a = of.transform.at<double>(0,0);
                double b = of.transform.at<double>(0,1);
                if(a == of.transform.at<double>(1,1))
                {
                    double s = sqrt(a*a+b*b);
                    double sa = b/s;
                
                    std::cout << next_timestamp << "," << cam << "," << resolution[0] << "," << resolution[1] << "," << dt << "," << x << "," << y << "," << s << "," << sa << std::endl;
                }
//                 if(*of.features.size)
//                 {
//                     gz4d::Vector< float, 2 > top = of.moved_features.at<gz4d::Vector<float,2> >(0) - of.features.at<gz4d::Vector<float,2> >(0);
//                     std::cout << of.transform << std::endl;
//                     
//                     std::cout << next_timestamp << "," << cam << "," << resolution[0] << "," << resolution[1] << "," << dt << "," << of.avg[0] << "," << of.avg[1]  << "," << top[0] << "," << top[1] << "," << of.errors.at<float>(0) << std::endl;
//                 }
            }
        }
        if(!next_proj_img.empty())
        {
            prev_data[cam].pi = next_pi;
            prev_data[cam].proj_img = next_proj_img;
            prev_data[cam].timestamp = next_timestamp;
        }
        
    }
    
    return 0;
    
}
