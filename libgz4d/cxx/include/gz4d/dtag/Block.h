// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_DTAG_BLOCK_H
#define GZ4D_DTAG_BLOCK_H

#include <gz4d/gz4d.h>
//#include <boost/cstdint.hpp>

namespace gz4d
{
    namespace dtag
    {
        struct BlockAddress
        {
            uint16_t block; ///< block number within chip.
            uint32_t byte; ///< byte offset in block relative to end of block header.
            BlockAddress()
            :block(0xffff),byte(0)
            {
            }
            
            BlockAddress(uint16_t block, uint32_t byte)
            :block(block),byte(byte)
            {
            }
            
            operator bool() const
            {
                return block != 0xffff;
            }
            
        };
    }
}

#endif
