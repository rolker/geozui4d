// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SWAP_H    
#define GZ4D_SWAP_H

#include <gz4d/Exception.h>

namespace gz4d
{
    namespace priv
    {
        template <int S> struct SwapSize
        {
            int dummy;
            SwapSize(){}
        };

        template <typename T> T Swap(T const &value, SwapSize<1> const &s)
        {
            return value;
        }

        template <typename T> T Swap(T const &value, SwapSize<2> const &s)
        {
            return((value << 8)&0xff00)|((value>>8)&0x00ff);
        }

        template <typename T> T Swap(T const &value, SwapSize<4> const &s)
        {
            return  ((value << 24)&0xff000000)|
                    ((value << 8)&0x00ff0000)|
                    ((value >> 8)&0x0000ff00)|
                    ((value>>24)&0x000000ff);
        }

        template <typename T> T Swap(T const &value, SwapSize<8> const &s)
        {
            return  ((value&0x000000ff) << 56)|
                    ((value&0x0000ff00) << 40)|
                    ((value&0x00ff0000) << 24)|
                    ((value&0xff000000) << 8)|
                    ((value >> 8)&0xff000000)|
                    ((value >> 24)&0x00ff0000)|
                    ((value >> 40)&0x0000ff00)|
                    ((value >> 56)&0x000000ff);
        }
    }

    template<typename T> T Swap(T const &value)
    {
        return priv::Swap(value, priv::SwapSize<sizeof(T)>());
    }

    template<typename T> T Swap(T const &value, bool s)
    {
        if(s)
            return Swap(value);
        return value;
    }
}

#endif
