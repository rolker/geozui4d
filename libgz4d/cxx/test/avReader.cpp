// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/av.h>
#include <boost/foreach.hpp>
#include <gz4d/time/ios.h>

int main(int argc, char* argv[])
{
    gz4d::AVReader avr;
    avr.Open(argv[1]);
    avr.DumpFormat();

    gz4d::AVReader::AudioStream::output_queue_type aq;
    if (avr.GetAudioStreams().size())
        aq = avr.GetAudioStreams()[0]->NewOutQueue();

    gz4d::AVReader::VideoStream::output_queue_type vq;
    if (avr.GetVideoStreams().size())
        vq = avr.GetVideoStreams()[0]->NewOutQueue();

    std::vector<gz4d::AVReader::SubtitleStream::output_queue_type> sqs;
    for(std::vector<std::shared_ptr<gz4d::AVReader::SubtitleStream> >::const_iterator i = avr.GetSubtitleStreams().begin(); i != avr.GetSubtitleStreams().end(); ++i)
    {
        gz4d::AVReader::SubtitleStream::output_queue_type q = (*i)->NewOutQueue();
        sqs.push_back(q);
    }

    for(int i = 0; i < 100; ++i)
    {
        if(!avr.ReadFrame())
            break;
        if(aq && !aq->empty())
        {
            gz4d::AVReader::AudioStream::output_type ap = aq->pop();
            std::cerr << "AudioPacket: " << ap->samples.size() << " samples" << std::endl;
        }
        if(vq && !vq->empty())
        {
            gz4d::AVReader::VideoFrame img = vq->pop();
            std::cerr << "Frame: " << img.frame.GetRaster().GetColumns() << " x " << img.frame.GetRaster().GetRows() << std::endl;
        }
        BOOST_FOREACH(gz4d::AVReader::SubtitleStream::output_queue_type q, sqs)
        {
            if(!q->empty())
            {
                gz4d::AVReader::SubtitleFrame f = q->pop();
                std::cerr << "Sub: " << f.presentationTime << "\t" << f.data << std::endl;
            }
        }
    }
    return 0;
}
