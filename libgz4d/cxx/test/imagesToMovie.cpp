// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/av/AVWriter.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include<gz4d/image/io.h>

int main(int argc, char* argv[])
{
    gz4d::AVWriter avw;
    if (argc < 5 || argc > 6)
    {
        std::cerr << "usage:\t imagesToMovie inPath inPrefix inExt [bps] out.avi" << std::endl;
        return 1;
    }

    boost::filesystem::path dir(argv[1]);
    std::string prefix(argv[2]);
    std::string ext(argv[3]);
    std::string out;
    unsigned int bps = 0;
    if(argc == 6)
    {
        bps = boost::lexical_cast<unsigned int>(argv[4]);
        out = argv[5];
    }
    else
        out = argv[4];


    bool init=false;

    boost::filesystem::directory_iterator dir_end;

    for(boost::filesystem::directory_iterator di(dir); di != dir_end; ++di)
    {
        std::string fn = di->path().filename().string();
        if(boost::algorithm::find_head(fn,prefix.size()) == prefix && boost::algorithm::find_tail(fn,ext.size())==ext)
        {
            std::cout << di->path() << std::endl;
            gz4d::ImageRGB8 i;
            i.Load(di->path().filename().string());
            gz4d::ImageYUV8::raster_type::Ptr yuvr(new gz4d::ImageYUV8::raster_type(i.GetRaster()));
            gz4d::ImageYUV8 iyuv(yuvr);
            if(!init)
            {
                avw.SetSize(iyuv.GetInfo().width,iyuv.GetInfo().height);
                if(bps)
                    avw.SetVideoBitrate(bps);
                avw.Open(out);
                init = true;
            }
            avw.AddFrame(iyuv);
        }
    }
    return 0;
}
