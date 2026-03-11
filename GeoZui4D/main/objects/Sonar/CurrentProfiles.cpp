// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"CurrentProfiles.h"
#include "engine/Scene3D.h"
#include "engine/InputMediator.h"
#include <boost/concept_check.hpp>
#include <gz4d/glo/Vertex.h>

REGISTER_OBJECT(CurrentProfiles, "gz4d");

CurrentProfiles::CurrentProfiles()
    :positionSet(false),scaleFactor(10.0),r(1.0),g(1.0),b(1.0)
{
    binaryFile = true;
    projectionRequired = true;

    DefineAttribute("scaleFactor","Scale factor","Scale factor used to determine length of glyph in meters from speed in m/s",scaleFactor);
    DefineAttribute("red","Red","Red component of base color",r);
    DefineAttribute("green","Green","Green component of base color",g);
    DefineAttribute("blue","Blue","Blue component of base color",b);

    DefineAttribute("particleRadius","Particle Radius","Radius in meters of particles emmitted. 0 to disable particles",requestedParticleParameters.radius);
    DefineAttribute("particleLifeSpan","Particle Life Span","Life span of particles in seconds",requestedParticleParameters.lifeSpan);
    DefineAttribute("particleDepthSpacing","Particle Depth Spacing","Spacing (in profile depth steps) of particles emmitted",requestedParticleParameters.depthSpacing);
    DefineAttribute("particleTimeSpacing","Particle Time Spacing","Interval in seconds between particle emission",requestedParticleParameters.timeSpacing);
    DefineAttribute("particleTimeResolution","Particle Time Resolution","Interval in seconds between particle updates",requestedParticleParameters.timeResolution);
    DefineAttribute("particleMaxGap","Particle Max Time Gap","Maximum gap with no data allowed",requestedParticleParameters.maxGap);
    DefineAttribute("tailSteps","Tail Steps","Number of tails segements",requestedParticleParameters.tailSteps);
    DefineAttribute("tailSegmentLength","Tail Segment Length","Length in seconds of a tail segements",requestedParticleParameters.tailSegmentLength);
}

CurrentProfiles::ParticleParameters::ParticleParameters()
:radius(1.0),lifeSpan("24h"),depthSpacing(1),timeSpacing("30m"),timeResolution("30m"),maxGap("2h"),tailSteps(5),tailSegmentLength("30m")
{
}

bool CurrentProfiles::ParticleParameters::operator==(const CurrentProfiles::ParticleParameters& other) const
{
    return other.radius == radius && other.lifeSpan == lifeSpan && other.depthSpacing == depthSpacing && other.timeSpacing == timeSpacing && other.timeResolution == timeResolution  && other.maxGap == maxGap && other.tailSteps == tailSteps && other.tailSegmentLength == tailSegmentLength;
}

bool CurrentProfiles::ParticleParameters::operator!=(const CurrentProfiles::ParticleParameters& other) const
{
    return !(*this == other);
}


void CurrentProfiles::HandlePacket(std::shared_ptr< gz4d::Socket::Packet >& data)
{
    ProfilePtr cp(data);
    if(cp->IsValid())
    {
        profiles[cp->timestamp]=cp;
        expandBBoxTime(cp->timestamp);
        return;
    }

    gz4d::binary::Wrapper<gz4d::binary::Navigation::Position::Geographic> nav(data);
    if(nav->IsValid())
    {
        navigation.Consume(nav);
        expandBBoxTime(nav->timestamp);
        if(!positionSet)
        {
            gz4d::nav::Fix::Ptr fix = navigation(nav->timestamp);
            if (!gz4d::IsNan(fix->latitude) && !gz4d::IsNan(fix->longitude))
            {
                SetLatLon(fix->latitude,fix->longitude);
                positionSet = true;
            }
        }
        return;
    }

    Object::HandlePacket(data);
}

