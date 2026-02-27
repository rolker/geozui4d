#include <gz4d/av.h>
#include <gz4d/image/io.h>

int main(int argc, char* argv[])
{
    gz4d::AVWriter avw;
    if (argc < 3)
        return 1;

    gz4d::ImageRGB8 i;
    i.Load(argv[2]);
    gz4d::ImageYUV8::raster_type::Ptr yuvr(new gz4d::ImageYUV8::raster_type(i.GetRaster()));
    gz4d::ImageYUV8 iyuv(yuvr);
    avw.SetSize(i.GetInfo().width,i.GetInfo().height);
    avw.Open(argv[1]);
    for(int j = 0; j < 60; j++)
    {
        avw.AddFrame(iyuv);
    }
    return 0;
}
