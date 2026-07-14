// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include <gz4d/dtag.h>
#include <iostream>
#include <gz4d/dtag/MultiSensorStream.h>
#include <cstdlib>
#include <filesystem>
#include <gz4d/time/ios.h>

void usage()
{
    std::cerr << "Usage: dtg2bin [options] dtagDir\n";
    std::cerr << "   options:\n";
    std::cerr << "     -t -5          (Time offet (hours) from UTC. Defaults to zero. Ex: -t -5 for EST)\n";
    exit(1);
}

int main(int argc, char * argv[])
{

    if (argc < 2)
        usage();

    float timeOffset = 0.0;
    std::vector<std::string> args;

    for(int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "-t")
        {
            if (i < argc-1)
            {
                timeOffset = atof(argv[++i]);
            }
            else
            {
                usage();
                exit(1);
            }
        }
        else
            args.push_back(argv[i]);
    }

    if(args.size() == 0)
    {
        usage();
        exit(1);
    }

    for(std::vector<std::string>::iterator i = args.begin(); i != args.end(); i++)
    {
        gz4d::dtag::DTAG dtag(*i);
        dtag.SetTimeOffset(timeOffset);
        gz4d::Time startTime = dtag.GetStartTime();
        gz4d::time::Duration timeSpan = dtag.GetEndTime() - startTime;
        int lastProg = 0;
        for(int j = 0; j < 20; ++j)
            std::cerr << std::string("-");
        std::cerr << std::endl;
        gz4d::dtag::MultiSensorStream s;
        dtag.GetStream(s);
        std::filesystem::path outname = *i;
        outname /= "out.bin";
        FILE * out = fopen(outname.string().c_str(),"wb");
        if(!out)
        {
            std::cerr << "Problem opening file: " << outname << std::endl;
            exit(1);
        }

        std::stringstream head;

        head << "tag_id " << dtag.GetID() << std::endl;
        head << "start_time " << std::fixed << std::setprecision(3) << dtag.GetStartTime().secondsSinceEpoch() << std::endl;

        head << "Time ";
        for (int j = 0; j < s.GetSensorCount(); ++j)
        {
            head << s.GetLabel(j);
            if (j < s.GetSensorCount() -1)
                head << " ";
        }
        fwrite(head.str().c_str(),1,head.str().size()+1,out);

        while(s)
        {
            gz4d::dtag::MultiSensorStream::SampleSet::Ptr samples = s.Next();
            if(samples)
            {
                int prog = (samples->sampleTime*20)/timeSpan.asSeconds();
                if(prog > lastProg)
                {
                    std::cerr << "*";
                    lastProg = prog;
                }
                double t = samples->sampleTime;
                for(std::vector<std::vector<int16_t> >::iterator i = samples->samples.begin(); i != samples->samples.end(); ++i)
                {
                    fwrite(&t,sizeof(t),1,out);
                    fwrite(&(i->front()),sizeof(int16_t),samples->samples.size(),out);
                    t += samples->period;
                }
            }
        }
        std::cerr << std::endl;
        fclose(out);

    }
    return 0;
}
