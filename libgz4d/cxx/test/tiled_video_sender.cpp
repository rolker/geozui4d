#include <gz4d/v4l/Tiler.h>
#include <gz4d/v4l/Capability.h>
#include <gz4d/socket/UDPSocket.h>


int main(int argc, char*argv[])
{
    namespace gv = gz4d::v4l;
    gv::Tiler t;
    //t.outerBorder(2);
    //t.innerBorder(4);
    //t.width(1280);
    //t.height(720);
    gz4d::time::Duration fp(1.0);
    //boost::rational<uint32_t> rfp(1,15);
    boost::rational<uint32_t> rfp(1,30);
    //boost::rational<uint32_t> rfp(1,20);

    int port = atoi(argv[2]);
    
    gz4d::UDPSocket udp;
    if(std::string(argv[1]) == "BROADCAST")
        udp.SetOption(gz4d::Socket::Broadcast,true);
    udp.Connect(gz4d::IPSocket::Address(argv[1],port));
    
    for(int i = 3; i < argc; ++i)
    {
        gv::Device dev(argv[i]);
        gv::Capability cap(dev);

        //gv::Stream::Ptr s(new gv::Stream(dev,cap.getFormats()[0].format,160,120));
        gv::Stream::Ptr s(new gv::Stream(dev,cap.getFormats()[0].format,320,240));
        //gv::Stream::Ptr s(new gv::Stream(dev,cap.getFormats()[0].format,640,480));
        boost::rational<uint32_t> sfp = s->framePeriod(rfp);
        std::cerr << argv[i] << "\tframe period: " << sfp << std::endl;
        fp = std::min(fp,gz4d::time::Duration(boost::rational_cast<double>(sfp)));
        s->Start();
        t.add(s,0,t.columns());
    }

    std::cerr << "Frame period: " << fp << std::endl;
    t.framePeriod(fp);
    if(!t.empty())
    {
        gz4d::Time lastReportTime = gz4d::time::Now();
        for(int j = 0; j*fp < gz4d::seconds(30); ++j)
        {
            gz4d::binary::Wrapper<gz4d::binary::Image::Frame> frame = t.Read();
            frame.SendVia(udp);
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
