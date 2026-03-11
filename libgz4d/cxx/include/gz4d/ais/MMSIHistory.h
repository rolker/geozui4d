// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_MMSIHISTORY_H
#define GZ4D_AIS_MMSIHISTORY_H

#include "Message.h"
#include "MessageHistory.h"

namespace gz4d
{
    namespace ais
    {
        /// Collects positions and voyage info for a single MMSI unit.
        class MMSIHistory
        {
            int _mmsi;
            PositionHistory _positions;
            VoyageHistory _voyage_info;
            
            
        public:
            MMSIHistory():_mmsi(-1){}
            MMSIHistory(int mmsi):_mmsi(mmsi){}
            
            /// @return: true on success.
            bool insert(Message const &m)
            {
                assert(m._message->mmsi > 0);
                if(_mmsi < 0)
                    _mmsi = m._message->mmsi;
                assert(m._message->mmsi == _mmsi);
                switch(m._message->message_id)
                {
                    case 1:
                    case 2:
                    case 3:
                    case 18:
                        _positions.insert(Position(m),m._timestamp);
                        return true;
                    case 5:
                    case 24:
                        _voyage_info.insert(Voyage(m),m._timestamp);
                        return true;
                }
                return false;
            }
            
            bool hasPosition() const
            {
                return !_positions.empty();
            }
            
            Position const &latestPosition() const
            {
                return _positions._history.rbegin()->second;
            }
            
            Time latestPositionTime() const
            {
                return _positions._current_timestamp;
            }

            Position interpolatedPosition(Time t) const
            {
                if (t <= _positions._history.begin()->first)
                    return _positions._history.begin()->second;
                if (t >= _positions._history.rbegin()->first)
                    return _positions._history.rbegin()->second;
                return _positions._history.GetInterpolated(t);
            }
        };
        
    }
}

#endif
