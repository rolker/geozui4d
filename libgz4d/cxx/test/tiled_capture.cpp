// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/v4l/Tiler.h>
#include <gz4d/v4l/Capability.h>
#include <gz4d/av.h>
#include <boost/filesystem.hpp>

int main(int argc, char*argv[])
{
    namespace gv = gz4d::v4l;
    gz4d::AVWriter out;
    gv::Tiler t;
    //t.outerBorder(2);
    //t.innerBorder(4);
    //t.width(1280);
    //t.height(720);
    gz4d::time::Duration fp(1.0);
    //boost::rational<uint32_t> rfp(1,15);
    boost::rational<uint32_t> rfp(1,30);
    //boost::rational<uint32_t> rfp(1,20);

    std::string vbase = "/dev/video";
    for(int i = 0; i < 10; ++i)
    {
        std::stringstream vs;
        vs << vbase << i;
         
        if(exists(boost::filesystem::path(vs.str())))
        {
            gv::Device dev(vs.str());
            gv::Capability cap(dev);

            gv::Stream::Ptr s(new gv::Stream(dev,cap.getFormats()[0].format,320,240));
            //gv::Stream::Ptr s(new gv::Stream(dev,cap.getFormats()[0].format,640,480));
            boost::rational<uint32_t> sfp = s->framePeriod(rfp);
            std::cerr << vs.str() << "\tframe period: " << sfp << std::endl;
            fp = std::min(fp,gz4d::time::Duration(boost::rational_cast<double>(sfp)));
            s->Start();
            t.add(s,0,t.columns());
        }
    }
    std::cerr << "Frame period: " << fp << std::endl;
    t.framePeriod(fp);
    if(!t.empty())
    {
        out.SetFPS(1/fp.asSeconds());
        out.SetSize(t.width(),t.height());
        out.Open("tile_capture_test.avi");

        gz4d::Time lastReportTime = gz4d::time::Now();
        for(int j = 0; j*fp < gz4d::seconds(20); ++j)
        {
            gz4d::binary::Wrapper<gz4d::binary::Image::Frame> frame = t.Read();
            out.AddFrame(*frame);
            gz4d::Time now = gz4d::time::Now();
            if(now - lastReportTime > gz4d::seconds(1.0))
            {
                std::cout << j << "\tsys time: " << now << "\t frame time: " << frame->timestamp << "\t diff: " << now-frame->timestamp << std::endl;
                lastReportTime = now;
            }
            //std::cout << j << "\t" << frame.timestamp << std::endl;
        }
        std::cout << "done..." << std::endl;
    }

    return 0;
}
