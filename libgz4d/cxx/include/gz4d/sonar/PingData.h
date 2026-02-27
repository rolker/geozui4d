#ifndef GZ4D_SONAR_PINGDATA_H
#define GZ4D_SONAR_PINGDATA_H

#include <gz4d/gz4d.h>
#include <vector>

namespace gz4d
{
    namespace sonar
    {
        template <typename CT> class PingData
        {
            public:
                typedef std::vector<CT> channels_type;
                channels_type &GetChannels() {return channels;}
            protected:
                channels_type channels;
        };
    }
}

#endif
