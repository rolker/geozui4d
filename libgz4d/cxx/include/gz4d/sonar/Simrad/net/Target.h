#ifndef GZ4D_SIMRAD_TARGET_H
#define GZ4D_SIMRAD_TARGET_H

#include <gz4d/gz4d.h>
#include <gz4d/sonar/Simrad/net/Subscription.h>
#include <gz4d/geo/Ellipsoid.h>
#include <gz4d/time.h>

namespace gz4d
{
    namespace Simrad
    {
        struct Target
        {
            Point<float> position;
            float value;
            bool operator==(Target const &t){return position == t.position && value == value;}
			Target const &operator=(Target const &o){position=o.position; value=o.value; return *this;}
        };

        struct TargetSet
        {
            gz4d::Time time;
            std::vector<Target> targets;
        };


        class TargetSubscription: public Subscription, public Producer<TargetSet>
        {
            protected:
                std::list<TargetSet> targets;
                boost::mutex targetsMutex;

                //static geo::Ellipsoid wgs84;
            public:
                typedef std::shared_ptr<TargetSubscription> Ptr;

                TargetSubscription(std::string t, std::string channel, ParameterGroup::Map const &pingParams)
                :Subscription(t,channel,pingParams)
                {
                }
                
        };
    }
}

#endif
