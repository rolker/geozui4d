// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/sonar/Simrad/file/FileSource.h>
#include <iostream>

int main(int argc, char *argv[])
{
    gz4d::ThreadedQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >::Ptr nmeaQueue(new gz4d::ThreadedQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >);

    for (int i = 1; i < argc; ++i)
    {
        gz4d::Simrad::FileSource fs(argv[i]);
        std::cerr << fs.GetInfo() << std::endl;

        fs.AddOutQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >(nmeaQueue);

        while(fs.ProcessNextDatagram())
        {
            while(!nmeaQueue->empty())
            {
                std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> s = nmeaQueue->pop();
                //std::cerr << s.first << ", " << (*(s.second))();
                std::cout << (*(s.second))();
            }

        }
    }

    return 0;
}
