#ifndef GZ4D_DATA_NMEA_PASHR_H
#define GZ4D_DATA_NMEA_PASHR_H

#include <gz4d/data/nmea/Sentence.h>
#include <gz4d/data/nmea/Time.h>

namespace gz4d
{
    namespace nmea
    {
        /// Attitude information.
        /// The Tate-Bryant convention defines roll relative to a plane made by
        /// the deck of the vessel after it has included the pitch angle of the
        /// vessel.
        struct PASHR: public Time
        {
            float heading;
            char heading_reference;
            
            /// Roll of the navigation system, measured in degrees,
            /// with leading sign, leading 0’s where needed and 2 decimal places.
            /// Positive values mean that the left side is up.
            float roll; 
            
            /// Pitch of the navigation system, measured in degrees,
            /// with leading sign, leading 0’s where needed and 2 decimal places.
            /// Positive values mean that the front is up.
            float pitch;
            
            float heave;
            float roll_accuracy;
            float pitch_accuracy;
            float heading_accuracy;
            int aiding_status;
            int imu_status;
            
            PASHR()
            :heading(std::numeric_limits<float>::quiet_NaN()),heading_reference(0),roll(std::numeric_limits<float>::quiet_NaN()),pitch(std::numeric_limits<float>::quiet_NaN()),heave(std::numeric_limits<float>::quiet_NaN()),roll_accuracy(std::numeric_limits<float>::quiet_NaN()),pitch_accuracy(std::numeric_limits<float>::quiet_NaN()),heading_accuracy(std::numeric_limits<float>::quiet_NaN()),aiding_status(-1),imu_status(-1)
            {
            }
            
            PASHR(Sentence const &s)
            :Time(s,1),heading(std::numeric_limits<float>::quiet_NaN()),heading_reference(0),roll(std::numeric_limits<float>::quiet_NaN()),pitch(std::numeric_limits<float>::quiet_NaN()),heave(std::numeric_limits<float>::quiet_NaN()),roll_accuracy(std::numeric_limits<float>::quiet_NaN()),pitch_accuracy(std::numeric_limits<float>::quiet_NaN()),heading_accuracy(std::numeric_limits<float>::quiet_NaN()),aiding_status(-1),imu_status(-1)
            {
                if(!s.IsType("PASHR"))
                    throw Exception("Invalid type");
                if(s.size() != 12)
                    throw Exception("Wrong size");
                
                heading = boost::lexical_cast<float>(s[2]);
                if(s[3].size()==1)
                    heading_reference = s[3][0];
                
                roll = boost::lexical_cast<float>(s[4]);
                pitch = boost::lexical_cast<float>(s[5]);
                heave = boost::lexical_cast<float>(s[6]);
                roll_accuracy = boost::lexical_cast<float>(s[7]);
                pitch_accuracy = boost::lexical_cast<float>(s[8]);
                heading_accuracy = boost::lexical_cast<float>(s[9]);
                aiding_status = boost::lexical_cast<int>(s[10]);
                imu_status = boost::lexical_cast<int>(s[11]);
            }
            
        };
    }
}

#endif
