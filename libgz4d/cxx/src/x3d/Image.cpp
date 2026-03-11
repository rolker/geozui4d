// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#define GZ4D_INTERNAL
#include <gz4d/x3d/Image.h>


uint8_t gz4d::x3d::Image::fromHex(char const &c) const
{
    if(c >= '0' && c <= '9')
        return c-'0';
    else
        return 10+c-'a';
}

uint8_t gz4d::x3d::Image::fromHex(std::string const &s, int c) const
{
    return (fromHex(s[2+c*2])<<4)|fromHex(s[2+c*2+1]);
}

gz4d::x3d::Image::Image(){}

std::istream &gz4d::x3d::operator>>(std::istream &str, Image &i)
{
    str >> i.width;
    str >> i.height;
    str >> i.components;
    for(std::size_t p = 0; p < i.width*i.height; ++p)
    {
        std::string v;
        str >> v;
        for(int c = 0; c < i.components; ++c)
            i.pixels.push_back(i.fromHex(v,c));
    }
    return str;
}

std::ostream &gz4d::x3d::operator<<(std::ostream &str, Image &i)
{
    str << i.width << " ";
    str << i.height << " ";
    str << i.components << " ";
/*    for(std::size_t p = 0; p < i.width*i.height; ++p)
    {
        for(int c = 0; c < i.components; ++c)
            str << i.pixels[p*i.components+c] << " ";
    }*/
    return str;
}

