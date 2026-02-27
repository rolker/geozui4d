#ifndef GZ4D_SONAR_TARGETDETECTION_H
#define GZ4D_SONAR_TARGETDETECTION_H

#include <gz4d/sonar/Sonar.h>
#include <gz4d/nav/Error.h>
#include <gz4d/Point.h>

namespace gz4d
{
    namespace sonar
    {
        template <typename T> inline std::pair<T,T> DetectTarget(T start, T end, float threshold = -50.0)
        {
            std::pair<T,T> ret(start,end);
            while(ret.first != end && *ret.first < threshold)
            {
                ++ret.first;
            }
            if(ret.first != end)
            {
                ret.second = ret.first+static_cast<typename T::difference_type>(1);
                while(ret.second != end && *ret.second >= threshold)
                    ++ret.second;
            }
            return ret;
        }

        template <typename T> inline std::vector<typename T::sample_cluster> DetectTargets(T &channel, float threshold = -50.0)
        {
            typename T::ts_iterator i = channel.Samples().begin();
            typename T::ts_iterator end = channel.Samples().end();
            if(channel.GetBottomDetection().second)
                end = channel.GetBottomDetection().second;
            i += channel.GetTotalTransmitDuration()*1.5/(*(typename T::time_iterator(i+static_cast<typename T::time_iterator::difference_type>(1)))- *(typename T::time_iterator(i)));
            std::vector<typename T::sample_cluster> ret;

            while (i != end)
            {
                ret.push_back(DetectTarget(i,end,threshold));
                i = ret.back().second;
                if(i!=end)
                    ++i;
            }
            return ret;
        }

        struct TargetDetection: public geo::Position<double>, public nav::Error<float>
        {
            Point<float> position;
            float backscatter;
            Time timestamp;
            uint32_t id;

            typedef std::shared_ptr<TargetDetection> Ptr;
            template <typename NavType> TargetDetection(float bs, Point<float> p, NavType nav, nav::Error<float> e)
                :backscatter(bs),position(p),nav::Error<float>(e)
            {
                if(nav)
                {
                    timestamp = nav->timestamp;

                    float hr = Radians(nav->heading.value());
                    float x = position[0]*cos(hr) + position[1]*sin(hr);
                    float y = position[1]*cos(hr) - position[0]*sin(hr);
                    position[0] = x;
                    position[1] = y;

                    latitude = nav->latitude;
                    longitude = nav->longitude;
                    position[2] -= nav->altitude;
                }
            }

            template <typename T> void SendVia(T &s)
            {
                binary::Sonar::Channel::TargetDetection td;
                td.timestamp = timestamp;
                td.channelID = id;
                td.longitude = longitude.value();
                td.latitude = latitude.value();
                td.x = position[0];
                td.y = position[1];
                td.z = position[2];
                td.samples.size = 0;
                td.variance.x = variance[0];
                td.variance.y = variance[1];
                td.variance.z = variance[2];
                s.Send(static_cast<binary::Packet const *>(&td));
            }
        };

        template <typename T> class TargetDetector: public Producer<TargetDetection::Ptr>, public Producer<std::shared_ptr<T> >
        {
            float threshold;
            public:
                TargetDetector(float threshold = -50.0):threshold(threshold){}
                typedef std::shared_ptr<TargetDetector> Ptr;
                void Consume(std::shared_ptr<T> &ping);

                template<typename PT> void AddOutQueue(typename Producer<PT>::output_queue_type &q)
                {
                    Producer<PT>::AddOutQueue(q);
                }

        };

        template <typename T> inline void TargetDetector<T>::Consume(std::shared_ptr<T> &ping)
        {
            if(ping->GetNavigation()->IsComplete())
            {
                for(typename T::channels_type::iterator c = ping->GetChannels().begin(); c != ping->GetChannels().end(); ++c)
                {
                    if(!c->Samples().empty())
                    {
                        c->SetTargetDetections(DetectTargets(*c,threshold));
                        for(typename std::vector<typename T::channel_type::sample_cluster>::const_iterator t = c->GetTargetDetections().begin(); t != c->GetTargetDetections().end(); ++t)
                        {
                            if(t->first != c->Samples().end())
                            {
                                typename T::channel_type::iterator i = t->first;
                                TargetDetection::Ptr td(new TargetDetection(*(typename T::channel_type::ts_iterator(i)),*(typename T::channel_type::xyz_iterator(i)),ping->GetNavigation(),*(typename T::channel_type::error_iterator(i))));
                                td->id = c->ID();
                                Producer<TargetDetection::Ptr>::Send(td);
                            }
                        }
                    }
                }
            }
            Producer<std::shared_ptr<T> >::Send(ping);
        }


    }

}

#endif
