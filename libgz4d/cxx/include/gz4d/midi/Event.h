#ifndef GZ4D_MIDI_EVENT_H
#define GZ4D_MIDI_EVENT_H

#include <gz4d/gz4d.h>
#include <vector>
//#include <stdint.h>
#include <gz4d/time.h>

namespace gz4d
{
    namespace midi
    {
        class Event
        {
            gz4d::Time _timestamp;
            uint8_t _command;
            uint8_t _channel;
            std::vector<uint8_t> _data;
            public:
                Event(gz4d::Time t, uint8_t const * begin, uint8_t const *end)
                :_timestamp(t),_command(255),_channel(0)
                {
                    if(end > begin)
                    {
                        _command = (begin[0]&0x70)>>4;
                        _channel = begin[0]&0x0f;
                        _data.insert(_data.end(),&begin[1],end);
                    }
                }
                
                Event(Event const &e)
                :_timestamp(e._timestamp),_command(e._command),_channel(e._channel),_data(e._data)
                {
                }
                

                bool valid() const {return !(_command&0xf8);}
                gz4d::Time timestamp() const {return _timestamp;}
                uint8_t command() const {return _command;}
                uint8_t channel() const {return _channel;}
                std::vector<uint8_t> const &data() const {return _data;}
        };
    }
}

#endif
