#ifndef GZ4D_AIS_HISTORY_H
#define GZ4D_AIS_HISTORY_H

#include "MMSIHistory.h"
#include "ApplicationSpecificHistory.h"
#include "BaseStationHistory.h"
#include <gz4d/time/ios.h>
#include <set>

namespace gz4d
{
    namespace ais
    {
        class History
        {
        protected:
            typedef std::map<int,gz4d::ais::MMSIHistory> MMSIHistoryMap;
            MMSIHistoryMap _targets;
            
            typedef std::map<int,BaseStationHistory> BaseStationHistoryMap;
            BaseStationHistoryMap _baseStations;
            
            typedef std::map<int,ApplicationSpecificHistory> ApplicationSpecificHistoryMap;
            ApplicationSpecificHistoryMap _binarySources;
            
            typedef std::map<BinaryMessageType,std::set<int> > BinaryMessageTypeMap;
            BinaryMessageTypeMap _binarySourcesByType;
            
            gz4d::time::Period _timePeriod;
        public:
            /// Insert an AIS message to the history.
            /// @return: True on success
            bool insert(Message const &m)
            {
                if(m._message->mmsi <= 0)
                    return false;

                if(m._timestamp.valid())
                    if(_timePeriod.valid())
                        _timePeriod|=m._timestamp;
                    else
                        _timePeriod = time::Period(m._timestamp,m._timestamp);
                
                switch(m._message->message_id)
                {
                    case 1:
                    case 2:
                    case 3:
                    case 5:
                    case 18:
                    case 24:
                        return _targets[m._message->mmsi].insert(m);
                        
                    case 4:
                    case 15:
                        return _baseStations[m._message->mmsi].insert(m);

                    case 8:
                    {
                        std::shared_ptr<Ais8> ais8 = std::dynamic_pointer_cast<Ais8>(m._message);
                        _binarySourcesByType[BinaryMessageType(ais8->dac,ais8->fi)].insert(ais8->mmsi);
                        return _binarySources[m._message->mmsi].insert(m);
                    }
                    
                }

                return false;
            }
            
            std::string report() const
            {
                std::stringstream ret;
                ret << time::Now() << std::endl;
                ret << "\t" << _targets.size() << " targets\n";
                ret << "\t" << _baseStations.size() << " base stations\n";
                ret << "\t" << _binarySources.size() << " binary sources\n";
                int lastDac = 0;
                for(BinaryMessageTypeMap::const_iterator t = _binarySourcesByType.begin(); t != _binarySourcesByType.end(); ++t)
                {
                    if(t->first.first != lastDac)
                    {
                        lastDac = t->first.first;
                        ret << "\t\tdac: " << lastDac << " (" << ApplicationSpecificHistory::dacToCountryStr(lastDac) << ")\n";
                    }
                    ret << "\t\t\tfi: " << t->first.second << "\t" << t->second.size() << " sources\n";
                }
                
                return ret.str();
            }
            
            time::Period const & getTimePeriod() const {return _timePeriod;}
        };
    }
}

#endif
