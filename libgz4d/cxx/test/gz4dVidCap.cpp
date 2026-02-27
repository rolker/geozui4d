#include<gz4d/v4l/Stream.h>
#include<gz4d/v4l/Capability.h>
#include <iostream>
#include <gz4d/vector/ios.h>
#include <fstream>
#include <gz4d/time/ios.h>
#include <gz4d/image/io.h>

int main(int argc, char * argv[])
{
    std::string d("/dev/video0");
    if (argc == 2)
        d = argv[1];
    gz4d::v4l::Device device(d);

    gz4d::v4l::Capability cap(device);

    std::cout << "Driver:\t" << cap.getDriver() << std::endl;
    std::cout << "Card:\t" << cap.getCard() << std::endl;
    std::cout << "Bus Info:\t" << cap.getBusInfo() << std::endl;
    std::cout << "Video Capture?:\t" << cap.isVideoCapture() << std::endl;
    std::cout << "Read/Write?:\t" << cap.isReadWrite() << std::endl;
    std::cout << "Streaming?:\t" << cap.isStreaming() << std::endl;
    std::cout << "Bounds:\t" << cap.getBounds().getMin() << "\t" << cap.getBounds().getSizes() << std::endl;
    std::cout << "Default Bounds:\t" << cap.getDefaultBounds().getMin() << "\t" << cap.getDefaultBounds().getSizes() << std::endl;
    std::cout << "Pixel Aspect:\t" << cap.getPixelAspect() << std::endl;
    for(std::vector<gz4d::v4l::Capability::Format>::const_iterator f = cap.getFormats().begin(); f != cap.getFormats().end(); ++f)
        std::cout << "\t" << f->description << "\tcompressed?: " << f->compressed << std::endl;
    
    gz4d::v4l::Stream s(device,cap.getFormats()[0].format,640,480);
    s.Start();
    gz4d::v4l::Stream::output_queue_type fq = s.NewOutQueue();
    for(int i = 0; i < 75; ++i)
    {
        gz4d::binary::Wrapper<gz4d::binary::Image::Frame> f = fq->pop();
        gz4d::ImageYUV8 img = gz4d::toImage(*f);
        std::stringstream fname;
        fname << "test_" << f->timestamp << ".jpg";
        img.Save(fname.str());
        std::cout << "*";
        std::cout.flush();
        if(i%25 == 0)
            std::cout << std::endl;
    }
    s.Stop();
    std::cout << std::endl;
}
