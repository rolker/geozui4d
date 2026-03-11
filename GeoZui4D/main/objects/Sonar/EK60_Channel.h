// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef EK60_CHANNEL_H
#define EK60_CHANNEL_H

#include "engine/ObjectCollection.h"
#include <map>
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/glo/Texture.h>
#include <gz4d/time.h>
#include <gz4d/Color.h>
#include "objects/Sonar/EK60.h"
#include <gz4d/sonar/Simrad/file/FileFormat.h>

class EK60_Channel: public Object
{
    struct Ping
    {
        float z1,z2;
        uint32_t number;
    };

    std::map<gz4d::Time,Ping> pings;
    std::vector<gz4d::Raster< gz4d::Pixel<gz4d::RGBA8> > > rasters;
    unsigned int lastRaster;

    EK60* ek60;

    float f; // frequency
    float G0; // Gain if data recorded with ER60 V1.3
    struct Gain_SaCorrection
    {
        float gain;
        float SaCorrection;
    };

    std::map<float,Gain_SaCorrection> gainTable; // index is pulse length
    float y; // Equivalent 2-way beam angle in steradians
    

    float beamWidthX;
    float beamWidthY;
    float beamDirectionX;
    float beamDirectionY;

    float startAngle_sin;
    float startAngle_cos;
    float endAngle_sin;
    float endAngle_cos;
    float midAngle_sin;
    float midAngle_cos;


    std::shared_ptr<gz4d::glo::Texture> texture;
    gz4d::Time displayTime;

    gz4d::time::Duration displaySpan;
    double fadeDelay;
    gz4d::Color baseColor;
    bool showFan;
    bool showRibbon;

    public:
        DERIVE_OBJECT(EK60_Channel);

        EK60_Channel(EK60 * ek60 = NULL);
        EK60_Channel(EK60 * ek60, std::string const &name);
        EK60_Channel(EK60 * ek60, gz4d::Simrad::ConfigurationBeam const &config);

        void DrawLowRes(float degradation, Window3D *win);
        int perform(int argc, const char **argv, Tcl_Interp *interp);
        void Idle(double time);
        void AddPing(gz4d::Simrad::SampleDatagram const &ping);
        void getHtmlInfo(std::stringstream &html, short level);
};

#endif
