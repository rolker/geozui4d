// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sound/Jack.h>
#include <string>
#include <gz4d/time.h>
#include <gz4d/time/ios.h>
#include <gz4d/sound/jack/Port.h>

int main(int argc, char * argv[])
{
    {
        gz4d::jack::Client c("midi_cat");
        gz4d::jack::MidiInputPort::Ptr mp = c.NewMidiInputPort("midi_in");
        c.Activate();
        gz4d::jack::MidiInputPort::output_queue_type q = mp->NewOutQueue();
        gz4d::Time timeout = gz4d::time::Now()+gz4d::time::Duration(90.0);
        while(gz4d::time::Now() < timeout)
        {
            if(q->wait_not_empty(0.5))
            {
                gz4d::midi::Event e = q->pop();
                std::cerr << e.timestamp() << "\tcmd: " << int(e.command()) << " channel: " << int(e.channel()) << " data: (";
                for(std::vector<uint8_t>::const_iterator d = e.data().begin(); d != e.data().end(); ++d)
                    std::cerr << int(*d) << ",";
                std::cerr << ")" << std::endl;
            }
        }
        std::cerr << "Done!" << std::endl;
    }
    gz4d::Sleep(gz4d::seconds(3));
    std::cerr << "Exiting..." << std::endl;
}
