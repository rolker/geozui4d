#ifndef CURRENTPROFILES_H
#define CURRENTPROFILES_H

#include "engine/Object.h"
#include <gz4d/TimeSequence.h>
#include <gz4d/nav/Fix.h>
#include <gz4d/nav/Track.h>
#include <gz4d/glo/DisplayList.h>

class CurrentProfiles : public Object
{
    typedef gz4d::binary::Wrapper<gz4d::binary::Sonar::CurrentProfile> ProfilePtr;
    typedef gz4d::TimeSequence<ProfilePtr> ProfileSeq;
    ProfileSeq profiles;
    gz4d::nav::Track navigation;
    bool positionSet;

    float scaleFactor;
    float r,g,b;

    struct ParticleParameters
    {
        float radius;
        gz4d::time::Duration lifeSpan;
        int depthSpacing;
        gz4d::time::Duration timeSpacing;
        gz4d::time::Duration timeResolution;
        gz4d::time::Duration maxGap;
        int tailSteps;
        gz4d::time::Duration tailSegmentLength;
        ParticleParameters();
        bool operator==(ParticleParameters const &other) const;
        bool operator!=(ParticleParameters const &other) const;
    };

    ParticleParameters requestedParticleParameters;
    ParticleParameters currentParticleParameters;

    struct Particle
    {
        gz4d::TimeSequence<gz4d::Point<float> > offsets;
    };

    std::vector<Particle> particles;
    gz4d::glo::DisplayList particleDisplayList;

    public:
        DERIVE_OBJECT(CurrentProfiles);

        CurrentProfiles();
        void DrawLowRes(float degradation, Window3D *win);
        void HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data);
        int Load();
        void Idle(gz4d::Time time);
};


#endif
