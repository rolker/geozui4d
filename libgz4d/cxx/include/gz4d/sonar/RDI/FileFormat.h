// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_RDI_FILEFORMAT_H
#define GZ4D_SONAR_RDI_FILEFORMAT_H

#include<gz4d/sonar/Sonar.h>

#pragma pack(1)

namespace gz4d
{
    namespace RDI
    {
        struct OldHeader
        {
            uint16_t EnsembleSize;
            uint16_t LeaderSize;
            uint16_t VelocitySize;
            uint16_t SpectralSize;
            uint16_t EchoSize;
            uint16_t PercentGoodSize;
            uint16_t StatusSize;

            OldHeader()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct OldLeader
        {
            uint8_t rtc_month;
            uint8_t rtc_day;
            uint8_t rtc_hour;
            uint8_t rtc_minute;
            uint8_t rtc_second;
            uint8_t tbp_minute;
            uint8_t tbp_second;
            uint8_t tbp_hundreds_second;
            uint16_t ping_per_ensemble;
            uint8_t bin_count;
            uint8_t bin_length;
            uint8_t pulse_length;
            uint8_t blank_after_transmit;
            uint8_t delay_after_blank;
            uint16_t ensemble_number;
            uint8_t bit_status;
            uint8_t adcp_config;
            uint8_t sn_threshold;
            uint8_t pg_threshold;
            uint16_t pitch;
            uint16_t roll;
            uint16_t heading;
            uint16_t adcp_temperature;
            uint8_t hi_voltage_in;
            uint8_t transmit_current;
            uint8_t low_voltage_in;
            uint8_t ctd_conductivity_count[3];
            uint8_t ctd_temperature_count[3];
            uint8_t ctd_depth_count[3];
            uint8_t bottom_tracking_velocity[6];
            uint16_t beam_range[4];
            uint8_t pitch_stdandard_deviation;
            uint8_t roll_stdandard_deviation;
            uint8_t heading_stdandard_deviation;
            uint8_t ctd_measurement_interval[3];
            uint8_t bottom_tracking_pg[2];

            OldLeader()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct OldEnsemble
        {
            typedef uint8_t value_type;
            OldHeader header;
            OldLeader leader;
            uint8_t velocity[];

            OldEnsemble()
            {
            }
            bool IsValid() const
            {
                return header.IsValid() && leader.IsValid();
            }
            uint16_t getArrayCount() const {return header.VelocitySize;}
        };

        /// The ADCP packs velocity data for each depth cell of each beam into a twobyte, two's-complement integer [-32768, 32767] with the LSB sent first. The ADCP scales velocity data in millimeters per second (mm/s). A value of -32768 (8000h) indicates bad velocity values.
        struct VelocityCell
        {
            int16_t Velocity[4];

            VelocityCell()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        /// 
        ///         Correlation magnitude data for Broad Bandwidth ensembles give the magnitude of the normalized echo autocorrelation at the lag used for estimating the Doppler phase change. The ADCP represents this magnitude by a linear scale between 0 and 255, where 255 is perfect correlation (i.e., a solid target).
        ///         Correlation magnitude data for Narrow Bandwidth ensembles give the magnitude of the energy (power) in the low pass filter. Values of 170 to 190 counts represent normal levels. Lower values mean a reduced signal to noise ratio.
        ///         
        struct CorrelationMagnitudeCell
        {
            uint8_t CorrelationMagnitude[4];

            CorrelationMagnitudeCell()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        /// 
        ///         The echo intensity scale factor is about 0.45 dB per ADCP count. The ADCP does not directly check for the validity of echo intensity data.
        ///         
        struct EchoIntensityCell
        {
            uint8_t EchoIntensity[4];

            EchoIntensityCell()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        /// 
        ///         The percent-good data field is a data-quality indicator that reports the percentage
        ///         (0 to 100) of good data collected for each depth cell of the velocity
        ///         profile. The setting of the EX-command (?EX - Coordinate Transformation,?
        ///         page 122) determines how the ADCP references percent-good data as
        ///         shown below.
        /// 
        ///         EX-Command      Coordinate      Velocity 1              Velocity 2              Velocity 3              Velocity 4
        ///                         System
        ///                                                                 Percentage Of Good Pings For:
        ///                                         Beam 1                  BEAM 2                  BEAM 3                  BEAM 4
        ///         xxx00xxx        Beam                                                    Percentage Of:
        ///         xxx01xxx        Instrument      3-Beam Transfor-        Transformations         More Than One           4-Beam Trans-
        ///                                         mations (note 1)        Rejected (note 2)       Beam Bad In Bin         formations
        ///         xxx10xxx        Ship
        ///         xxx11xxx        Earth
        /// 
        ///         1. Because profile data did not exceed correlation threshold (?WC - Broad Bandwidth Profiling Water Correlation Threshold,? page 140).
        ///         2. Because the error velocity threshold was exceeded (?WE - Broad Bandwidth Profiling Error Velocity Threshold,? page 142).
        /// 
        ///         At the start of the velocity profile, the backscatter echo strength is typically
        ///         high on all four beams. Under this condition, the ADCP uses all four beams
        ///         to calculate the orthogonal and error velocities. As the echo returns from
        ///         far away depth cells, echo intensity decreases. At some point, the echo will
        ///         be weak enough on any given beam to cause the ADCP to reject some of its
        ///         depth cell data. This causes the ADCP to calculate velocities with three
        ///         beams instead of four beams. When the ADCP does 3-beam solutions, it
        ///         stops calculating the error velocity because it needs four beams to do this.
        ///         At some further depth cell, the ADCP rejects all cell data because of the
        ///         weak echo. As an example, let us assume depth cell 60 has returned the following
        ///         percent-good data.
        ///         
        ///         FIELD #1 = 50, FIELD #2 = 5, FIELD #3 = 0, FIELD #4 = 45
        ///         
        ///         If the EX-command was set to collect velocities in BEAM coordinates, the
        ///         example values show the percentage of pings having good solutions in cell
        ///         60 for each beam based on the Low Correlation Threshold (WC-command).
        ///         Here, beam 1=50%, beam 2=5%, beam 3=0%, and beam 4=45%. These are
        ///         neither typical nor desired percentages. Typically, you would want all four
        ///         beams to be about equal and greater than 25%.
        ///         On the other hand, if velocities were collected in Instrument, Ship, or Earth
        ///         coordinates, the example values show:
        ///         
        ///         Field 1 ? Percentage of good 3-beam solutions ? Shows percentage of successful
        ///         velocity calculations (50%) using 3-beam solutions because the correlation
        ///         threshold (WC) was not exceeded.
        ///         
        ///         Field 2 ? Percentage of transformations rejected ? Shows percent of error
        ///         velocity (5%) that was less than the WE-command setting. WE has a default
        ///         of 5000 mm/s. This large WE setting effectively prevents the ADCP
        ///         from rejecting data based on error velocity.
        ///         
        ///         Field 3 ? Percentage of more than one beam bad in bin ? 0% of the velocity
        ///         data were rejected because not enough beams had good data.
        ///         
        ///         Field 4 ? Percentage of good 4-beam solutions ? 45% of the velocity data
        ///         collected during the ensemble for depth cell 60 were calculated using four
        ///         beams.
        ///         
        struct PercentGoodCell
        {
            uint8_t PercentGood[4];

            PercentGoodCell()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Attitude
        {
            uint16_t Heading;
            uint16_t Pitch;
            uint16_t Roll;
            uint16_t HeadingRate;
            uint16_t PitchRate;
            uint16_t RollRate;

            Attitude()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct Header
        {
            typedef uint16_t value_type;
            uint8_t HeaderID;
            uint8_t DataSourceID;
            uint16_t EnsembleSize;
            uint8_t Spare;
            uint8_t DataCount;
            uint16_t Offsets[];

            Header()
            {
            }
            bool IsValid() const
            {
                return HeaderID == 0x7f && DataSourceID == 0x7f;
            }
            uint8_t getArrayCount() const {return DataCount;}
        };

        struct DataSection
        {
            uint16_t SectionID;

            DataSection()
            {
            }
            bool IsValid() const
            {
                return true;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct FixedLeader :DataSection
        {
            uint8_t CPUFirmwareVersion;
            uint8_t CPUFirmwareRevision;
            /// 
            ///                     This field defines the ADCP hardware configuration. Convert
            ///                     this field (2 bytes, LSB first) to binary and interpret as follows.
            ///                     LSB
            ///                     BITS 7 6 5 4 3 2 1 0
            ///                          - - - - - 0 0 0 75-kHz SYSTEM
            ///                          - - - - - 0 0 1 150-kHz SYSTEM
            ///                          - - - - - 0 1 0 300-kHz SYSTEM
            ///                          - - - - - 0 1 1 600-kHz SYSTEM
            ///                          - - - - - 1 0 0 1200-kHz SYSTEM
            ///                          - - - - - 1 0 1 2400-kHz SYSTEM
            ///                          - - - - - 1 1 0 38-kHz SYSTEM
            ///                          - - - - 0 - - - CONCAVE BEAM PAT.
            ///                          - - - - 1 - - - CONVEX BEAM PAT.
            ///                          - - 0 0 - - - - SENSOR CONFIG #1
            ///                          - - 0 1 - - - - SENSOR CONFIG #2
            ///                          - - 1 0 - - - - SENSOR CONFIG #3
            ///                          - 0 - - - - - - XDCR HD NOT ATT.
            ///                          - 1 - - - - - - XDCR HD ATTACHED
            ///                          0 - - - - - - - DOWN FACING BEAM
            ///                          1 - - - - - - - UP-FACING BEAM
            ///                     MSB
            ///                     BITS 7 6 5 4 3 2 1 0
            ///                          - - - - - - 0 0 15E BEAM ANGLE
            ///                          - - - - - - 0 1 20E BEAM ANGLE
            ///                          - - - - - - 1 0 30E BEAM ANGLE
            ///                          - - - - - - 1 1 OTHER BEAM ANGLE
            ///                          0 1 0 0 - - - - 4-BEAM JANUS CONFIG
            ///                          0 1 0 1 - - - - 5-BM JANUS CFIG DEMOD)
            ///                          1 1 1 1 - - - - 5-BM JANUS CFIG.(2 DEMD)
            ///                     Example: Hex 5249 (i.e., hex 49 followed by hex 52) identifies
            ///                     a 150-kHz system, convex beam pattern, down-facing,
            ///                     30E beam angle, 5 beams (3 demods).
            ///                     
            uint16_t SystemConfiguration;
            uint16_t Reserved;
            uint8_t NumberOfBeams;
            uint8_t NumberOfCells;
            uint16_t NumberOfPings;
            /// 
            ///                     Contains the length of one depth cell (?WS - Broad Bandwidth
            ///                     Profiling Depth Cell Size,? page 143).
            ///                     Scaling: LSD = 1 centimeter; Range = 1 to 6400 cm (210 feet)
            ///                     
            uint16_t DepthCellLength;
            /// 
            ///                     Contains the blanking distance used by the ADCP to allow the
            ///                     transmit circuits time to recover before the receive cycle begins
            ///                     (?WF - Broad Bandwidth Profiling Blank after Transmit,?
            ///                     page 142).
            ///                     Scaling: LSD = 1 centimeter; Range = 0 to 9999 cm (328
            ///                     feet)
            ///                     
            uint16_t BlankAfterTransmit;
            uint8_t SignalProcessingMode;
            uint8_t BroadBandwidthProfilingWaterCorrelationThreshold;
            uint8_t NumberCodeReps;
            uint8_t PercentGoodMinimum;
            uint16_t ErrorVelocityMaximum;
            uint8_t TPPMinutes;
            uint8_t TPPSeconds;
            uint8_t TPPHundredths;
            /// 
            ///                     Contains the coordinate transformation processing parameters
            ///                     (?EX - Coordinate Transformation,? page 122). These
            ///                     firmware switches indicate how the Ocean Surveyor collected
            ///                     data.
            ///                     xxx00xxx = NO TRANSFORMATION (BEAM COORDINATES)
            ///                     xxx01xxx = INSTRUMENT COORDINATES
            ///                     xxx10xxx = SHIP COORDINATES
            ///                     xxx11xxx = EARTH COORDINATES
            ///                     xxxxx1xx = TILTS (PITCH AND ROLL) USED IN SHIP
            ///                     OR EARTH TRANSFORMATION
            ///                     xxxxxx1x = 3-BEAM SOLUTION USED IF ONE BEAM IS
            ///                     BELOW THE CORRELATION THRESHOLD SET
            ///                     BY THE WC-COMMAND
            ///                     xxxxxxx1 = BIN MAPPING USED
            ///                     
            uint8_t CoordinateTransformation;
            /// 
            ///                     Contains a correction factor for physical heading misalignment
            ///                     (?EA - Heading Alignment,? page 111).
            ///                     Scaling: LSD = 0.01 degree; Range = -179.99 to 180.00 degrees
            ///                     
            uint16_t HeadingAlignment;
            /// 
            ///                     Contains a correction factor for electrical/magnetic heading
            ///                     bias (?EV - Heading Bias,? page 120).
            ///                     Scaling: LSD = 0.01 degree; Range = -179.99 to 180.00
            ///                     degrees
            ///                     
            uint16_t HeadingBias;
            /// 
            ///                     Contains the selected source of environmental sensor data
            ///                     (?EZ - Sensor Source,? page 123). These firmware switches
            ///                     indicate the following.
            ///                     Field      Description
            ///                     x1xxxxxx = calculates EC from ED, ES, and ET
            ///                     xx1xxxxx = uses ED from depth sensor
            ///                     xxx1xxxx = uses EH from transducer heading sensor
            ///                     xxxx1xxx = uses EP from transducer pitch sensor
            ///                     xxxxx1xx = uses ER from transducer roll sensor
            ///                     xxxxxx1x = uses ES from conductivity sensor
            ///                     xxxxxxx1 = uses ET from transducer temp sensor
            ///                     NOTE: If the field = 0, or if the sensor is not available, the
            ///                     ADCP uses the manual command setting. If the field = 1, the
            ///                     ADCP uses the reading from the internal sensor or an external
            ///                     synchro sensor (only applicable to heading, roll, and
            ///                     pitch). Although you can enter a "2" in the EZ-command
            ///                     string, the ADCP only displays a 0 (manual) or 1 (int/ext sensor).
            ///                     
            uint8_t SensorSource;
            /// 
            ///                     This field reflects what sensors are available. The bit pattern
            ///                     is the same as listed for the EZ-command (above), except
            ///                     that the EC bit is always zero because there is no speed of
            ///                     sound sensor.
            ///                     
            uint8_t SensorsAvail;
            /// 
            ///                     This field contains the distance to the middle of the first depth
            ///                     cell (bin). This distance is a function of depth cell length
            ///                     (WS), the profiling mode (WM), the blank after transmit distance
            ///                     (WF), and speed of sound.
            ///                     
            uint16_t Bin1Distance;
            /// 
            ///                     This field, set by the WT-command, contains the length of the
            ///                     transmit pulse. When the ADCP receives a <BREAK> signal,
            ///                     it sets the transmit pulse length as close as possible to the
            ///                     depth cell length (WS-command). This means the ADCP
            ///                     uses a WT command of zero. However, the WT field contains
            ///                     the actual length of the transmit pulse used.
            ///                     Scaling: LSD = 1 centimeter; Range = 0 to 65535 cm (2150
            ///                     feet)
            ///                     
            uint16_t XmitPulseLength;
            uint16_t Reserved1;
            uint8_t FalseTargetThresh;
            uint8_t Reserved2;
            /// 
            ///                     This field, determined mainly by the setting of the WMcommand,
            ///                     contains the distance between pulse repetitions.
            ///                     Scaling: LSD = 1 centimeter; Range = 0 to 65535 centimeters
            ///                     
            uint16_t TransmitLagDistance;
            uint8_t Reserved3[8];

            FixedLeader()
            {
                SectionID = 0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableLeader :DataSection
        {
            uint16_t EnsembleNumber;
            uint8_t RTCYear;
            uint8_t RTCMonth;
            uint8_t RTCDay;
            uint8_t RTCHour;
            uint8_t RTCMinute;
            uint8_t RTCSecond;
            uint8_t RTCHundreths;
            uint8_t EnsembleNumberMSB;
            uint16_t Reserved;
            uint16_t SpeedOfSound;
            /// 
            ///                     Contains the depth of the transducer below the water surface
            ///                     (?ED - Depth of Transducer,? page 113). This value may be a
            ///                     manual setting or a reading from a depth sensor.
            ///                     Scaling: LSD = 1 decimeter; Range = 1 to 9999 decimeters
            ///                     
            uint16_t DepthOfTransducer;
            /// 
            ///                     Contains the ADCP heading angle (?EH - Heading Angle,?
            ///                     page 116). This value may be a manual setting or a reading
            ///                     from a heading sensor.
            ///                     Scaling: LSD = 0.01 degree; Range = 000.00 to 359.99 degrees
            ///                     
            uint16_t Heading;
            /// 
            ///                     Contains the ADCP pitch angle (?EP - Pitch and Roll Angles,?
            ///                     page 118). This value may be a manual setting or a reading
            ///                     from a tilt sensor. Positive values mean that Beam #3 is spatially
            ///                     higher than Beam #4.
            ///                     Scaling: LSD = 0.01 degree; Range = -179.00 to +180.00
            ///                     degrees
            ///                     
            uint16_t Pitch;
            /// 
            ///                     Contains the ADCP roll angle (?ER - Roll (Tilt 2),? page 119).
            ///                     This value may be a manual setting or a reading from a tilt
            ///                     sensor. For up-facing ADCPs, positive values mean that
            ///                     Beam #2 is spatially higher than Beam #1. For down-facing
            ///                     ADCPs, positive values mean that Beam #1 is spatially higher
            ///                     than Beam #2.
            ///                     Scaling: LSD = 0.01 degree; Range = -60.00 to +60.00 degrees
            ///                     
            uint16_t Roll;
            /// 
            ///                     Contains the salinity value of the water at the transducer head
            ///                     (?ES - Salinity,? page 119). This value may be a manual setting
            ///                     or a reading from a conductivity sensor.
            ///                     Scaling: LSD = 1 part per thousand; Range = 0 to 50 ppt
            ///                     
            uint16_t Salinity;
            /// 
            ///                     Contains the temperature of the water at the transducer head
            ///                     (?ET - Temperature,? page 119). This value may be a manual
            ///                     setting or a reading from a temperature sensor.
            ///                     Scaling: LSD = 0.01 degree; Range = -5.00 to +40.00 degrees
            ///                     
            uint16_t Temperature;
            /// 
            ///                     Minimum time between ping groups (mm:ss.ff)
            ///                     
            uint8_t Minutes;
            /// 
            ///                     Minimum time between ping groups (mm:ss.ff)
            ///                     
            uint8_t Seconds;
            /// 
            ///                     Minimum time between ping groups (mm:ss.ff)
            ///                     
            uint8_t Hundredths;
            uint8_t HeadingStandardDeviation;
            uint8_t PitchStandardDeviation;
            uint8_t RollStandardDeviation;
            /// 
            ///                     Contains the Error Status Word (ESW). The ESW is cleared
            ///                     (set to zero) between each ensemble.
            ///                     LSB
            ///                     BITS 07 06 05 04 03 02 01 00
            ///                          x  x  x  x  x  x  x  1  Xmit Shutdown
            ///                          x  x  x  x  x  x  1  x  Xmit Over Current
            ///                          x  x  x  x  x  1  x  x  Not used
            ///                          x  x  x  x  1  x  x  x  Not used
            ///                          x  x  x  1  x  x  x  x  Not used
            ///                          x  x  1  x  x  x  x  x  Not used
            ///                          x  1  x  x  x  x  x  x  Not used
            ///                          1  x  x  x  x  x  x  x  Not used
            ///                     MSB
            ///                     BITS 15 14 13 12 11 10 09 08
            ///                          x  x  x  x  x  x  x  1  Not Used
            ///                          x  x  x  x  x  x  1  x  Watchdog Disabled
            ///                          x  x  x  x  x  1  x  x  Not Used
            ///                          x  x  x  x  1  x  x  x  Not Used
            ///                          x  x  x  1  x  x  x  x  Not Used
            ///                          x  x  1  x  x  x  x  x  Not Used
            ///                          x  1  x  x  x  x  x  x  Not used
            ///                          1  x  x  x  x  x  x  x  Not used
            ///                     LSB
            ///                     BITS 07 06 05 04 03 02 01 00
            ///                          x  x  x  x  x  x  x  1  Not used
            ///                          x  x  x  x  x  x  1  x  Not used
            ///                          x  x  x  x  x  1  x  x  Not used
            ///                          x  x  x  x  1  x  x  x  Not used
            ///                          x  x  x  1  x  x  x  x  Not used
            ///                          x  x  1  x  x  x  x  x  Not used
            ///                          x  1  x  x  x  x  x  x  Not used
            ///                          1  x  x  x  x  x  x  x  Not used
            ///                     MSB
            ///                     BITS 15 14 13 12 11 10 09 08
            ///                          x  x  x  x  x  x  x  1  Not Used
            ///                          x  x  x  x  x  x  1  x  Not used
            ///                          x  x  x  x  x  1  x  x  Not Used
            ///                          x  x  x  x  1  x  x  x  Not Used
            ///                          x  x  x  1  x  x  x  x  Not Used
            ///                          x  x  1  x  x  x  x  x  Not Used
            ///                          x  1  x  x  x  x  x  x  Not used
            ///                          1  x  x  x  x  x  x  x  Not used
            ///                     
            uint8_t ErrorStatusWord[4];
            uint8_t Reserved2[14];

            VariableLeader()
            {
                SectionID = 0x0080;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x0080;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VelocityData :DataSection
        {
            typedef VelocityCell value_type;
            VelocityCell cell[];

            VelocityData()
            {
                SectionID = 0x0100;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x0100;
            }
        };

        struct CorrelationMagnitudeData :DataSection
        {
            typedef CorrelationMagnitudeCell value_type;
            CorrelationMagnitudeCell cell[];

            CorrelationMagnitudeData()
            {
                SectionID = 0x0200;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x0200;
            }
        };

        struct EchoIntensityData :DataSection
        {
            typedef EchoIntensityCell value_type;
            EchoIntensityCell cell[];

            EchoIntensityData()
            {
                SectionID = 0x0300;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x0300;
            }
        };

        struct PercentGoodData :DataSection
        {
            typedef PercentGoodCell value_type;
            PercentGoodCell cell[];

            PercentGoodData()
            {
                SectionID = 0x0400;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x0400;
            }
        };

        struct BottomTrackData :DataSection
        {
            uint16_t NumberOfPings;
            uint16_t Reserved1;
            uint8_t CorrelationMagnitudeMinimum;
            uint8_t EvaluationAmplitudeMinimum;
            uint16_t Reserved2;
            uint8_t Mode;
            uint16_t ErrorVelocityMaximum;
            uint8_t Reserved3[4];
            /// 
            ///                     Contains the two lower bytes of the vertical range
            ///                     from the ADCP to the sea bottom (or surface) as
            ///                     determined by each beam. This vertical range does
            ///                     not consider the effects of pitch and roll. When bottom
            ///                     detections are bad, BT Range = 0. See bytes
            ///                     78 through 81 for MSB description and scaling.
            ///                     Scaling: LSD = 1 cm; Range = 0 to 65535 cm
            ///                     
            uint16_t Range[4];
            /// 
            ///                     The meaning of the velocity depends on the EX
            ///                     command setting (?EX - Coordinate Transformation,?
            ///                     page 122). The four velocities are as follows:
            ///                     a) Beam Coordinates: Beam 1, Beam 2, Beam 3,
            ///                     Beam 4
            ///                     b) Instrument Coordinates: 1->2, 4->3, toward face,
            ///                     error
            ///                     c) Ship Coordinates: Stbd, Fwd, Upward, Error
            ///                     d) Earth Coordinates: East, North, Upward, Error
            ///                     
            int16_t Velocity[4];
            /// 
            ///                     Contains the correlation magnitude in relation to the
            ///                     sea bottom (or surface) as determined by each
            ///                     beam. Bottom-track correlation magnitudes have
            ///                     the same format and scale factor as water-profiling
            ///                     magnitudes.
            ///                     
            uint8_t Correlation[4];
            /// 
            ///                     Contains the evaluation amplitude of the matching
            ///                     filter used in determining the strength of the bottom
            ///                     echo.
            ///                     Scaling: LSD = 1 count; Range = 0 to 255 counts
            ///                     
            uint8_t EvaluationAmplitude[4];
            uint8_t Reserved4[4];
            uint8_t Reserved5[6];
            uint8_t Reserved6[20];
            /// 
            ///                     Stores the maximum tracking depth value (?BX - Maximum Tracking Depth,? page 102).
            ///                     Scaling: LSD = 1 decimeter; Range = 80 to 9999 decimeters
            ///                     
            uint16_t MaxDepth;
            /// 
            ///                     Contains the Receiver Signal Strength Indicator (RSSI) value in the center of the bottom echo as determined by each beam.
            ///                     Scaling: LSD = ? 0.45 dB per count; Range = 0 to 255 counts
            ///                     
            uint8_t ReceiverSignalStrengthIndicator[4];
            /// Contains the Gain level for shallow water.
            uint8_t Gain;
            /// 
            ///                     Contains the most significant byte of the vertical
            ///                     range from the ADCP to the sea bottom (or surface)
            ///                     as determined by each beam. This vertical range
            ///                     does not consider the effects of pitch and roll. When
            ///                     bottom detections are bad, BT Range=0. See bytes
            ///                     17 through 24 for LSB description and scaling.
            ///                     Scaling: LSD = 65,536 cm, Range = 65,536 to
            ///                     16,777,215 cm
            ///                     
            uint8_t RangeMSB[4];

            BottomTrackData()
            {
                SectionID = 0x0600;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x0600;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct FixedAttitudeData :DataSection
        {
            uint8_t AttitudeOutputCoordinates[8];
            uint8_t ExternalPitchRollScaling;
            uint8_t FixedHeadingScalling[3];
            uint16_t RollMisalignment;
            uint16_t PitchMisalignment;
            uint8_t PitchRollAndCoordinateFrame[5];
            uint8_t Orientation;
            uint16_t HeadingOffset;
            uint8_t SensorSource[8];

            FixedAttitudeData()
            {
                SectionID = 0x3000;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3000;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData40 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData40()
            {
                SectionID = 0x3040;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3040;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData44 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData44()
            {
                SectionID = 0x3044;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3044;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData48 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData48()
            {
                SectionID = 0x3048;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3048;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData4C :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData4C()
            {
                SectionID = 0x304C;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x304C;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData50 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData50()
            {
                SectionID = 0x3050;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3050;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData54 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData54()
            {
                SectionID = 0x3054;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3054;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData58 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData58()
            {
                SectionID = 0x3058;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3058;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData5C :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData5C()
            {
                SectionID = 0x305C;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x305C;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData60 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData60()
            {
                SectionID = 0x3060;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3060;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData64 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData64()
            {
                SectionID = 0x3064;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3064;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData68 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData68()
            {
                SectionID = 0x3068;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3068;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData6C :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData6C()
            {
                SectionID = 0x306C;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x306C;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData70 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData70()
            {
                SectionID = 0x3070;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3070;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData74 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData74()
            {
                SectionID = 0x3074;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3074;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData78 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData78()
            {
                SectionID = 0x3078;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3078;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData7C :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData7C()
            {
                SectionID = 0x307C;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x307C;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData80 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData80()
            {
                SectionID = 0x3080;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3080;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData84 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData84()
            {
                SectionID = 0x3084;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3084;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData88 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData88()
            {
                SectionID = 0x3088;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3088;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData8C :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData8C()
            {
                SectionID = 0x308C;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x308C;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData90 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData90()
            {
                SectionID = 0x3090;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3090;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData94 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData94()
            {
                SectionID = 0x3094;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3094;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData98 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData98()
            {
                SectionID = 0x3098;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x3098;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeData9C :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeData9C()
            {
                SectionID = 0x309C;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x309C;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataA0 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataA0()
            {
                SectionID = 0x30A0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30A0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataA4 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataA4()
            {
                SectionID = 0x30A4;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30A4;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataA8 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataA8()
            {
                SectionID = 0x30A8;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30A8;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataAC :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataAC()
            {
                SectionID = 0x30AC;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30AC;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataB0 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataB0()
            {
                SectionID = 0x30B0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30B0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataB4 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataB4()
            {
                SectionID = 0x30B4;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30B4;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataB8 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataB8()
            {
                SectionID = 0x30B8;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30B8;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataBC :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataBC()
            {
                SectionID = 0x30BC;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30BC;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataC0 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataC0()
            {
                SectionID = 0x30C0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30C0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataC4 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataC4()
            {
                SectionID = 0x30C4;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30C4;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataC8 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataC8()
            {
                SectionID = 0x30C8;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30C8;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataCC :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataCC()
            {
                SectionID = 0x30CC;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30CC;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataD0 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataD0()
            {
                SectionID = 0x30D0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30D0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataD4 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataD4()
            {
                SectionID = 0x30D4;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30D4;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataD8 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataD8()
            {
                SectionID = 0x30D8;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30D8;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataDC :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataDC()
            {
                SectionID = 0x30DC;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30DC;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataE0 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataE0()
            {
                SectionID = 0x30E0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30E0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataE4 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataE4()
            {
                SectionID = 0x30E4;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30E4;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataE8 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataE8()
            {
                SectionID = 0x30E8;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30E8;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataEC :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataEC()
            {
                SectionID = 0x30EC;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30EC;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataF0 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataF0()
            {
                SectionID = 0x30F0;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30F0;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataF4 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataF4()
            {
                SectionID = 0x30F4;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30F4;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataF8 :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataF8()
            {
                SectionID = 0x30F8;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30F8;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        struct VariableAttitudeDataFC :DataSection
        {
            Attitude Attitudes[8];

            VariableAttitudeDataFC()
            {
                SectionID = 0x30FC;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x30FC;
            }
            uint16_t getArrayCount() const {return 0;}
        };

        /// 
        ///                 In the BAM (Binary Angular Measure) format, the most
        ///                 significant bit of the word has a weight of 180 degrees,
        ///                 and you keep dividing by 2 as you proceed to the right.
        ///                 The lease significant bit for a 32-bit BAM is about 8E-8
        ///                 arc degrees (180/231), or just under 1 cm of longitudinal
        ///                 distance at the equator, where 1 arc minute = 1 Nautical
        ///                 mile. If you interpret the BAM word as an unsigned
        ///                 number, the range is 0 to (360-lsb) degrees, and if you
        ///                 interpret the BAM as a signed number, the range is ?
        ///                 180 to (180-lsb) degrees. The least significant bit for a
        ///                 16-bit BAM is about 0.0055 degrees (180/215). Some
        ///                 32-bit BAM examples are:
        ///                 UNSIGNED
        ///                 0x40000000 90 degrees
        ///                 0x80000000 180 degrees
        ///                 0xC0000000 270 degrees
        ///                 0xFFFFFFFF 360 degrees minus one LSB degrees
        ///                 SIGNED
        ///                 0x40000000 90 degrees
        ///                 0x80000000 minus 180 degrees
        ///                 0xC0000000 minus 90 degrees
        ///                 0xFFFFFFFF minus one LSB degrees
        /// 
        ///                 The Speed Made Good (SMG) and Direction Made
        ///                 Good (DMG) quantities are calculated from the navigation
        ///                 fixes that enter the system between ADCP outputs,
        ///                 and are calculated as follows:
        ///                 If:
        ///                 aLat(i) = the average of the latitudes of the nav fixes in
        ///                 interval i
        ///                 aLon(i) = the average of the longitudes of the nav fixes
        ///                 in interval i
        ///                 Ta(i) = the average of the time of validity of the nav
        ///                 fixes in interval i
        ///                 dLat = the difference in average latitude between averaging
        ///                 intervals
        ///                 dLon = the difference in average longitude between
        ///                 averaging intervals
        ///                 VelMGn (i) = the velocity made good in the North direction
        ///                 for interval i
        ///                 VelMGe (i) = the velocity made good in the East direction
        ///                 for interval i
        ///                 LatToDist (dLat) is a function that converts delta Latitude
        ///                 to a distance
        ///                 LonToDist (dLon) is a function that converts delta Longitude
        ///                 to a distance
        ///                 Smg (i) = speed made good in interval i
        ///                 Dmg (i) = direction made good in interval i
        ///                 Then:
        ///                 dLat = (aLat (i-1) ? aLat (i))
        ///                 dLon = (aLon (i-1) ? aLon (i))
        ///                 VelMgn (i) = LatToDist (dLat) / (Ta(i-1) ? Ta(i))
        ///                 VelMge (i) = LonToDist (dLon) / (Ta(i-1) ? Ta(i))
        ///                 Smg(i) = sqrt(VelMGn(i)^2 + VelMGe(i)^2)
        ///                 Dmg(i) = atan(VelMGe(i) / VelMGn(i))
        ///                 
        struct NavigationData :DataSection
        {
            uint8_t UTCDay;
            uint8_t UTCMonth;
            uint16_t UTCYear;
            /// 
            ///                     UTC time since midnight; LSB = 0.01 seconds
            ///                     
            uint32_t UTCTimeFirstFix;
            /// 
            ///                     PC Time ? UTC (signed); LSB = milliseconds
            ///                     
            uint32_t PCClockOffset;
            /// 
            ///                     This the first latitude position received after the previous ADCP ping.
            ///                     LSB=approx. 8E-8 deg (32-bit BAM)
            ///                     
            uint32_t FirstLatitude;
            /// 
            ///                     This is the first longitude position received after the previous ADCP ping.
            ///                     LSB=approx. 8E-8 deg (32-bit BAM)
            ///                     
            uint32_t FirstLongitude;
            /// 
            ///                     Time since midnight UTC; LSB=1E-4 seconds
            ///                     
            uint32_t UTCTimeLastFix;
            /// 
            ///                     This is the last latitude position received prior to the current ADCP ping.
            ///                     LSB=approx. 8E-8 deg (32-bit BAM)
            ///                     
            uint32_t LastLatitude;
            /// 
            ///                     This is the last longitude position received prior to the current ADCP ping.
            ///                     LSB=approx. 8E-8 deg (32-bit BAM)
            ///                     
            uint32_t LastLongitude;
            /// 
            ///                     Average Navigational Speed mm/sec (signed)
            ///                     
            int16_t AvgSpeed;
            /// 
            ///                     Average True Navigational Ship Track Direction
            ///                     LSB=approx. 0.0055 deg (16-bit BAM)
            ///                     
            uint16_t AvgTrackTrue;
            /// 
            ///                     Average Magnetic Navigational Ship Track Direction
            ///                     LSB=approx. 0.0055 deg (16-bit BAM)
            ///                     
            uint16_t AvgTrackMagnetic;
            /// 
            ///                     Speed calculated between navigation positions.
            ///                     LSB=one mm/sec (signed)
            ///                     
            int16_t SpeedMadeGood;
            /// 
            ///                     Direction calculated between navigation positions.
            ///                     LSB = approx. 0.0055 deg (16-bit BAM)
            ///                     
            int16_t DirectionMadeGood;
            uint16_t Reserved1;
            /// 
            ///                     Describes the validity of the data. Each bit has represents
            ///                     a separate flag and has its own meaning 1=true,
            ///                     0=false. The flag bits are defined as follows:
            ///                     bit 0 = Data updated
            ///                     bit 1 = PSN Valid
            ///                     bit 2 = Speed Valid
            ///                     bit 3 = Mag Track Valid
            ///                     bit 4 = True Track Valid
            ///                     bit 5 = Date/Time Valid
            ///                     bit 6 = SMG/DMG Valid
            ///                     bit 7 = Pitch/Roll Valid
            ///                     bit 8 = Heading Valid
            ///                     bit 9 = ADCP Time Valid
            ///                     bit 10 = Clock Offset Valid
            ///                     bit 11 = Reserved
            ///                     bit 12 = Reserved
            ///                     bit 13 = Reserved
            ///                     bit 14 = Reserved
            ///                     bit 15 = Reserved
            ///                     
            uint16_t Flags;
            uint16_t Reserved2;
            uint32_t EnsembleNumber;
            uint16_t EnsembleYear;
            uint8_t EnsembleDay;
            uint8_t EnsembleMonth;
            /// 
            ///                     Number of seconds since midnight; LSB = 0.01 seconds
            ///                     
            uint32_t EnsembleTime;
            /// 
            ///                     
            uint16_t Pitch;
            /// 
            ///                     
            uint16_t Roll;
            /// 
            ///                     
            uint16_t Heading;
            uint16_t NumberSpeedSamplesAveraged;
            uint16_t NumberTrueTrackSamplesAveraged;
            uint16_t NumberMagneticTrackSamplesAveraged;
            uint16_t NumberHeadingSamplesAveraged;
            uint16_t NumberPitchRollSamplesAveraged;

            NavigationData()
            {
                SectionID = 0x2000;
            }
            bool IsValid() const
            {
                return DataSection::IsValid() && SectionID == 0x2000;
            }
            uint16_t getArrayCount() const {return 0;}
        };

    }
}

#pragma pack()

#endif
