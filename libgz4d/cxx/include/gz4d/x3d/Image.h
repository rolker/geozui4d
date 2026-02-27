#ifndef GZ4D_X3D_IMAGE_H
#define GZ4D_X3D_IMAGE_H

#include <gz4d/gz4d.h>
//#include<boost/cstdint.hpp>
#include <vector>
#include <string>
#include <istream>

namespace gz4d
{

    namespace x3d
    {
        struct GZ4DEXPORT Image
        {
            std::size_t width;
            std::size_t height;
            uint8_t components;
            std::vector<uint8_t> pixels;

            uint8_t fromHex(char const &c) const;

            uint8_t fromHex(std::string const &s, int c) const;

            Image();
        };

        std::istream &operator>>(std::istream &str, Image &p);
        std::ostream &operator<<(std::ostream &str, Image &p);

    }
}

#endif
