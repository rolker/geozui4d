// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/dtag.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char * argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: dtg2wav dtag [output.wav]\n";
        exit(1);
    }

    gz4d::dtag::DTAG dtag(argv[argc-1]);

/*    gz4d::dtag::MultiAudioStream a(&dtag);

    if (argc == 3)
        a.SaveWAV(argv[2]);
    else
    {
        std::string outfile = std::string(argv[1])+".wav";
        a.SaveWAV(outfile);
    }*/
    return 0;
}
