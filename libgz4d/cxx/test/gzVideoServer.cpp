// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include<gz4d/v4l/Stream.h>
#include<gz4d/v4l/Capability.h>
#include <iostream>
#include <gz4d/vector/ios.h>
#include <fstream>
#include <gz4d/socket/TCPSocket.h>
#include <boost/thread.hpp>
#include <boost/utility.hpp>


struct StreamState
{
    int fmt;
    int width;
    int height;
    int fps;
    
    std::string outFilename;

    std::string previewAddress;
    int previewPort;
    gz4d::time::Duration preview_period;

    StreamState():fmt(0),width(0),height(0),fps(0),previewPort(0){}
};

struct DeviceRecord
{
    gz4d::v4l::Device device;
    std::shared_ptr<gz4d::v4l::Capability> capabilities;

    std::shared_ptr<gz4d::v4l::Stream> stream;
    gz4d::binary::Image::Stream::Header stream_header;
    gz4d::binary::Wrapper<gz4d::binary::Image::Stream::Frame> last_frame;
    bool last_frame_fresh;

    std::shared_ptr<std::ofstream> outfile;

    std::shared_ptr<gz4d::TCPSocket> preview_socket;
    gz4d::Time last_preview_time;
    

    StreamState requestedState;
    StreamState currentState;
    
    DeviceRecord(std::string const &devname):device(devname,false),last_frame_fresh(false){capabilities = std::shared_ptr<gz4d::v4l::Capability>(new gz4d::v4l::Capability(device));}
};


typedef std::shared_ptr<DeviceRecord> DRPtr;

struct Devices: boost::noncopyable
{
    std::vector<DRPtr> devices;
    boost::mutex mutex;
    boost::mutex previewMutex;
};

void PreviewThread(Devices *devices)
{
    while(!boost::this_thread::interruption_requested())
    {
        for(int i = 0; i < devices->devices.size(); ++i)
        {
            DRPtr d = devices->devices[i];
            StreamState rState;
            {
                boost::mutex::scoped_lock lock(devices->mutex);
                rState = d->requestedState;
            }

            if (rState.previewAddress != d->currentState.previewAddress || rState.previewPort != d->currentState.previewPort)
            {
                d->preview_socket.reset();
                d->currentState.previewPort = 0;
            }

            if(rState.previewPort > 0 && !d->preview_socket)
            {
                //                 std::cerr << "opening preview socket" << std::endl;
                try
                {
                    d->last_preview_time = gz4d::time::Now() - rState.preview_period;
                    d->preview_socket = std::shared_ptr<gz4d::TCPSocket>(new gz4d::TCPSocket);

                    d->preview_socket->Connect(gz4d::IPSocket::Address(rState.previewAddress,rState.previewPort));
                    d->preview_socket->SetNonBlocking();
                    d->currentState.previewAddress = rState.previewAddress;
                    d->currentState.previewPort = rState.previewPort;
                    d->currentState.preview_period = rState.preview_period;
                }
                catch(gz4d::Socket::Exception e)
                {
                    std::cerr << "problem connecting to preview client: " << e.what() << std::endl;
                    d->preview_socket.reset();
                    d->currentState.previewPort = 0;
                }
            }

            if(d->preview_socket)
            {
                std::vector<char> data;
                {
                    boost::mutex::scoped_lock lock(devices->previewMutex);
                    if(d->last_frame->dataSize > 0 && d->last_preview_time+d->currentState.preview_period <= gz4d::time::Now() && d->last_frame_fresh)
                    {
                        for(int j = 0; j < devices->devices[i]->last_frame->dataSize; j +=2)
                            data.push_back(devices->devices[i]->last_frame->data[j]);
                        d->last_preview_time = gz4d::time::Now();
                        d->last_frame_fresh = false;
                    }
                }
                if (!data.empty())
                {
                    try
                    {
                        d->preview_socket->Send(data);
                    }
                    catch(gz4d::Socket::Exception e)
                    {
                        std::cerr << "problem sending preview frame: " << e.what() << std::endl;
                        d->preview_socket.reset();
                        d->currentState.previewPort = 0;
                    }
                }
            }
        }
        boost::this_thread::yield();
    }
}

