#include<gz4d/v4l/Stream.h>
#include<gz4d/v4l/Capability.h>
#include <iostream>
#include <gz4d/vector/ios.h>
#include <fstream>
#include <gz4d/socket/UDPSocket.h>
#include <gz4d/socket/TCPSocket.h>


int main(int argc, char * argv[])
{

    if (argc != 5 && argc != 6)
    {
        std::cerr << "usage: gzVideoSender device udp_port width height [address]\n\taddress defaults to BROADCAST" << std::endl;
        exit(1);
    }
    std::string address = "BROADCAST";
    if (argc == 6)
        address = argv[5];

    gz4d::v4l::Device device(argv[1],false);

    int port = atoi(argv[2]);

    //gz4d::UDPSocket udp;
    gz4d::TCPSocket udp;
    
    if (address == "BROADCAST")
        udp.SetOption(gz4d::Socket::Broadcast,true);
    udp.Connect(gz4d::IPSocket::Address(address,port));

    int w = atoi(argv[3]);
    int h = atoi(argv[4]);

    gz4d::v4l::Capability cap(device);

    std::cout << "Driver:\t" << cap.getDriver() << std::endl;
    std::cout << "Card:\t" << cap.getCard() << std::endl;
    std::cout << "Bus Info:\t" << cap.getBusInfo() << std::endl;
    std::cout << "Video Capture?:\t" << cap.isVideoCapture() << std::endl;
    std::cout << "Read/Write?:\t" << cap.isReadWrite() << std::endl;
    std::cout << "Streaming?:\t" << cap.isStreaming() << std::endl;
    //std::cout << "Bounds:\t" << cap.getBounds().getMin() << "\t" << cap.getBounds().getSizes() << std::endl;
    //std::cout << "Default Bounds:\t" << cap.getDefaultBounds().getMin() << "\t" << cap.getDefaultBounds().getSizes() << std::endl;
    //std::cout << "Pixel Aspect:\t" << cap.getPixelAspect() << std::endl;
    std::vector<gz4d::v4l::Capability::Format>::const_iterator m =  cap.getFormats().end();
    for(std::vector<gz4d::v4l::Capability::Format>::const_iterator f = cap.getFormats().begin(); f != cap.getFormats().end(); ++f)
    {
        std::cout << "\t" << f->description << "\tcompressed?: " << f->compressed << std::endl;
        if (f->description == "MJPEG")
            m = f;
    }
    m = cap.getFormats().begin();

    gz4d::v4l::Stream s(device,m->format,w,h);
    
    boost::rational<uint32_t> rfp(1,10);

    boost::rational<uint32_t> sfp = s.framePeriod(rfp);
    std::cout << "frame period: " << sfp << std::endl;

    s.Start(false);
    while(true)
    {
        //std::cerr << "trying..." << std::endl;
        if(!s.SendTo(udp))
        {
            //std::cerr << "sleeping" << std::endl;
            gz4d::Sleep(gz4d::seconds(0.01));
        }
    }
    s.Stop();
    std::cout << std::endl;
}
