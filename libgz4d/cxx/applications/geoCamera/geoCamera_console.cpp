// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/v4l/Stream.h>
#include <gz4d/v4l/Capability.h>
#include <gz4d/Image.h>
#include <gz4d/ptu/Ptu.h>
#include <gz4d/gPhoto/Camera.h>
#include <gz4d/time.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <gz4d/util/Xml.h>


struct Setting
{
    std::string iso;
    std::string shutterspeed;
    std::string aperture;
    Setting(std::string const &iso, std::string const &ss, std::string const &ap)
        :iso(iso),shutterspeed(ss),aperture(ap)
    {
    }
};



double Meter(std::string const &iso, std::string const &aperture, std::string const &shutterspeed)
{
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
    }
    
    std::ostringstream ss_filename;
    
    gz4d::Time now = gz4d::time::Now();
    
    ss_filename << now << "_meter.jpg";
    
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
    return ret;
}



void SetupCamera(gz4d::Ptu &ptu)
{
    gz4d::xml::Doc doc;
    doc.ParseFile("/home/roland/.camera.xml");
    gz4d::xml::Node cam_node = doc.GetFirstNode("Camera");

    gz4d::xml::Attribute a = cam_node.GetFirstAttribute("iso");
    std::string iso = a.Value();
    a = cam_node.GetFirstAttribute("shutterspeed");
    std::string ss = a.Value();
    a = cam_node.GetFirstAttribute("aperture");
    std::string ap = a.Value();

    std::vector<Setting> settings;

    settings.push_back(Setting("1600","1/25","4"));
    settings.push_back(Setting("1600","1/50","4"));
    settings.push_back(Setting("1600","1/100","4"));
    settings.push_back(Setting("1600","1/100","5.6"));
    settings.push_back(Setting("800","1/100","5.6"));
    settings.push_back(Setting("400","1/100","5.6"));
    settings.push_back(Setting("400","1/100","8"));
    settings.push_back(Setting("400","1/100","11"));
    settings.push_back(Setting("400","1/100","16"));
    settings.push_back(Setting("400","1/100","22"));
    settings.push_back(Setting("400","1/200","22"));

    int current_setting;
    for(current_setting = 0; current_setting < settings.size(); ++current_setting)
        if(settings[current_setting].iso == iso && settings[current_setting].shutterspeed == ss && settings[current_setting].aperture == ap)
            break;

    if (current_setting >= settings.size() || current_setting < 0)
        current_setting = 0;

    bool ok = false;
    int tries = 0;
    while(!ok && tries < 25)
    {
        ptu.PanTilt(0.0,0.0);
        gz4d::Time::Sleep(0.5);
        double front = Meter(settings[current_setting].iso, settings[current_setting].aperture, settings[current_setting].shutterspeed);
        
        ptu.PanTilt(60.0,-10.0);
        gz4d::Time::Sleep(0.5);
        double left = Meter(settings[current_setting].iso, settings[current_setting].aperture, settings[current_setting].shutterspeed);
        
        ptu.PanTilt(-60.0,-10.0);
        gz4d::Time::Sleep(0.5);
        double right = Meter(settings[current_setting].iso, settings[current_setting].aperture, settings[current_setting].shutterspeed);

        std::cerr << "iso: " << settings[current_setting].iso << "\taperture: " << settings[current_setting].aperture << "\t shutterspeed: " <<  settings[current_setting].shutterspeed << "\tleft: " << left << "\tfront: " << front << "\tright: " << right << "\tavg: " << (front+left+right)/3.0 << std::endl;

        if( (front+left+right)/3.0 < 0.3)
            --current_setting;
        else if((front+left+right)/3.0 > 0.5)
            ++current_setting;
        else
            ok = true;

        if(current_setting < 0)
        {
            current_setting = 0;
            ok = true;
        }

        if(current_setting >= settings.size())
        {
            current_setting = settings.size()-1;
            ok = true;
        }
        ++tries;
    }

    std::ofstream s("/home/roland/.camera.xml");
    s << "<Camera iso=\"" << settings[current_setting].iso << "\" aperture=\"" << settings[current_setting].aperture << "\" shutterspeed=\"" << settings[current_setting].shutterspeed << "\" />" << std::endl;
}

