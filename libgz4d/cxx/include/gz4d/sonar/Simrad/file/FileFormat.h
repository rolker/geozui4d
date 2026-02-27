#ifndef GZ4D_SIMRAD_FILEFORMAT_H
#define GZ4D_SIMRAD_FILEFORMAT_H

#include<gz4d/sonar/Simrad.h>

/// \file
/// 
///         Vessel coordinate system
///         The ME70 Scientific multibeam echo sounder uses a right
///         hand coordinate system with positive X forward, positive Y to
///         starboard and positive Z down. This means that depth values
///         are positive.
///         Angles are de?ned as clockwise rotation in the positive direction
///         of the axis. 0 degrees is de?ned to be at the horizontal plane for
///         angles around the X and Y axis (roll and pitch). For example,
///         roll is around the x-axis, and a positive roll means that starboard
///         side goes down.
///         There are no restrictions to where the coordinate system's origo
///         is located in the X and Y directions. In the Z direction however,
///         the origo must be located on the water line.
///         Separate Array and Vessel coordinate systems are de?ned, but
///         coincide with each other. Deviation due to inaccuracies during
///         installation will be represented by minor differences between
///         these two coordinate systems.
///         The centre beam of the ME70 fan is at 0 degrees athwart beam
///         steering, beams towards the port side are located at angles of less
///         than 0 degrees (negative values), while beams towards starboard
///         side are located at angles higher than 0 degrees (positive values).
///         A vertical beam has alongship beam steering of 0 degrees. Beams
///         pointing to the aft are located at angles of less than 0 degrees
///         (negative values), while beams towards the bow are located at
///         angles higher than 0 degrees (positive values).
///     

#pragma pack(1)

namespace gz4d
{
    namespace Simrad
    {
        struct DatagramHeader
        {
            int32_t DatagramType;
            ///  contains a 64-bit integer value stating the number of 100 nanosecond intervals since January 1, 1601. This is the internal ?filetime? used by the Windows NT operating system.
            uint64_t DateTime;

            DatagramHeader()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct ConfigurationHeader
        {
            char SurveyName[128];
            char TransectName[128];
            char SounderName[128];
            char Version[30];
            int16_t Multiplexing;
            /// Difference between UTC and local time
            int32_t TimeBias;
            float SoundVelocityAverage;
            float SoundVelocityTransducer;
            char Spare[84];
            int32_t TransducerCount;

            ConfigurationHeader()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct ConfigurationBeam
        {
            char ChannelId[128];
            /// BeamType: This is a 32?bit integer, but only the last three bits are used.
            ///                 Bit 0: 0 = Single beam, 1 = Split beam
            ///                 Bit 1: 0 = Beam in fan, 1 = Reference beam
            ///                 Bit 2: 0 = Reference beam ?A?, 1 = Reference beam ?B?
            ///             
            int32_t BeamType;
            float Frequency;
            /// The single gain parameter was used actively in raw data files generated with software version 1.3. This was before PulseLengthTable, GainTable and SaCorrectionTable were introduced in software version 1.4 to enable gain and Sa correction parameters for each pulse length.
            float Gain;
            float EquivalentBeamAngle;
            float BeamWidthAlongship;
            float BeamWidthAthwartship;
            float AngleSensitivityAlongship;
            float AngleSensitivityAthwarthship;
            float AngleOffsetAlongship;
            float AngleOffsetAthwartship;
            /// future use
            float PosX;
            /// future use
            float PosY;
            /// future use
            float PosZ;
            /// Clockwise rotation around the Y axis. See Vessel coordinate system.
            ///             
            float BeamSteeringAngleAlongship;
            /// Counter clockwise rotation around the X axis. See Vessel coordinate system
            ///             
            float BeamSteeringAngleAthwartship;
            /// Not used
            float BeamSteeringAngleUnused;
            float PulseLengthTable[5];
            char Spare1[8];
            float GainTable[5];
            char Spare2[8];
            float SaCorrectionTable[5];
            char Spare3[8];
            char GPTSoftwareVersion[16];
            char Spare4[28];

            ConfigurationBeam()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Datagram
        {
            DatagramHeader DgHeader;

            Datagram()
            {
            }
            bool IsValid() const
            {
                return DgHeader.IsValid();
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct ConfigurationDatagram :Datagram
        {
            typedef ConfigurationBeam value_type;
            ConfigurationHeader ConfigHeader;
            ConfigurationBeam Beam[];

            ConfigurationDatagram()
            {
                DgHeader.DatagramType = 0x304e4f43;
            }
            bool IsValid() const
            {
                return Datagram::IsValid() && DgHeader.DatagramType == 0x304e4f43 && ConfigHeader.IsValid();
            }
            int32_t getArrayCount() const {return ConfigHeader.TransducerCount;}
        };

        /// 
        ///                     The CON1 datagram contain the description of the beam mode
        ///                     used during the recording of the ?le. The description is in terms
        ///                     of an XML text string containing the information from the beam
        ///                     con?guration XML formats.
        ///                 
        struct ConfigurationDatagram1 :Datagram
        {
            typedef char value_type;
            /// xml string
            char BeamConfiguration[];

            ConfigurationDatagram1()
            {
                DgHeader.DatagramType = 0x314e4f43;
            }
            bool IsValid() const
            {
                return Datagram::IsValid() && DgHeader.DatagramType == 0x314e4f43;
            }
        };

        /// The NMEA datagrams contain original NMEA 0183 input
        ///                     sentences.
        struct NMEADatagram :Datagram
        {
            typedef char value_type;
            char Text[];

            NMEADatagram()
            {
                DgHeader.DatagramType = 0x30454d4e;
            }
            bool IsValid() const
            {
                return Datagram::IsValid() && DgHeader.DatagramType == 0x30454d4e;
            }
        };

        /// The annotation datagram contains comment text.
        struct AnotationDatagram :Datagram
        {
            typedef char value_type;
            char Text[];

            AnotationDatagram()
            {
                DgHeader.DatagramType = 0x30474154;
            }
            bool IsValid() const
            {
                return Datagram::IsValid() && DgHeader.DatagramType == 0x30474154;
            }
        };

        /// The sample datagram contains data from just one beam channel.
        ///                      It can contain power sample data, or it can contain both power
        ///                      and angle sample data.
        ///                 
        struct SampleDatagram :Datagram
        {
            typedef int16_t value_type;
            /// Channel number
            int16_t Channel;
            /// Datatype: 1 = Power, 2 = Angle, 3 = Both
            int16_t Mode;
            float TransducerDepth;
            float Frequency;
            float TransmitPower;
            float PulseLength;
            float BandWidth;
            float SampleInterval;
            float SoundVelocity;
            float AbsorptionCoefficient;
            float Heave;
            float Roll;
            float Pitch;
            float Temperature;
            float Heading;
            /// 0 = Active, 1 = Passive, 2 = Test, -1 = Unknown
            int16_t TransmitMode;
            char Spare[6];
            /// First sample
            int32_t Offset;
            /// Number of samples
            int32_t Count;
            /// The power data contained in the sample datagram is compressed. In order to restore the correct value(s), you must decompress the value according to the equation below.
            ///                         y=x (10 log(2))/256
            ///                         where:
            ///                         x = power value derived from the datagram
            ///                         y = converted value (in dB)
            int16_t Power[];

            SampleDatagram()
            {
                DgHeader.DatagramType = 0x30574152;
            }
            bool IsValid() const
            {
                return Datagram::IsValid() && DgHeader.DatagramType == 0x30574152;
            }
            int32_t getArrayCount() const {return Count;}
        };

    }
}

#pragma pack()

#endif
