// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/TextToSpeech.h>
#include <iostream>
#include <gz4d/time.h>
#include <gz4d/time/ios.h>

IMPLEMENT_GZ4D_TEXTTOSPEECH

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cerr << "usage: gz_tts text1 [text2 ...]" << std::endl;
        return 1;
    }

    gz4d::TextToSpeech tts;

    for(int i = 1; i < argc; ++i)
        tts.say(argv[i]);

    for(int i = 1; i < argc; ++i)
    {
        for (int r = 80; r <= 120; r += 20)
        {
            std::cout << "rate: " << r << "%" << std::endl;
            tts.rate(r/100.0);
            tts.say(argv[i]);
        }
        for (int fsm = 85; fsm <= 115; fsm += 15)
        {
            std::cout << "f0 mean: " << fsm << "%" << std::endl;
            tts.frequency(fsm/100.0);
            tts.say(argv[i]);
        }
        for(int fss = 25; fss <=175; fss += 75)
        {
            std::cout << "f0 std: " << fss << "%" << std::endl;
            tts.frequency(1.0,fss/100.0);
            tts.say(argv[i]);
        }
    }
    return 0;
}
