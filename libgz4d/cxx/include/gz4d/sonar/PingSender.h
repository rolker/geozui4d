#ifndef GZ4D_SONAR_PINGSENDER_H
#define GZ4D_SONAR_PINGSENDER_H

#include <gz4d/socket/UDPSocket.h>

namespace gz4d
{
    namespace sonar
    {
        template<typename T> class PingSender
        {
            UDPSocket socket;

            public:
                typedef std::shared_ptr<PingSender> Ptr;

                PingSender(IPSocket::Address const &a);
                void Consume(std::shared_ptr<T> &ping);
        };

        template <typename T> PingSender<T>::PingSender(IPSocket::Address const &a)
        {
            socket.Bind(IPSocket::Address("ANY",0));
            socket.Connect(a);
        }

        template <typename T> void PingSender<T>::Consume(std::shared_ptr<T> &ping)
        {
            if(ping->GetNavigation()->IsComplete())
            {
                ping->GetNavigation()->SendVia(socket);
                for(typename T::channels_type::iterator c = ping->GetChannels().begin(); c != ping->GetChannels().end(); ++c)
                {
                    if(!c->Samples().empty())
                    {
                        binary::Wrapper<binary::Sonar::Channel::Samples> samples;
                        samples->geometry.width.x = c->GetGeometry().width[0];
                        samples->geometry.width.y = c->GetGeometry().width[1];
                        samples->geometry.direction.x = c->GetGeometry().direction[0];
                        samples->geometry.direction.y = c->GetGeometry().direction[1];
                        samples->timestamp = ping->Timestamp();
                        samples->channelID = c->ID();
                        samples->offset = c->GetSampleOffset();
                        samples->sampleInterval = c->GetSampleInterval();
                        typename T::channel_type::iterator end = c->Samples().end();
                        if(c->GetBottomDetection().second)
                            end = c->GetBottomDetection().second;

                        samples->samples.size = end-c->Samples().begin();
                        samples.Resize();
                        std::copy(typename T::channel_type::sv_iterator(c->Samples().begin()),typename T::channel_type::sv_iterator(end),samples->samples.values);

                        samples.SendVia(socket);
                    }
                }
            }
        }
    }
}

#endif
