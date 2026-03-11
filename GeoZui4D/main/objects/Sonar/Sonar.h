// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef SONAR_H
#define SONAR_H

#include "engine/Object.h"
#include <gz4d/TimeSequence.h>
#include <gz4d/nav/Fix.h>
#include <gz4d/nav/Track.h>
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>
#include <gz4d/glo/Texture.h>

#include <map>
#include <gz4d/sonar/Channel.h>

class Sonar : public Object
{
    typedef gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::Samples> SamplesPtr;
    struct Ping
    {
        SamplesPtr samples;
        int rasterNumber;
        int rasterIndex;
        float textureCoord;
    };

    typedef gz4d::TimeSequence<Ping> PingSeq;

    std::map<int,PingSeq> channels;

    struct ChannelDetails
    {
        gz4d::Time renderTime;
        bool visible;
        double renderHistory;
        std::vector<gz4d::Raster< gz4d::Pixel<gz4d::RGBA8> > > rasters;
        std::vector<std::shared_ptr<gz4d::glo::Texture> > textures;
        std::vector<bool> rasterUpdated;

        ChannelDetails():visible(true),renderHistory(3600.0){}
    };

    bool renderFan;
    bool renderCurtains;
    bool positionSet;
    bool offsetsNeedUpdate;

    float minDB;
    float maxDB;
    float alphaLimit;
    gz4d::time::Duration maxPingGap;

    std::map<int,ChannelDetails> channelDetails;
    gz4d::nav::Track navigation;

    struct ChannelOffset
    {
        gz4d::Point<double> start;
        gz4d::Point<double> diff;
        double step;
    };

    struct Offset
    {
        gz4d::Point<double> position;
        float heading;
        std::map<int,ChannelOffset> channel;
    };

    gz4d::TimeSequence<Offset> offsets;

    void UpdateOffsets();

    public:
        DERIVE_OBJECT(Sonar);

        Sonar();
        void DrawLowRes(float degradation, Window3D *win);

        void HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data);
        int Load();
        void Idle(double time);
        int perform(int argc, const char **argv, Tcl_Interp *interp);
        void getHtmlInfo(std::stringstream &html, short level);


};

#endif