void WorkThread(Devices *devices)
{
    while(!boost::this_thread::interruption_requested())
    {
        for(int i = 0; i < devices->devices.size(); ++i)
        {
//             std::cerr << "device: " << i << std::endl;
            DRPtr d = devices->devices[i];
            StreamState rState;
            {
                boost::mutex::scoped_lock lock(devices->mutex);
                rState = d->requestedState;
            }

//             std::cerr << "got rState" << std::endl;
//             std::cerr << rState.fmt << ", " << rState.width << ", " << rState.height << ", " << rState.fps << std::endl;

            if (rState.fmt != d->currentState.fmt || rState.width != d->currentState.width || rState.height != d->currentState.height || rState.fps != d->currentState.fps)
            {
                d->stream.reset();
                d->currentState.fps = 0;
            }

            if(rState.fps > 0 && !d->stream)
            {
//                 std::cerr << "opening stream" << std::endl;
                d->stream = std::shared_ptr<gz4d::v4l::Stream>(new gz4d::v4l::Stream(d->device, d->capabilities->getFormats()[rState.fmt].format,rState.width,rState.height));

                boost::rational<uint32_t> rfp(1,rState.fps);
                d->stream->framePeriod(rfp);

                d->stream_header.timestamp = gz4d::time::Now();
                d->stream_header.streamID = i;
                d->stream_header.width = rState.width;
                d->stream_header.height = rState.height;
                d->stream_header.fourcc.value = d->capabilities->getFormats()[rState.fmt].format;
                d->stream_header.framePeriod.numerator = 1;
                d->stream_header.framePeriod.denominator = rState.fps;
                d->last_frame->streamID = i;
                d->stream->Start(false);
                
                d->currentState.fmt = rState.fmt;
                d->currentState.width = rState.width;
                d->currentState.height = rState.height;
                d->currentState.fps = rState.fps;
            }


            if(rState.outFilename != d->currentState.outFilename)
            {
                if(rState.outFilename.empty())
                    d->outfile.reset();
                else
                    d->outfile = std::shared_ptr<std::ofstream>(new std::ofstream(rState.outFilename.c_str()));
                d->currentState.outFilename = rState.outFilename;
            }
            
            if(devices->devices[i]->stream)
            {
                if(devices->devices[i]->outfile)
                {
                    if(devices->devices[i]->outfile->tellp() == 0)
                        devices->devices[i]->outfile->write(reinterpret_cast<char const *>(&devices->devices[i]->stream_header), devices->devices[i]->stream_header.size);
                    gz4d::binary::Wrapper<gz4d::binary::Image::Packet> p(devices->devices[i]->last_frame);
                    boost::mutex::scoped_lock lock(devices->previewMutex);
                    if(devices->devices[i]->stream->Read(p))
                    {
                        d->last_frame_fresh = true;
                        devices->devices[i]->last_frame.StreamTo(*devices->devices[i]->outfile);
                    }
                }
                else
                {
                    boost::mutex::scoped_lock lock(devices->previewMutex);
                    gz4d::binary::Wrapper<gz4d::binary::Image::Packet> p(devices->devices[i]->last_frame);
                    if(devices->devices[i]->stream->Read(p))
                        d->last_frame_fresh = true;
                }
            }
        }
        boost::this_thread::yield();
    }
}

