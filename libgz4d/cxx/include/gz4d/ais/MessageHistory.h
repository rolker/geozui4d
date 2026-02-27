#ifndef GZ4D_AIS_MESSAGEHISTORY_H
#define GZ4D_AIS_MESSAGEHISTORY_H

#include <gz4d/TimeSequence.h>
#include "Position.h"
#include "Voyage.h"
#include "DateTime.h"


namespace gz4d
{
    namespace ais
    {
        template <typename T> struct MessageHistory
        {
            TimeSequence<T> _history;
            Time _current_timestamp;
            
            typedef T value_type;
            
            void insert(T const &value, Time timestamp)
            {
                if(_history.empty())
                {
                    _history[timestamp] = value;
                    _current_timestamp = timestamp;
                }
                else
                {
                    assert(timestamp >= _current_timestamp);
                    if(timestamp > _current_timestamp)
                    {
                        T latest = _history.rbegin()->second;
                        if(value != latest)
                        {
                            _history[_current_timestamp] = latest;
                            _history[timestamp] = value;
                        }
                        _current_timestamp = timestamp;
                    }
                }
            }
            
            bool empty() const
            {
                return _history.empty();
            }
            
        };
        
        typedef MessageHistory<Position> PositionHistory;
        typedef MessageHistory<Voyage> VoyageHistory;
        typedef MessageHistory<DateTime> TimeHistory;
    }
}

#endif
