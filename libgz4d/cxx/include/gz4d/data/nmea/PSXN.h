#ifndef GZ4D_DATA_NMEA_PSXN_H
#define GZ4D_DATA_NMEA_PSXN_H

#include <gz4d/data/nmea/Sentence.h>
#include <gz4d/util/Utilities.h>

namespace gz4d
{
    namespace nmea
    {
        struct PSXN
        {
            int type;
            int horizontal_quality;
            int vertical_quality;
            int heading_quality;
            int roll_pitch_quality;
            float gyro_calibration;
            float gyro_offset;
            float roll; ///< positive roll is right side down
            float pitch; ///< positive pitch is nose up
            float heading;
            float heave;

            PSXN()
            :type(-1),horizontal_quality(-1),vertical_quality(-1),heading_quality(-1),roll_pitch_quality(-1),gyro_calibration(Nan<float>()),gyro_offset(Nan<float>()),roll(Nan<float>()),pitch(Nan<float>()),heading(Nan<float>()),heave(Nan<float>())
            {
            }
            
            PSXN(Sentence const &s)
            :type(-1),horizontal_quality(-1),vertical_quality(-1),heading_quality(-1),roll_pitch_quality(-1),gyro_calibration(Nan<float>()),gyro_offset(Nan<float>()),roll(Nan<float>()),pitch(Nan<float>()),heading(Nan<float>()),heave(Nan<float>())
            {
                if(!s.IsType("PSXN"))
                    throw Exception("Invalid type");
                
                if(s.size() < 4)
                    throw Exception("Wrong size");
                
                type = boost::lexical_cast<int>(s[1]);
                
                switch(type)
                {
                    case 20:
                        if(s.size() != 6)
                            throw Exception("Wrong size");
                        if(!s[2].empty())
                            horizontal_quality = boost::lexical_cast<int>(s[2]);
                        if(!s[3].empty())
                            vertical_quality = boost::lexical_cast<int>(s[3]);
                        if(!s[4].empty())
                            heading_quality = boost::lexical_cast<int>(s[4]);
                        if(!s[5].empty())
                            roll_pitch_quality = boost::lexical_cast<int>(s[5]);
                        break;
                    case 22:
                        if(s.size() != 4)
                            throw Exception("Wrong size");
                        if(!s[2].empty())
                            gyro_calibration = boost::lexical_cast<float>(s[2]);
                        if(!s[3].empty())
                            gyro_offset = boost::lexical_cast<float>(s[3]);
                        break;
                    case 23:
                        if(s.size() != 6)
                            throw Exception("Wrong size");
                        if(!s[2].empty())
                            roll = boost::lexical_cast<float>(s[2]);
                        if(!s[3].empty())
                            pitch = boost::lexical_cast<float>(s[3]);
                        if(!s[4].empty())
                            heading = boost::lexical_cast<float>(s[4]);
                        if(!s[5].empty())
                            heave = boost::lexical_cast<float>(s[5]);
                        break;
                    default:
                        throw Exception("Unknown PSXN type");
                }
            }
            
        };
    }
}

#endif
