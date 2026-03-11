// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_V4L_DEVICE_H
#define GZ4D_V4L_DEVICE_H

//gz4d_cmake depends v4l

#include <gz4d/gz4d.h>
#include <fcntl.h>
#include <memory>
#include <string>

namespace gz4d
{
    namespace v4l
    {
        class Capability;
        class Stream;

        class Device
        {
            friend class Capability;
            friend class Stream;

            int fd;

            public:
                typedef std::shared_ptr<Device> Ptr;

                Device(std::string const &dev_name, bool blocking=true)
                {
                    if (blocking)
                        fd = open(dev_name.c_str(),O_RDWR);
                    else
                        fd = open(dev_name.c_str(),O_RDWR|O_NONBLOCK);
                }
                
                Device(Device const &d)
                :fd(dup(d.fd))
                {
                }
                

                ~Device()
                {
                    if(fd > 0)
                        close(fd);
                }
                
        };
    }
}

#endif
