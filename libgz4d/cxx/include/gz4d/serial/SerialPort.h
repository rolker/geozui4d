// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SERIAL_SERIALPORT_H
#define GZ4D_SERIAL_SERIALPORT_H

//gz4d_cmake depends serial

#include <gz4d/gz4d.h>
#include <memory>
#include <string>
#include <cstdio>
#include <fcntl.h>
#include <termios.h>

namespace gz4d
{
    /// Data communication socket.
    /// \ingroup utilities
    class SerialPort
    {
        struct Handle
        {
            int fd;
            
            Handle(std::string const &port)
            {
                fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
                //fcntl(fd, F_SETFL, FNDELAY);
                fcntl(fd, F_SETFL, 0);
            }
            
            ~Handle()
            {
                if(fd != -1)
                    close(fd);
            }
        };
        
        std::shared_ptr<Handle> handle;

        public:
            void Open(std::string const &port)
            {
                handle = std::shared_ptr<Handle>(new Handle(port));
            }
            

            void Write(char const *ptr, int len)
            {
                write(handle->fd,ptr,len);
            }
            
            template <typename dataType>
                    void Write(dataType const &data){Write(&data[0],data.size());}

            std::size_t Read(void *buf, std::size_t nbyte)
            {
                ssize_t ret = read(handle->fd,buf,nbyte);
                if(ret >= 0)
                    return ret;
                return 0;
            }
            
            template <typename dataType>
                    void Read(dataType &data){std::size_t ret = Read(&data[0],data.size()); if(ret < 0) data.resize(0); else data.resize(ret);}

            void SetBaudRate(int baudRate)
            {
                speed_t br;
                switch(baudRate)
                {
                    case 4800:
                        br = B4800;
                        break;
                    case 9600:
                        br = B9600;
                        break;
                    case 19200:
                        br = B19200;
                        break;
                    case 38400:
                        br = B38400;
                        break;
                    case 57600:
                        br = B57600;
                        break;
                    case 115200:
                        br = B115200;
                        break;
                    default:
                        br = B9600;
                }
                
                termios options;
                tcgetattr(handle->fd, &options);
                cfsetispeed(&options, br);
                cfsetospeed(&options, br);
                options.c_cflag |= (CLOCAL | CREAD);
                tcsetattr(handle->fd, TCSANOW, &options);
            }
            
    };
}

#endif
