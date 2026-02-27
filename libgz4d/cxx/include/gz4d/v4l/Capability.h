#ifndef GZ4D_V4L_CAPABILITY_H
#define GZ4D_V4L_CAPABILITY_H

#include <gz4d/gz4d.h>
#include <string>
#include <gz4d/Box.h>
#include <gz4d/v4l/Device.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <cstring>

namespace gz4d
{
    namespace v4l
    {
        class Device;

        class Capability
        {
            public:
                typedef Box< Vector<int32_t,2> > BoundsType;
            private:
                std::string driver;
                std::string card;
                std::string bus_info;
                bool video_capture;
                bool readwrite;
                bool streaming;

                BoundsType bounds;
                BoundsType default_bounds;

                float pixelaspect;

            public:
                struct Format
                {
                    std::string description;
                    uint32_t format;
                    bool compressed;
                    bool operator==(Format const &o) const {return description == o.description && format == o.format && compressed == o.compressed;}
                };

                struct Input
                {
                    std::string name;
                    bool isCamera;
                    bool isTuner;
                    uint64_t standard;

                    Input():isCamera(false),isTuner(false){}
                };

                Capability(Device const &device)
                :video_capture(false),readwrite(false),streaming(false),pixelaspect(0.0)
                {
                    v4l2_capability cap;
                    if (ioctl(device.fd,VIDIOC_QUERYCAP,&cap) == 0)
                    {
                        driver = reinterpret_cast<char const *>(cap.driver);
                        card = reinterpret_cast<char const *>(cap.card);
                        bus_info = reinterpret_cast<char const *>(cap.bus_info);
                        
                        video_capture = cap.capabilities & V4L2_CAP_VIDEO_CAPTURE;
                        readwrite = cap.capabilities & V4L2_CAP_READWRITE;
                        streaming = cap.capabilities & V4L2_CAP_STREAMING;
                    }
                    
                    if(video_capture)
                    {
                        /*        v4l2_input input;
                         *        memset(&input,0,sizeof(input));
                         *        while(ioctl(device.fd, VIDIOC_ENUMINPUT, &input)==0)
                         *        {
                         *            std::cerr << "input:\t" << input.name << std::endl;
                         *            ++input.index;
                         }*/
                        
                        //         v4l2_standard standard;
                        //         memset(&standard,0,sizeof(standard));
                        //         while(ioctl(device.fd, VIDIOC_ENUMSTD, &standard)==0)
                        //         {
                            //             std::cerr << "standard:\t" << standard.name << std::endl;
                            //             ++standard.index;
                            //         }
                            
                            v4l2_cropcap cropcap;
                            memset(&cropcap, 0, sizeof(cropcap));
                            cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                            if(ioctl(device.fd,VIDIOC_CROPCAP, &cropcap) == 0)
                            {
                                bounds.setMin(Vector<int32_t,2>(cropcap.bounds.left,cropcap.bounds.top));
                                bounds.setSizesFromMin(Vector<int32_t,2>(cropcap.bounds.width,cropcap.bounds.height));
                                
                                default_bounds.setMin(Vector<int32_t,2>(cropcap.defrect.left,cropcap.defrect.top));
                                default_bounds.setSizesFromMin(Vector<int32_t,2>(cropcap.defrect.width,cropcap.defrect.height));
                                
                                pixelaspect = cropcap.pixelaspect.numerator/float(cropcap.pixelaspect.denominator);
                            }
                            
                            v4l2_fmtdesc fmtdesc;
                            memset(&fmtdesc, 0, sizeof(fmtdesc));
                            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                            fmtdesc.index = 0;
                            while(ioctl(device.fd,VIDIOC_ENUM_FMT,&fmtdesc)==0)
                            {
                                Format f;
                                f.format = fmtdesc.pixelformat;
                                f.description = reinterpret_cast<char * const>(fmtdesc.description);
                                f.compressed = fmtdesc.flags & V4L2_FMT_FLAG_COMPRESSED;
                                formats.push_back(f);
                                ++fmtdesc.index;
                            }
                            
                            v4l2_input input;
                            memset(&input, 0, sizeof(input));
                            input.index = 0;
                            while(ioctl(device.fd,VIDIOC_ENUMINPUT,&input)==0)
                            {
                                Input i;
                                i.name = reinterpret_cast<char * const>(input.name);
                                switch(input.type)
                                {
                                    case V4L2_INPUT_TYPE_TUNER:
                                        i.isTuner = true;
                                        break;
                                    case V4L2_INPUT_TYPE_CAMERA:
                                        i.isCamera = true;
                                        break;
                                    default:
                                        break;
                                }
                                i.standard = input.std;
                                inputs.push_back(i);
                                ++input.index;
                            }
                    }
                    
                }
                

                std::string const &getDriver() const
                {
                    return driver;
                }
                
                std::string const &getCard() const
                {
                    return card;
                }
                
                std::string const &getBusInfo() const
                {
                    return bus_info;
                }
                

                bool isVideoCapture() const
                {
                    return video_capture;
                }
                
                bool isReadWrite() const
                {
                    return readwrite;
                }
                
                bool isStreaming() const
                {
                    return streaming;
                }
                

                BoundsType const &getBounds() const
                {
                    return bounds;
                }
                
                BoundsType const &getDefaultBounds() const
                {
                    return default_bounds;
                }
                

                float getPixelAspect() const
                {
                    return pixelaspect;
                }
                
                std::vector<Format> const & getFormats() const
                {
                    return formats;
                }
                

                std::vector<Input> const & getInputs() const
                {
                    return inputs;
                }
                
            private:
                std::vector<Format> formats;
                std::vector<Input> inputs;
        };
    }
}


#endif