std::string capture(gz4d::gphoto::Camera &camera, float pan, float tilt, boost::filesystem::path const &path)
{
    gz4d::Time now = gz4d::time::Now();

    std::ostringstream ss_c,ss_w,ss_ptu;

    ss_c << now << "_canon.jpg";
    ss_w << now << "_webcam.jpg";
    ss_ptu << now << "_ptu.txt";

    boost::filesystem::path cfn = path/ss_c.str();
    camera.Capture(cfn);

    //gz4d::ImageRGB8 webcam_image = stream->Read();
    //boost::filesystem::path wfn = path/ss_w.str();
    //webcam_image.Save(wfn);

    boost::filesystem::path pfn = path/ss_ptu.str();
    std::ofstream ptu_data(pfn.string().c_str());
    ptu_data << "pan " << pan << "\ntilt " << tilt << "\n" << now << std::endl;

    return ss_c.str();
}

int main(int argc, char * argv[])
{
    if (argc != 7)
    {
        std::cerr << "usage: geoCamera_console pan_max, pan_step, tilt_max, tilt_min, tilt_step, repititions" << std::cerr;
        exit(1);
    }

    double pan_max = atof(argv[1]);
    double pan_step = atof(argv[2]);
    double tilt_start = atof(argv[3]);
    double tilt_stop = atof(argv[4]);
    double tilt_step = atof(argv[5]);
    int reps = atoi(argv[6]);

    gz4d::Ptu ptu("/dev/ttyUSB0");
    ptu.VelocityMode(false);
    ptu.Reset();

    ptu.PanAcceleration(400.0);
    ptu.TiltAcceleration(400.0);
    ptu.PanSpeed(180.0);
    ptu.TiltSpeed(180.0);

    SetupCamera(ptu);

    gz4d::gphoto::Camera camera;

    //gz4d::v4l::Device device("/dev/video0");

    //gz4d::v4l::Capability cap(device);
    //std::shared_ptr<gz4d::v4l::Stream> stream(new gz4d::v4l::Stream(device,cap.getFormats()[0].format,800,600));
    //stream->Start();

    //while(true)
    for(int i = 0; i <reps; ++i)
    {
        std::stringstream now_ss;
        now_ss << gz4d::time::Now();
        boost::filesystem::path path(now_ss.str());
        boost::filesystem::create_directory(path);


        ptu.PanTilt(0.0,0.0);
        gz4d::Time::Sleep(0.5);
        std::cerr << "Captured " << capture(camera, 0.0, 0.0, path) << std::endl;

        for(float pan=pan_max; pan >= 0.0; pan -= pan_step)
            for(float tilt = tilt_start; tilt <= tilt_stop;  tilt += tilt_step)
            {
                std::cerr << "Moving to " << pan << ", " << tilt << std::endl;
                ptu.PanTilt(pan,tilt);
                gz4d::Time::Sleep(0.5);
                std::cerr << "Captured " << capture(camera, pan, tilt, path) << std::endl;
                if (pan > 0.0)
                {
                    std::cerr << "Moving to " << -pan << ", " << tilt << std::endl;
                    ptu.PanTilt(-pan,tilt);
                    gz4d::Time::Sleep(0.5);
                    std::cerr << "Captured " << capture(camera, -pan, tilt, path) << std::endl;
                }
            }
        if(i == reps-1)
        {
            ptu.PanTilt(0.0,0.0);
            gz4d::Time::Sleep(0.5);
            std::cerr << "Captured " << capture(camera, 0.0, 0.0, path) << std::endl;
        }
    }

    return 0;
}