int main(int argc, char * argv[])
{

    if (argc < 3)
    {
        std::cerr << "usage: gzVideoServer tcp_port dev1 [dev2 ...]" << std::endl;
        exit(1);
    }

    std::string address = "ANY";
    int port = atoi(argv[1]);


    gz4d::TCPSocket s;
    s.SetOption(gz4d::Socket::ReuseAddress,1);
    s.Bind(gz4d::IPSocket::Address(address,port));
    s.Listen(1);

    Devices devices;

    bool shutdown = false;
    while (!shutdown)
    {
        devices.devices.clear();
        
        for(int i = 2; i < argc; ++i)
        {
            DRPtr d(new DeviceRecord(argv[i]));
            devices.devices.push_back(d);
        }

        std::cout << "Looking for a client...\n";
        gz4d::IPSocket::Address clientAddress;
        gz4d::Socket client = s.Accept(clientAddress);

        std::cout << "Connection from: " << clientAddress.GetAddress() << ":" << clientAddress.GetPort() << "\n";
        client.SetNonBlocking();

        client.Send(std::string("# welcome to gzVideoServer. type 'help' for help.\n> "));
//         std::cerr << "sent welcome msg" << std::endl;

        std::string in_data;
        std::string in_buffer;

        boost::thread workThread(WorkThread,&devices);
        boost::thread previewThread(PreviewThread,&devices);
//         std::cerr << "started work thread" << std::endl;
        
        bool done = false;
        while(!done)
        {

            in_data.resize(1024);
            try
            {
                client.Recv(in_data);
//                 std::cout << "Received: " << in_data << "\n";

                in_buffer.append(in_data);

                if(in_buffer.find('\n') != std::string::npos)
                {
                    //std::cout << "in_buffer before: ***" << in_buffer << "***\n";
                    std::istringstream cmdstr(in_buffer.substr(0,in_buffer.find('\n')));
                    //std::cout << "cmdstr: ***" << cmdstr.str() << "***\n";
                    in_buffer = in_buffer.substr(in_buffer.find('\n')+1);
                    //std::cout << "in_buffer after: ***" << in_buffer << "***\n";

                    std::string cmd;
                    cmdstr >> cmd;

//                     std::cout << "cmd: " << cmd << std::endl;

                    if (cmd == "list")
                    {
                        boost::mutex::scoped_lock lock(devices.mutex);
                        std::stringstream ret;
                        for(int i = 0; i < devices.devices.size(); ++i)
                        {
                            ret << "device " << i << " " << devices.devices[i]->capabilities->getCard() << std::endl;
                            for(int f = 0; f < devices.devices[i]->capabilities->getFormats().size(); ++f)
                            {
                                char const *fourcc = reinterpret_cast<char const *>(&(devices.devices[i]->capabilities->getFormats()[f].format));
                                ret << "    format " << f << " " << std::string(fourcc,&fourcc[4]) << " " << devices.devices[i]->capabilities->getFormats()[f].description << std::endl;
                            }
                        }
                        client.Send(ret.str());
                    }
                    else if (cmd == "stream" || cmd == "record" || cmd == "preview")
                    {
                        boost::mutex::scoped_lock lock(devices.mutex);
                        int dev;
                        cmdstr >> dev;
                        if(dev < 0 || dev >= devices.devices.size())
                            client.Send(std::string("Error: invalid device number\n"));
                        else
                        {
                            DRPtr d = devices.devices[dev];
                            std::string subcmd;
                            cmdstr >> subcmd;
                            if(subcmd == "start")
                            {
                                if (cmd == "stream")
                                {
                                    int fmt,width, height, fps;
                                    cmdstr >> fmt >> width >> height >> fps;
                                    if (cmdstr.fail())
                                        client.Send(std::string("Error: usage: stream dev# start fmt# width height fps\n"));
                                    else
                                    {

                                        if(fmt < 0 || fmt >= d->capabilities->getFormats().size())
                                            client.Send(std::string("Error: invalid format number\n"));
                                        else
                                        {
                                            d->requestedState.fmt = fmt;
                                            d->requestedState.width = width;
                                            d->requestedState.height = height;
                                            d->requestedState.fps = fps;
                                            
                                        }
                                    }
                                }
                                else if (cmd == "record")
                                {
                                    std::string fname;
                                    cmdstr >> fname;
                                    d->requestedState.outFilename = fname;
                                    
                                }
                                else if (cmd == "preview")
                                {
                                    double period;
                                    std::string address;
                                    int port;
                                    cmdstr >> period >> address >> port;
                                    if (cmdstr.fail())
                                        client.Send(std::string("Error: usage: preview dev# start frame_period address port\n"));
                                    else
                                    {
                                        d->requestedState.preview_period = gz4d::seconds(period);
                                        d->requestedState.previewAddress = address;
                                        d->requestedState.previewPort = port;
                                    }
                                }
                            }
                            else if (subcmd == "stop")
                            {
                                if (cmd == "stream")
                                {
                                    d->requestedState.fps = 0;
                                }
                                else if (cmd == "record")
                                {
                                    d->requestedState.outFilename = "";
                                }
                                else if (cmd == "preview")
                                {
                                    d->requestedState.previewPort = 0;
                                }
                            }
                            else
                            {
                                client.Send(std::string("Error: type help for more information\n"));
                            }
                        }
                    }
                    else if(cmd == "quit" || cmd == "exit")
                    {
                        done = true;
                        std::cout << "Exit request from client...\n";
                        break;
                    }
                    else if(cmd == "shutdown")
                    {
                        done = true;
                        shutdown = true;
                        std::cout << "Shutdown request from client...\n";
                        break;
                    }
                    else if(cmd == "help")
                    {
                        client.Send(std::string("[exit | quit]\n            exits session\n\n"));
                        client.Send(std::string("shutdown\n            exits session and shuts down server\n\n"));
                        client.Send(std::string("list\n            list devices and formats. Use to find out dev and fmt parameters\n\n"));
                        client.Send(std::string("stream dev [start fmt width height fps | stop]\n"));
                        client.Send(std::string("            start or stop a stream.\n\n"));
                        client.Send(std::string("record dev [start filename | stop]\n"));
                        client.Send(std::string("            start or stop recording to a file.\n\n"));
                        client.Send(std::string("preview dev [start interval address port | stop]\n"));
                        client.Send(std::string("            start or stop sending preview frames to TCP socket.\n\n"));
                    }
                    else
                        client.Send(std::string("Error: Unknown command. Type help for more information\n"));

                    client.Send(std::string("> "));
                }
            }
            catch(gz4d::Socket::WouldBlockException)
            {
                boost::this_thread::yield();
            }
            catch(gz4d::Socket::ConnectionClosedException)
            {
                std::cout << "Connection closed by peer\n";
                break;
            }
        }
        std::cout << "Connection closed.\n";
        client.Close();
        workThread.interrupt();
        previewThread.interrupt();
        workThread.join();
        previewThread.join();
    }
    std::cout << "Shutting down.\n";
    s.Close();
}
