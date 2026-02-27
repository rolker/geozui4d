#include <gz4d/image/io.h>
#include <iostream>
#include <gz4d/vector/ios.h>

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
        std::cerr << "usage: image2txt image\n";
        exit(1);
    }
    gz4d::image::Info info = gz4d::image::info(argv[1]);
    std::cout << argv[1] << " " << info.width << " x " << info.height << ", " << info.format << std::endl;
    if(info.format == "L16")
    {
        gz4d::ImageL16 i;
        i.Load(argv[1]);
        for(int r = 0; r < info.height; ++r)
        {
            std::cout << r << ":\t";
            for(int c = 0; c < info.width; ++c)
            {
                std::cout << i.GetRaster().GetRow(r)[c];
                if(c < info.width-1)
                    std::cout << ", ";
            }
            std::cout << std::endl;
        }
    }
    
}
