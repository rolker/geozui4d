#ifndef DELTAT_H
#define DELTAT_H

#include "engine/ObjectCollection.h"
#include <vector>
#include <gz4d/gz4d.h>
//#include <gz4d/fft/DiscreetFourierTransform.h>
// Following was adpated from deltat.bfd

// File:         deltat.bfd
// Purpose:      Describe the Imagenex Delta-T Multibeam format (raw data)
// Date:         2006-10-26
//
// Note that the Imagenex file appears to be big-endian.

#pragma pack(push, 1)

struct DeltaT_Header
{
        uint8_t      hdr[3];         // ID magic sequence, should be '837' in that order LSB.
        uint8_t      id;                     // ID for following data; 10 == IUX, 11 == IVX
        uint16_t     length;         // Total length of the data packet in bytes (8192B or 16384B)
        uint16_t     n_bytes;        // Number of bytes from the sonar (8013B or 16013B)
        uint8_t      date[12];       // ASCII data, should be "DD-MMM-YYYY", zero terminated
        uint8_t      time[9];        // ASCII data, should be "HH:MM:SS", zero terminated
        uint8_t      csec[4];        // ASCII data, should be ".hh", zero terminated
        uint32_t     vframe_len;     // Length of video frame (top bit => frame exists)
        uint8_t      flags;          // Bitwise, information on transducer and displays:
                                        // b[7]: reserved = 1
                                        // b[6]: xdcr position (0 = Down, 1 = Up)
                                        // b[5..3]: reserved = 0
                                        // b[2..0]: display mode: 0 == sector, 1 == linear, 2 == perspective, 3 == profile, 4 == beamtest
        uint8_t      start_gain;     // Gain 0..20dB in 1dB increments
        uint8_t      rsrvd[5];       // Reserved (should be (0, 0, 0, 10, 0))
        uint8_t      pulse_len;      // Pulse length (in microseconds)/10, so 1..250 == 10 ..2500 microseconds.
        uint8_t      rsrvd2;         // Reserved, should always be 0.
        uint16_t     sspeed;         // Sound speed (m/s)*10; top bit is a flag: 0 => 1500.0 m/s, 1 =>
                                        //       you should strip the top bit, divide by ten and call it m/s.
        uint8_t      latitude[14];   // GPS ship's position latitude (ASCII)
        uint8_t      longitude[14];  // GPS ship's position longitude (ASCII)
        uint8_t      vspeed;         // GPS ship's speed (kts)/10
        uint16_t     heading;        // GPS ship's heading (degrees)*10
        uint8_t      rsrvd3;         // Reserved, should be 0.
        uint16_t     frequency;      // Operating frequency, kHz.
        uint16_t     pitch;          // Pitch angle in degrees/10; top bit is a flag: 0 => not available, 1 =>
                                        //       you should strip the top bit, divide by ten and call it degrees.
        uint16_t     roll;           // Roll angle in degrees/10; top bit is a flag: 0 => not available, 1 =>
                                        //       you should strip the top bit, divide by ten and call it degrees.
        uint16_t     yaw;            // Yaw angle in degrees/10; top bit is a flag: 0 => not available, 1 =>
                                        //       you should strip the top bit, divide by ten and call it degrees.
        uint16_t     reprate;        // Time (ms) between pings.
        uint8_t      disp_gain;      // Display gain (0--100%)
        uint8_t      rsrvd4[9];      // Reserved, always zero.
};

struct DeltaT_DataHeader {
        uint8_t      hdr[3];         // Will be either 'IUX' (smaller data) or 'IVX' (larger data)
        uint8_t      id;             // Head identifier (always 0x10)
        uint8_t      ser_status;     // Serial status: bitwise b[7]: char over-run, b[6]: switches accepted; b[1]: internal test.
        uint8_t      pkt_num;        // Packet number (0 .. 7 (IUX) or 0 .. 15 (IVX))
        uint8_t      version;        // Firmware version:
                                        //       0 = 12 header bytes, 8000 data bytes, 1 terminator
                                        //       1 = 32 header bytes, 1000 data bytes, 1 terminator
                                        //       2 = 32 header bytes, 16000 byte IVX mode
        uint8_t      range;          // Range in meters, 5 -- 100m.
        uint8_t      rsrvd[2];       // Reserved (probably zero)
        uint16_t     data_len;       // Data length following.
};

struct DeltaT_IUX_Data {
        DeltaT_DataHeader        hdr;
        uint8_t                      data[500][16];
        uint8_t                      term_byte;              // Always 0xFC
        uint8_t                      fill[79];
};

struct DeltaT_IVX_Data {
        DeltaT_DataHeader        hdr;
        uint8_t                      data[500][16][2];
        uint8_t                      term_byte;              // Always 0xFC
        uint8_t                      fill[271];
};

#pragma pack(pop)

#define DeltaT_iux 0x0A
#define DeltaT_ivx 0x0B

char const *month_name[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

struct BeamSample
{
    //float phaseShift[16]; ///< Phase correction of each element calculated from distance difference, sound speed and frequency. Units are radians
    float amplitude; ///< Amplitude resulting from beamforming, scaled to 0.0 to 1.0 for use with glColor.
    Point2f position[4]; ///< Corners of beam sample relative to sonar head. Conterclockwise, used for rendering GL_QUAD.
};

struct Beam
{
    BeamSample sample[500]; ///< 500 samples in DeltaT case.
};

struct BeamformedPing
{
    std::vector<Beam> beam;
    float soundSpeed;
    float elementSpacing;
    float frequency;
    int beamCount;
    float swathAngle;
    float range;
    float sampleRange;
    float beamWidth;
    float startAngle;
    bool flipElements;
    std::vector<std::vector<double> > element;
    std::vector<std::vector<std::complex<double> > > synt_element;
    std::vector<std::vector<std::complex<double> > > element_frequency;
};

class DeltaT : public ObjectCollection
{
public:
    DeltaT();
    ~DeltaT();

    DERIVE_OBJECT(DeltaT);

    void DrawLowRes(float degradation, Window3D *win);
    int DrawHighRes(int eye, Window3D *win);
    int Load();
    uint16_t swap(uint16_t in){uint16_t out;((uint8_t*)&out)[0] = ((uint8_t*)&in)[1];((uint8_t*)&out)[1] = ((uint8_t*)&in)[0]; return out;}
    virtual void Idle(double time);
    int perform(int argc, const char **argv, Tcl_Interp *interp);


private:
    bool buildTimeIndex();
    void LoadPing(unsigned long pingNo);
    void beamform();
    void adjustBeams();

    DeltaT_Header header;
    DeltaT_IUX_Data * iux;
    DeltaT_IVX_Data * ivx;
    uint8_t data_id; ///< flag indicating iux or ivx data
    long recSize; ///< length of one ping record on file, to jump around in the file.
    std::vector<double> times; ///< List of times for each ping. The index of a certain time is 
    unsigned long timeIndex;
    bool needNewList;
    GLuint displayList;

    int beamCount;
    float swathAngle;
    float elementSpacing;
    bool showRaw;
    bool showBeamformedRectangle;
    bool showRawPhase;
    bool showRawBits;
    bool flipElements;
    float xOffset;
    float frequencyRatio;
    BeamformedPing ping;
    //gz4d::DiscreetFourierTransform dft;
    //gz4d::DiscreetFourierTransform bf_dft;
};

#endif
