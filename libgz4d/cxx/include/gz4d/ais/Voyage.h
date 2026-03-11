// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_VOYAGE_H
#define GZ4D_AIS_VOYAGE_H

#include "Message.h"
#include <gz4d/Box.h>

namespace gz4d
{
    namespace ais
    {
        struct Voyage
        {
            int imoNumber;
            std::string callsign;
            std::string vessel_name;
            int shipType;
            Box<Vector<int, 2> > dimensions;
            float draught;
            std::string destination;
            
            Voyage():imoNumber(0),shipType(0),draught(0.0){}
            
            Voyage(Message const &m)
            {
                switch(m._message->message_id)
                {
                    case 5:
                    {
                        std::shared_ptr<Ais5> ais5 = std::dynamic_pointer_cast<Ais5>(m._message);
                        imoNumber = ais5->imo_num;
                        callsign = ais5->callsign;
                        vessel_name = ais5->name;
                        shipType = ais5->type_and_cargo;
                        // assuming dim_b is distance to stern and dim_c is distance to port side
                        Vector<int,2> ll(-ais5->dim_c,-ais5->dim_b);
                        // assuming dim_a is distance to bow and dim_d is distance to starboard side
                        Vector<int,2> ur(ais5->dim_d,-ais5->dim_a);
                        dimensions = Box<Vector<int,2> >(ll,ur);
                        draught = ais5->draught;
                        destination = ais5->destination;
                    }
                    break;
                    case 24:
                    {
                        std::shared_ptr<Ais24> ais24 = std::dynamic_pointer_cast<Ais24>(m._message);
                        callsign = ais24->callsign;
                        vessel_name = ais24->name;
                        shipType = ais24->type_and_cargo;
                        // assuming dim_b is distance to stern and dim_c is distance to port side
                        Vector<int,2> ll(-ais24->dim_c,-ais24->dim_b);
                        // assuming dim_a is distance to bow and dim_d is distance to starboard side
                        Vector<int,2> ur(ais24->dim_d,-ais24->dim_a);
                        dimensions = Box<Vector<int,2> >(ll,ur);
                        draught = Nan<float>();
                    }
                }
            }
            
            bool operator!=(Voyage const &other) const
            {
                return imoNumber != other.imoNumber ||
                       callsign != other.callsign ||
                       vessel_name != other.vessel_name ||
                       shipType != other.shipType ||
                       dimensions != other.dimensions ||
                       draught != other.draught ||
                       destination != other.destination;
            }
            
            bool operator==(Voyage const &other) const
            {
                return !((*this)!=other);
            }
            
        };
        
    }
}

#endif