void CurrentProfiles::Idle(gz4d::Time time)
{
    if(time != lastIdleTime)
    {
        if((time >= bbox.start && time <= bbox.end) || (lastIdleTime >= bbox.start && lastIdleTime <= bbox.end))
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }

    if(particles.empty() || requestedParticleParameters != currentParticleParameters)
    {
        particles.clear();
        currentParticleParameters = requestedParticleParameters;
        if(currentParticleParameters.radius > 0.0)
        {
            for(gz4d::Time startTime = profiles.GetTimeSpan().first; startTime < profiles.GetTimeSpan().second; startTime = startTime + currentParticleParameters.timeSpacing)
            {
                std::cerr << startTime << " " << 100*(startTime - profiles.GetTimeSpan().first)/(profiles.GetTimeSpan().second-profiles.GetTimeSpan().first) << "%" << std::endl;
                ProfileSeq::const_iterator startProfile = profiles.GetLatest(startTime);
                for(int i=0; i < startProfile->second->count; i += currentParticleParameters.depthSpacing)
                {
                    particles.push_back(Particle());
                    particles.back().offsets[startTime]=gz4d::Point<float>(0.0,0.0,startProfile->second->vectors[i].depth);
                    for(gz4d::Time it = startTime+currentParticleParameters.timeResolution; it < startTime + currentParticleParameters.lifeSpan; it = it + currentParticleParameters.timeResolution)
                    {
                        ProfileSeq::const_iterator before = profiles.GetLatest(it-currentParticleParameters.timeResolution/2.0);
                        ProfileSeq::const_iterator after = profiles.GetNext(it-currentParticleParameters.timeResolution/2.0);
                        if(before != profiles.end() && after != profiles.end())
                        {
                            if(before->first > it-currentParticleParameters.maxGap)
                            {
                                float br = (after->first - it-currentParticleParameters.timeResolution/2.0)/(after->first - before->first);
                                float ar = 1.0 - br;
                                gz4d::Vector<float,3> current(before->second->vectors[i].vector.x*br+after->second->vectors[i].vector.x*ar,
                                                        before->second->vectors[i].vector.y*br+after->second->vectors[i].vector.y*ar,
                                                        before->second->vectors[i].vector.z*br+after->second->vectors[i].vector.z*ar);
                                particles.back().offsets[it]=particles.back().offsets.rbegin()->second + current*currentParticleParameters.timeResolution;
                            }
                            else
                                break;
                        }
                    }
                }
            }
        }
    }

    Object::Idle(time);
}

int CurrentProfiles::Load()
{
    int ret = Open();
    if (!ret)
    {
        while (true)
        {
            char header[5];
            if(fread(header,1,4,in)<4)
                break;
            header[4] = 0;
            if(std::string(header) != "gz4d")
                break;
            uint8_t version[2];
            if(fread(version,1,2,in)<2)
                break;
            uint64_t size;
            if(fread(&size,sizeof(size),1,in)<1)
                break;
            fseek(in,-(6+sizeof(size)),SEEK_CUR);
            std::shared_ptr<std::vector<char> > data(new std::vector<char>(size));
            if(fread(&data->front(),1,size,in) < size)
                break;
            HandlePacket(data);
        }
    }
    return ret;
}

void CurrentProfiles::DrawLowRes(float degradation, Window3D* win)
{
    ProfileSeq::const_iterator p = profiles.GetLatest(lastIdleTime);
    glDisable(GL_LIGHTING);
    if(p != profiles.end())
    {
        glColor3f(r,g,b);
        glLineWidth(3.0);
        glBegin(GL_LINES);
        for(int i = 0; i < p->second->count; ++i)
        {
            glVertex3f(0.0,0.0,p->second->vectors[i].depth);
            glVertex3f(p->second->vectors[i].vector.x*scaleFactor,p->second->vectors[i].vector.y*scaleFactor,p->second->vectors[i].depth+p->second->vectors[i].vector.z*scaleFactor);
        }
        glEnd();
        glPointSize(3);
        glBegin(GL_POINTS);
        for(int i = 0; i < p->second->count; ++i)
        {
            glVertex3f(0.0,0.0,p->second->vectors[i].depth);
            //glVertex3f(p->second->vectors[i].vector.x,p->second->vectors[i].vector.y,p->second->vectors[i].depth+p->second->vectors[i].vector.z);
        }
        glEnd();

    }

    //glEnable(GL_LIGHTING);
    EnableTexture();

    glPointSize(3.0);
    glBegin(GL_POINTS);
    for(std::vector<Particle>::const_iterator p = particles.begin(); p != particles.end(); ++p)
    {
        if(p->offsets.InInterval(lastIdleTime))
        {
            float intensity = 1.0-(lastIdleTime - p->offsets.begin()->first)/currentParticleParameters.lifeSpan.asSeconds();
            glColor3f(intensity,intensity,intensity);
            gz4d::Point<float> offset = p->offsets.GetInterpolated(lastIdleTime);
            gz4d::glo::Vertex(offset);
        }
    }
    glEnd();

    glLineWidth(2.0);
    if(currentParticleParameters.tailSteps > 0)
        for(std::vector<Particle>::const_iterator p = particles.begin(); p != particles.end(); ++p)
        {
            if(p->offsets.InInterval(lastIdleTime))
            {
                glBegin(GL_LINE_STRIP);

                float intensity = 1.0-(lastIdleTime - p->offsets.begin()->first)/currentParticleParameters.lifeSpan.asSeconds();
                glColor3f(intensity,intensity,intensity);
                gz4d::Point<float> offset = p->offsets.GetInterpolated(lastIdleTime);
                gz4d::glo::Vertex(offset);
                for(int i = 0; i < currentParticleParameters.tailSteps; ++i)
                {
                    if(p->offsets.InInterval(lastIdleTime - currentParticleParameters.tailSegmentLength*(i+1)))
                    {
                        offset = p->offsets.GetInterpolated(lastIdleTime- (i+1)*currentParticleParameters.tailSegmentLength);
                        float fade = 1.0 - i/float(currentParticleParameters.tailSteps);
                        glColor3f(intensity*fade,intensity*fade,intensity*fade);
                        gz4d::glo::Vertex(offset);
                    }
                }
                glEnd();
            }
        }

    
    DisableTexture();
}
