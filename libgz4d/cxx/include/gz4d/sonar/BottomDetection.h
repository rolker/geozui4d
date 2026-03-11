// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_SONAR_BOTTOMDETECTION_H
#define GZ4D_SONAR_BOTTOMDETECTION_H

#include <gz4d/sonar/Sonar.h>
#include <gz4d/nav/Error.h>
#include <gz4d/Point.h>
#include <gz4d/data/Binary.h>
#include <numeric>

namespace gz4d
{
    namespace sonar
    {
        template <typename T> inline std::pair<T,T> DetectBottom(T start, T end, typename T::difference_type steps)
        {
            T max = start;
            float maxVal;
            float curVal = 0.0;
            std::pair<T,T> cur(start,start+steps);
            if (cur.second > end)
                cur.second = end;
            for (T i = cur.first; i != cur.second; ++i)
            {
                if(!IsNan(*i))
                    curVal += *i;
            }
            maxVal = curVal;
            while(cur.second != end && maxVal < 0.0)
            {
                if(cur.second + static_cast<typename T::difference_type>(1) != end)
                {
                    if(!IsNan(*(cur.first)))
                        curVal -= *(cur.first);
                    if(!IsNan(*(cur.second+static_cast<typename T::difference_type>(1))))
                        curVal += *(cur.second+static_cast<typename T::difference_type>(1));
                }
                ++cur.first;
                ++cur.second;
                if(curVal > maxVal)
                {
                    max = cur.first;
                    maxVal = curVal;
                }
            }
            return std::pair<T,T>(max,max+steps);
        }

        template <typename T> inline typename T::sample_cluster DetectBottom(T &channel)
        {
            typename T::ss_iterator i = channel.Samples().begin();
            typename T::ss_iterator end = channel.Samples().end();
/*            typename T::sv_iterator i = channel.Samples().begin();
            typename T::sv_iterator end = channel.Samples().end();*/
            float si = *(typename T::time_iterator(i+static_cast<typename T::ss_iterator::difference_type>(1)))- *(typename T::time_iterator(i)); // sample interval
            if(channel.GetTotalTransmitDuration() > 0.0)
                i += channel.GetTotalTransmitDuration()*1.5/si;
            else
            {
                ++i;
                while(*i > -50.0 && i != end)
                    ++i;
            }
            int steps = channel.GetPulse().duration/si;
            return DetectBottom(i,end,std::max(steps,1));
        }

        struct BottomDetection: public geo::Position<double>, public nav::Error<float>
        {
            Point<float> position;
            std::vector<float> backscatter;
            Time timestamp;
            uint32_t id;

            typedef std::shared_ptr<BottomDetection> Ptr;
            template <typename NavType> BottomDetection(std::vector<float> bs, Point<float> p, NavType nav, nav::Error<float> e)
                :backscatter(bs),position(p),nav::Error<float>(e)
            {
                if(nav)
                {
                    timestamp = nav->timestamp;

                    float hr = Radians(nav->heading.value());
                    float x = position[0]*::cos(hr) + position[1]*::sin(hr);
                    float y = position[1]*::cos(hr) - position[0]*::sin(hr);
                    position[0] = x;
                    position[1] = y;

                    latitude = nav->latitude;
                    longitude = nav->longitude;
                    position[2] -= nav->altitude;
                }
            }

            template <typename T> void SendVia(T &s)
            {
                binary::Wrapper<binary::Sonar::Channel::BottomDetection> bd;
                bd->timestamp = timestamp;
                bd->channelID = id;
                bd->longitude = longitude.value();
                bd->latitude = latitude.value();
                bd->x = position[0];
                bd->y = position[1];
                bd->z = position[2];
                bd->samples.size = backscatter.size();
                bd.Resize();
                std::copy(backscatter.begin(),backscatter.end(),bd->samples.values);
                bd->variance.x = variance[0];
                bd->variance.y = variance[1];
                bd->variance.z = variance[2];
                bd.SendVia(s);
            }
        };

        template <typename T> class BottomDetector: public Producer<BottomDetection::Ptr>, public Producer<std::shared_ptr<T> >
        {
            public:
                BottomDetector(){}
                typedef std::shared_ptr<BottomDetector> Ptr;
                void Consume(std::shared_ptr<T> &ping);

                template<typename PT> void AddOutQueue(typename Producer<PT>::output_queue_type &q)
                {
                    Producer<PT>::AddOutQueue(q);
                }

        };

        template <typename T> inline void BottomDetector<T>::Consume(std::shared_ptr<T> &ping)
        {
            for(typename T::channels_type::iterator c = ping->GetChannels().begin(); c != ping->GetChannels().end(); ++c)
            {
                if(!c->Samples().empty())
                {
                    c->SetBottomDetection(DetectBottom(*c));
                    if(ping->GetNavigation() && ping->GetNavigation()->IsComplete())
                    {

                        if(c->GetBottomDetection().first != c->Samples().end())
                        {

                            typename T::channels_type::value_type::ss_iterator i = c->GetBottomDetection().first + (c->GetBottomDetection().second-c->GetBottomDetection().first)/2;

                            std::vector<float> bs(typename T::channels_type::value_type::ss_iterator(c->GetBottomDetection().first), typename T::channels_type::value_type::ss_iterator(c->GetBottomDetection().second));
                            //bs.push_back(std::accumulate(typename T::channel_type::ss_iterator(c->GetBottomDetection().first), typename T::channel_type::ss_iterator(c->GetBottomDetection().second),0.0)/float(c->GetBottomDetection().second - c->GetBottomDetection().first));
                            BottomDetection::Ptr bd(new BottomDetection(bs,*(typename T::channels_type::value_type::xyz_iterator(i)),ping->GetNavigation(),*(typename T::channels_type::value_type::error_iterator(i))));
                            bd->id = c->ID();
                            Producer<BottomDetection::Ptr>::Send(bd);
                        }
                    }
                }
            }
            Producer<std::shared_ptr<T> >::Send(ping);
        }
    }
}

#endif
