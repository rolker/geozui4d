// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/axis/ocr.h>
#include <gz4d/image/io.h>
#include <filesystem>
#include <gz4d/time/ios.h>

int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: axis_ocr img1.jpg [img2.jpg ...]\n";
        exit(1);
    }
    
    gz4d::axis::OCR ocr;
    
    for(int i = 1; i < argc; ++i)
    {
        gz4d::ImageL8 img;
        img.Load(argv[i]);
        std::string fname = std::filesystem::path(argv[i]).filename().string();
        std::cout << fname << ",";
        std::string tstring = ocr(img);
        std::cout << tstring << ",";
        std::istringstream ss(tstring);
        gz4d::Time t;
        try
        {
            ss >> t;
        }
        catch(...)
        {
        }
        gz4d::Time ftime = boost::lexical_cast<gz4d::Time>(fname.substr(0,fname.find('_')));
        double diff = (ftime - t).asSeconds();
        std::cout << diff  << ",";
        std::cout << ftime <<  "," << t << std::endl;
        if(fabs(diff) > 1.0)
            exit(1);
    }
}
