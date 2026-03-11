// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_V4L_STREAM_H
#define GZ4D_V4L_STREAM_H

#include <gz4d/gz4d.h>
#include <boost/utility.hpp>
#include <gz4d/v4l/Device.h>
#include <gz4d/Image.h>
#include <gz4d/thread/Producer.h>
#include <gz4d/time/Point.h>
#include <boost/rational.hpp>
#include <gz4d/data/binary/Packet.h>
#include <gz4d/data/binary/Wrapper.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cstring>
#include <iostream>
#include <sys/mman.h>
#include <gz4d/socket/Socket.h>

#define GZ4D_FRAGMENT_DATA_SIZE 1400


namespace gz4d
{
    class Socket;

    namespace v4l
    {

        class GZ4DEXPORT Stream: public Producer<binary::Wrapper<binary::Image::Frame> >, boost::noncopyable
        {
            Device _device;
            uint32_t _format;
            std::size_t _width;
            std::size_t _height;
            uint32_t _bytesperline;
            bool _threaded;

            struct Buffer
            {
                void * _start;
                size_t _length;
            };

            std::vector<Buffer> _buffers;

            boost::thread inputThread;
            void InputThread()
            {
                Start(false);
                _threaded = true;
                
                while(!boost::this_thread::interruption_requested())
                {
                    binary::Wrapper<binary::Image::Frame> f = Read();
                    if(f->dataSize > 0)
                        Send(f);
                    boost::this_thread::yield();
                }
                
                StreamOff();
            }
            
            binary::Wrapper<binary::Image::Frame> Read()
            {
                gz4d::binary::Wrapper<gz4d::binary::Image::Frame> ret;
                gz4d::binary::Wrapper<gz4d::binary::Image::Packet> p(ret);
                Read(p);
                
                return ret;
            }
            
            void StreamOff()
            {
                v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if(ioctl(_device.fd, VIDIOC_STREAMOFF, &type))
                    std::cerr << "Error: VIDIOC_STREAMOFF: " << strerror(errno) << std::endl;
            }
            
            public:
                typedef std::shared_ptr<Stream> Ptr;

                Stream(Device const &dev, uint32_t format, uint32_t width, uint32_t height)
                :_device(dev),_format(format),_width(width),_height(height),_bytesperline(width*2),_buffers(20),_threaded(false)
                {
                    v4l2_cropcap cropcap;
                    memset(&cropcap, 0, sizeof(cropcap));
                    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    if(ioctl(_device.fd,VIDIOC_CROPCAP, &cropcap) == 0)
                    {
                        v4l2_crop crop;
                        memset (&crop, 0, sizeof (crop));
                        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        crop.c = cropcap.defrect;
                        if(ioctl (_device.fd, VIDIOC_S_CROP, &crop) == -1 && errno != EINVAL)
                            std::cerr << "Stream::Stream: error with set crop ioctl: " << strerror(errno) << std::endl;
                    }
                    else
                        std::cerr << "Stream::Stream: error with get crop ioctl: " << strerror(errno) << std::endl;
                    
                    v4l2_format fmt;
                    memset(&fmt, 0, sizeof(fmt));
                    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    fmt.fmt.pix.width = _width;
                    fmt.fmt.pix.height = _height;
                    fmt.fmt.pix.pixelformat = format;
                    if(ioctl(_device.fd, VIDIOC_S_FMT, &fmt) != 0)
                    {
                        std::cerr << "Stream::Stream: error with set format ioctl: " << strerror(errno) << std::endl;
                        _width = 0;
                        _height = 0;
                        _buffers.clear();
                        return;
                    }
                    _width = fmt.fmt.pix.width;
                    _height = fmt.fmt.pix.height;
                    _bytesperline = fmt.fmt.pix.bytesperline;
                    
                    
                    v4l2_requestbuffers req;
                    memset(&req, 0, sizeof(req));
                    req.count = _buffers.size();
                    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    req.memory = V4L2_MEMORY_MMAP;
                    
                    if(ioctl(_device.fd, VIDIOC_REQBUFS, &req)!= 0)
                    {
                        std::cerr << "Error: " << strerror(errno) << std::endl;
                        _buffers.clear();
                        return;
                    }
                    
                    //std::cerr << "buffer count: requested: " << _buffers.size() << " got: " << req.count << std::endl;
                    _buffers.resize(req.count);
                    
                    for (int i = 0; i < _buffers.size(); ++i)
                    {
                        v4l2_buffer buf;
                        memset(&buf,0,sizeof(buf));
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;
                        
                        if(ioctl(_device.fd, VIDIOC_QUERYBUF, &buf))
                            std::cerr << "Error: VIDIOC_QUERYBUF: " << strerror(errno) << std::endl;
                        
                        
                        _buffers[i]._length = buf.length;
                        _buffers[i]._start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, _device.fd, buf.m.offset);
                        
                        if (_buffers[i]._start == MAP_FAILED)
                        {
                            std::cerr << "Failed mapping buffer " << i << std::endl;
                            _buffers.resize(i);
                        }
                        else
                        {
                            //std::cerr << "buffer " << i << " " << _buffers[i]._length << " bytes" << std::endl;
                        }
                    }
                }
                
                ~Stream()
                {
                    for(int i = 0; i < _buffers.size(); ++i)
                        munmap(_buffers[i]._start, _buffers[i]._length);
                    Stop();
                }
                

