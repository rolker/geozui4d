// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_APPLICATIONSPECIFICHISTORY_H
#define GZ4D_AIS_APPLICATIONSPECIFICHISTORY_H

#include "Message.h"
#include <gz4d/TimeSequence.h>

namespace gz4d
{
    namespace ais
    {
        typedef std::pair<int,int> BinaryMessageType;
        
        class ApplicationSpecificHistory
        {
            std::map<BinaryMessageType,TimeSequence<Message> > _messages;
        public:
            static std::string dacToCountryStr(int dac)
            {
                switch(dac)
                {
                    case 1:
                        return "IMO";
                    case 239:
                    case 240:
                    case 241:
                        return "Greece";
                    case 316:
                        return "Canada";
                    case 366:
                        return "USA";
                }
                return "Unknown";
            }
            
            bool insert(Message const &m)
            {
                switch(m._message->message_id)
                {
                    case 8:
                    {
                        std::shared_ptr<Ais8> ais8 = std::dynamic_pointer_cast<Ais8>(m._message);
                        BinaryMessageType dacfi(ais8->dac, ais8->fi);
                        _messages[dacfi][m._timestamp] = m;
                        return true;
                    }
                }
                return false;
            }
            
        };
    }
}

#endif
