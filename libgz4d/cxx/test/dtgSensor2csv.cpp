#include <gz4d/dtag.h>
#include <iostream>
#include <gz4d/dtag/MultiSensorStream.h>
#include <cstdlib>
#include <gz4d/time/ios.h>

void usage()
{
    std::cerr << "Usage: dtgSensor2csv [options] dtagDir\n";
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
        for(int i = 0; i < 20; ++i)
            std::cerr << std::string("-");
        std::cerr << std::endl;
        gz4d::dtag::MultiSensorStream s;
        dtag.GetStream(s);
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
                gz4d::Time t = dtag.GetStartTime()+gz4d::time::Duration(samples->sampleTime);
                for(std::vector<std::vector<int16_t> >::iterator i = samples->samples.begin(); i != samples->samples.end(); ++i)
                {
                    std::cout << t << ", ";
                    for(std::vector<int16_t>::iterator j = i->begin(); j != i->end(); ++j)
                    {
                        std::cout << *j;
                        if (j+1 != i->end())
                            std::cout << ", ";
                    }
                    t += gz4d::time::Duration(samples->period);
                    std::cout << std::endl;
                }
            }
        }
        std::cerr << std::endl;

    }
    return 0;
}
