#ifndef EK60_H
#define EK60_H

#include "engine/ObjectCollection.h"
#include <map>
#include <gz4d/time.h>
#include <gz4d/sonar/Simrad/file/FileFormat.h>
#include <gz4d/data.h>

class EK60_Channel;

class EK60 : public ObjectCollection
{
    friend class EK60_Channel;

    struct Position
    {
        float x,y;
    };

    std::vector<unsigned char> packetData;
    std::vector<EK60_Channel*> channels;
    std::map<gz4d::Time,Position> positions;
    std::map<gz4d::Time,float> headings;
    double baseX, baseY;
    bool me70;

    void AddPosition(gz4d::Time t, double lat, double lon);
    void AddHeading(gz4d::Time t, float h);
    public:
        EK60();
        ~EK60();

        DERIVE_OBJECT(EK60);

        int Load();
        int perform(int argc, const char **argv, Tcl_Interp *interp);
        bool ReadPacket();
        bool GetPosition(gz4d::Time t, float &x, float &y);
        bool GetHeading(gz4d::Time t, float &h);
        bool IsME70(){return me70;}
};


#endif
