// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_MESSAGE_H
#define GZ4D_AIS_MESSAGE_H

//gz4d_cmake depends ais

#include <ais.h>
#include <memory>
#include <gz4d/time.h>

using namespace libais;

namespace gz4d
{
    namespace ais
    {
        struct Message
        {
            std::shared_ptr<AisMsg> _message;
            std::string _channel;
            std::string _receiver;
            std::string _vdm_msg_id;

            Time _timestamp;
            Time _receiver_timestamp;
            
            std::string _payload;
        };
        
    }
}

#endif
