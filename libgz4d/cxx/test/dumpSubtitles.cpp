#include <gz4d/av.h>
#include <boost/foreach.hpp>
#include <gz4d/time/ios.h>

int main(int argc, char* argv[])
{
    gz4d::AVReader avr;
    avr.Open(argv[1]);
    avr.DumpFormat();

    std::vector<gz4d::AVReader::SubtitleStream::output_queue_type> sqs;
    for(std::vector<std::shared_ptr<gz4d::AVReader::SubtitleStream> >::const_iterator i = avr.GetSubtitleStreams().begin(); i != avr.GetSubtitleStreams().end(); ++i)
    {
        gz4d::AVReader::SubtitleStream::output_queue_type q = (*i)->NewOutQueue();
        sqs.push_back(q);
    }

    while(avr.ReadFrame())
    {
        BOOST_FOREACH(gz4d::AVReader::SubtitleStream::output_queue_type q, sqs)
        {
            if(!q->empty())
            {
                gz4d::AVReader::SubtitleFrame f = q->pop();
                std::cout << "Sub: " << f.presentationTime << "\t" << f.data << std::endl;
            }
        }
    }
    return 0;
}
