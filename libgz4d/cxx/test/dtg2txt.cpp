// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/dtag.h>
#include <iostream>

int main(int argc, char * argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: dtg2txt dtagDir [packet_count (defaults to 1)]\n";
        exit(1);
    }

    gz4d::dtag::DTAG dtag(argv[1]);
    gz4d::dtag::MultiAudioStream a;
    dtag.GetStream(a);

    int pc = 1;
    if(argc == 3)
        pc = atoi(argv[2]);

    for (int i = 0; i < pc; ++i)
    {
        std::vector<gz4d::AudioPacket::Ptr> ap = a.Next();
        for(int j = 0; j < ap[0]->samples.size(); ++j)
        {
            for(int c = 0; c < ap.size(); ++c)
            {
                std::cout << std::fixed << std::setprecision(14) << ap[c]->samples[j];
                if(c < ap.size()-1)
                    std::cout << ", ";
                else
                    std::cout << std::endl;
            }
        }
    }
    return 0;
}
