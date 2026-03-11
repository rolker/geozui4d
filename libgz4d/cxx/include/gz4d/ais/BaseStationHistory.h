// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_AIS_BASESTATIONHISTORY_H
#define GZ4D_AIS_BASESTATIONHISTORY_H

#include "Message.h"
#include "MessageHistory.h"

namespace gz4d
{
    namespace ais
    {
        /// Collects base station reports and interogations.
        class BaseStationHistory
        {
            int _mmsi;
            TimeHistory _reports;
            
            
        public:
            BaseStationHistory():_mmsi(-1){}
            BaseStationHistory(int mmsi):_mmsi(mmsi){}
            
            /// @return: true on success.
            bool insert(Message const &m)
            {
                assert(m._message->mmsi > 0);
                if(_mmsi < 0)
                    _mmsi = m._message->mmsi;
                assert(m._message->mmsi == _mmsi);
                switch(m._message->message_id)
                {
                    case 4:
                        try
                        {
                            _reports.insert(DateTime(m),m._timestamp);
                            return true;
                        }
                        catch (...)
                        {
                            return false;
                        }
                    case 15:
                        return true;
                }
                return false;
            }
            
            bool hasReports() const
            {
                return !_reports.empty();
            }
            
            DateTime const &latestReport() const
            {
                return _reports._history.rbegin()->second;
            }
            
            Time latestReportTime() const
            {
                return _reports._current_timestamp;
            }
            
        };
        
    }
}

#endif
