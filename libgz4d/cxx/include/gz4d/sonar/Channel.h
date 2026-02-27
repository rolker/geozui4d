#ifndef GZ4D_SONAR_CHANNEL_H
#define GZ4D_SONAR_CHANNEL_H

#include <gz4d/sonar/BeamGeometry.h>
#include <gz4d/sonar/SampleContainer.h>
#include <boost/operators.hpp>
#include <gz4d/data/binary/Packet.h>


namespace gz4d
{
    namespace sonar
    {
        struct Pulse
        {
            float duration;
            float frequency;
            Pulse():duration(Nan<float>()),frequency(Nan<float>()){}
        };

        template <typename Derived, typename CT> class ChannelBase
        {
            public:
                typedef std::pair<typename CT::iterator, typename CT::iterator> sample_cluster;

                typedef CT container_type;
                typedef typename CT::iterator iterator;
                typedef typename CT::sv_iterator sv_iterator;
                typedef typename CT::range_iterator range_iterator;
                typedef typename CT::xyz_iterator xyz_iterator;
                typedef typename CT::ts_iterator ts_iterator;
                typedef typename CT::ss_iterator ss_iterator;
                typedef typename CT::time_iterator time_iterator;
                typedef typename CT::error_iterator error_iterator;

                CT &Samples(){return  Derived::Samples();}
                CT const &Samples() const{return Derived::Samples();}

                ChannelBase():id(0),sampleInterval(Nan<float>()){}

                void SetBottomDetection(sample_cluster const &s){bottomDetection = s;}
                sample_cluster const &GetBottomDetection() const {return bottomDetection;}

                void SetTargetDetections(std::vector<sample_cluster> const &s){targetDetections = s;}
                std::vector<sample_cluster> const &GetTargetDetections() const {return targetDetections;}

                uint32_t ID(){return id;}
                BeamGeometry const &GetGeometry() const {return geometry;}
                Pulse const &GetPulse() const {return pulse;}
                float GetSampleInterval() const {return sampleInterval;}
                float GetSampleOffset() const {return sampleOffset;}
                float GetTotalTransmitDuration() const {return totalTransmitDuration;}
                Time GetTimestamp() const {return timestamp;}

            protected:
                Time timestamp;
                uint32_t id;
                BeamGeometry geometry;
                Pulse pulse;
                float sampleInterval;
                float sampleOffset;
                float totalTransmitDuration;
                sample_cluster bottomDetection;
                std::vector<sample_cluster> targetDetections;
        };
        
        template <typename CDT> class Channel: public ChannelBase<Channel<CDT>, SampleContainer<CDT> >
        {
            SampleContainer<CDT> samples;
            public:
                Channel(){}
                Channel(std::shared_ptr<CDT> &cd)
                :samples(cd->begin(),cd->end(),cd)
                {
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::timestamp = cd->Timestamp();
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::id = cd->ID();
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::sampleInterval = cd->SampleInterval();
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::sampleOffset = cd->SampleOffset();

                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::geometry = cd->BeamGeometry();
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::pulse.duration = cd->PulseDuration();
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::pulse.frequency = cd->Frequency();
                    ChannelBase<Channel<CDT>, SampleContainer<CDT> >::totalTransmitDuration = cd->TotalTransmitDuration();
                }

                SampleContainer<CDT> const &Samples() const {return samples;}
                SampleContainer<CDT> &Samples() {return samples;}
        };

    }
}

#endif