                boost::rational<uint32_t> framePeriod()
                {
                    v4l2_streamparm sparm;
                    memset(&sparm,0,sizeof(sparm));
                    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    if(ioctl(_device.fd, VIDIOC_G_PARM, &sparm)== 0)
                    {
                        return boost::rational<uint32_t>(sparm.parm.capture.timeperframe.numerator,sparm.parm.capture.timeperframe.denominator);
                    }
                    return 0;
                }
                
                boost::rational<uint32_t> framePeriod(boost::rational<uint32_t> fp)
                {
                    v4l2_streamparm sparm;
                    memset(&sparm,0,sizeof(sparm));
                    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    
                    if(ioctl(_device.fd, VIDIOC_G_PARM, &sparm) == 0)
                    {
                        if (!sparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
                            std::cerr << "Stream::framePeriod setting frame period not supported" << std::endl;
                    }
                    else
                    {
                        std::cerr << "Stream::framePeriod error getting param: " << strerror(errno) << std::endl;
                    }
                    
                    memset(&sparm,0,sizeof(sparm));
                    sparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    
                    sparm.parm.capture.timeperframe.numerator = fp.numerator();
                    sparm.parm.capture.timeperframe.denominator = fp.denominator();
                    if(ioctl(_device.fd, VIDIOC_S_PARM, &sparm) != 0)
                        std::cerr << "Stream::framePeriod error setting param: " << strerror(errno) << std::endl;
                    
                    return framePeriod();
                }
                

                void Start(bool threaded=true)
                {
                    if (threaded)
                        inputThread = boost::thread(boost::bind(&Stream::InputThread,this));
                    else
                    {
                        for (int i = 0; i < _buffers.size(); ++i)
                        {
                            v4l2_buffer buf;
                            memset(&buf,0,sizeof(buf));
                            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                            buf.memory = V4L2_MEMORY_MMAP;
                            buf.index = i;
                            if(ioctl(_device.fd, VIDIOC_QBUF, &buf))
                                std::cerr << "Error: VIDIOC_QBUF: " << strerror(errno) << std::endl;
                        }
                        
                        v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        if(ioctl(_device.fd, VIDIOC_STREAMON, &type))
                            std::cerr << "Error: VIDIOC_STREAMON: " << strerror(errno) << std::endl;
                    }
                }
                
                void Stop()
                {
                    if(_threaded)
                    {
                        if(inputThread.joinable())
                        {
                            inputThread.interrupt();
                            inputThread.join();
                        }
                    }
                    else
                    {
                        StreamOff();
                    }
                }
                
                bool SendTo(Socket &s)
                {
                    v4l2_buffer buf;
                    memset(&buf, 0, sizeof(buf));
                    
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    
                    if(ioctl(_device.fd,VIDIOC_DQBUF, &buf) == 0)
                    {
                        if(buf.index < _buffers.size())
                        {
                            s.Send(reinterpret_cast<char *>(_buffers[buf.index]._start),buf.bytesused);
                            if(ioctl(_device.fd,VIDIOC_QBUF, &buf))
                                std::cerr << "Error: VIDIOC_QBUF: " << strerror(errno) << std::endl;
                            return true;
                        }
                    }
                    return false;
                }
                
                bool SendTo(std::basic_ostream< char > &strm)
                {
                    binary::Wrapper<binary::Image::Frame> f = Read();
                    if(f->dataSize > 0)
                        f.StreamTo(strm);
                }
                

                bool Read(gz4d::binary::Wrapper<gz4d::binary::Image::Packet> &p)
                {
                    v4l2_buffer buf;
                    memset(&buf, 0, sizeof(buf));
                    
                    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    buf.memory = V4L2_MEMORY_MMAP;
                    
                    bool ret = false;
                    
                    if(ioctl(_device.fd,VIDIOC_DQBUF, &buf) == 0)
                    {
                        if(buf.index < _buffers.size())
                        {
                            p->timestamp = buf.timestamp;
                            gz4d::binary::Wrapper<gz4d::binary::Image::Stream::Frame> sf(p);
                            if(sf->IsValid())
                            {
                                sf->dataSize = buf.bytesused;
                                sf.Resize();
                                sf->frameNumber = buf.sequence;
                                memcpy(sf->data , _buffers[buf.index]._start,buf.bytesused);
                                ret = true;
                            }
                            gz4d::binary::Wrapper<gz4d::binary::Image::Frame> f(p);
                            if(f->IsValid())
                            {
                                f->dataSize = buf.bytesused;
                                f.Resize();
                                
                                f->width = _width;
                                f->height = _height;
                                f->fourcc.value = _format;
                                f->frameNumber = buf.sequence;
                                
                                memcpy(f->data , _buffers[buf.index]._start,buf.bytesused);
                                ret = true;
                            }
                            
                            if(ioctl(_device.fd,VIDIOC_QBUF, &buf))
                                std::cerr << "Error: VIDIOC_QBUF: " << strerror(errno) << std::endl;
                        }
                    }
                    else if(errno != EAGAIN)
                        std::cerr << "Error: VIDIOC_DQBUF: (" << errno << ") " << strerror(errno) << std::endl;
                    return ret;
                }
                
                

                std::size_t width() const {return _width;}
                std::size_t height() const {return _height;}
        };
    }
}

#endif
