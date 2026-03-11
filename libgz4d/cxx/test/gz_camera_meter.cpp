// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/gPhoto/Camera.h>
#include <gz4d/time.h>
#include <gz4d/Image.h>
#include <fstream>
#include <gz4d/vector/ios.h>
#include <gz4d/time/ios.h>
#include <gz4d/image/io.h>

double Meter(std::ofstream &info, std::string const &iso, std::string const &aperture, std::string const &shutterspeed)
{

    std::cout << "\niso\t" << iso << std::endl;
    std::cout << "aperture\t" << aperture << std::endl;
    std::cout << "shutterspeed\t" << shutterspeed << std::endl;

    {
        gz4d::gphoto::Camera camera;

        gz4d::gphoto::Config cfg = camera.GetConfig();
        cfg[std::string("imgsettings")][std::string("iso")].Set(iso);
        camera.SetConfig(cfg);

        cfg = camera.GetConfig();
        cfg[std::string("capturesettings")][std::string("aperture")].Set(aperture);
        camera.SetConfig(cfg);

        cfg = camera.GetConfig();
        cfg[std::string("capturesettings")][std::string("shutterspeed")].Set(shutterspeed);
        camera.SetConfig(cfg);

        cfg = camera.GetConfig();

        std::cout << "iso\t" << cfg[std::string("imgsettings")][std::string("iso")].Get() << std::endl;
        std::cout << "aperture\t" << cfg[std::string("capturesettings")][std::string("aperture")].Get() << std::endl;
        std::cout << "shutterspeed\t" << cfg[std::string("capturesettings")][std::string("shutterspeed")].Get() << std::endl;
    }

    std::ostringstream ss_filename;

    gz4d::Time now = gz4d::time::Now();

    ss_filename << now << "_meter.jpg";

    std::cout << ss_filename.str() << std::endl;

    info << ss_filename.str() << ",";
    info << iso << ",";
    info << aperture << ",";
    info << shutterspeed << ",";

    {
        gz4d::gphoto::Camera camera;
        camera.Capture(ss_filename.str());
    }

    gz4d::ImageRGB8 image;
    image.Load(ss_filename.str());

    gz4d::Vector<double,3> color_accumulator;
    for(std::size_t i = 0; i < image.GetRaster().GetRows(); ++i)
        for(std::size_t j = 0; j < image.GetRaster().GetColumns(); ++j)
        {
            gz4d::Color c = image.GetRaster().GetRow(i)[j];
            color_accumulator[0] += c[0];
            color_accumulator[1] += c[1];
            color_accumulator[2] += c[2];
        }

    color_accumulator /= double(image.GetRaster().GetRows()*image.GetRaster().GetColumns());

    double ret = (color_accumulator[0]+color_accumulator[1]+color_accumulator[2])/3.0;
    
    info << color_accumulator[0] << "," << color_accumulator[1] << "," << color_accumulator[2] << "," << ret << std::endl;

    std::cout << color_accumulator << "\t" << ret << std::endl;

    return ret;
}

int main(int argc, char * argv[])
{
    std::ostringstream ss_info_filename;
    gz4d::Time now = gz4d::time::Now();
    ss_info_filename << now << "_meter.csv";
    std::ofstream info(ss_info_filename.str().c_str());

    info << "image,iso,aperture,shutterspeed,r,g,b,average" << std::endl;

    std::vector<std::string> isos;
    //isos.push_back("200");
    //isos.push_back("250");
    //isos.push_back("320");
    isos.push_back("400");
    //isos.push_back("500");
    //isos.push_back("640");
    isos.push_back("800");
    //isos.push_back("1000");
    //isos.push_back("1250");
    //isos.push_back("1600");

    std::vector<std::string> aps;
    //aps.push_back("3.5");
    //aps.push_back("5");
    //aps.push_back("5.6");
    //aps.push_back("6.3");
    //aps.push_back("7.1");
    aps.push_back("8");
    //aps.push_back("9");
    //aps.push_back("10");
    aps.push_back("11");
    //aps.push_back("13");
    //aps.push_back("14");
    aps.push_back("16");
    //aps.push_back("18");
    //aps.push_back("20");
    aps.push_back("22");

    std::vector<std::string> ss;
    //ss.push_back("1/10");
    //ss.push_back("1/25");
    //ss.push_back("1/50");
    ss.push_back("1/100");
    //ss.push_back("1/125");
    ss.push_back("1/160");
    //ss.push_back("1/200");
    ss.push_back("1/500");
    //ss.push_back("1/1000");

/*    for(std::vector<std::string>::iterator i = isos.begin(); i != isos.end(); ++i)
        for( std::vector<std::string>::iterator a = aps.begin(); a != aps.end(); ++a)
            for( std::vector<std::string>::iterator s = ss.begin(); s != ss.end(); ++s)
                Meter(info, *i, *a, *s);*/
    double m = Meter(info, "400", "8", "1/100");
    m = Meter(info, "400", "8", "1/50");
    m = Meter(info, "400", "8", "1/25");
    m = Meter(info, "400", "8", "1/10");
    m = Meter(info, "400", "8", "1/4");
    m = Meter(info, "400", "8", "1/200");
    m = Meter(info, "400", "8", "1/400");
    m = Meter(info, "400", "8", "1/800");
    m = Meter(info, "400", "8", "1/1000");
    m = Meter(info, "400", "8", "1/1600");
}
