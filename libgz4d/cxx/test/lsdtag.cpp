#include <gz4d/dtag.h>
#include <gz4d/dtag/Chunk.h>
#include <gz4d/dtag/Block.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>

void usage()
{
    std::cerr << "Usage: lsdtag [options] dtag1prefix [dtag2prefix...]\n";
    std::cerr << "   options:\n";
    std::cerr << "     -e DTG         {Extension, defaults to dtg. This can be useful on a case sensative system!)\n";
    std::cerr << "     -t -5          (Time offet (hours) from UTC. Defaults to zero. Ex: -t -5 for EST)\n";
    std::cerr << "     -b             (Scan and report block information\n";
    std::cerr << "     -c             (Scan and report chunk information\n";
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        usage();
        exit(1);
    }

    float timeOffset = 0.0;
    std::string extension="dtg";
    std::vector<std::string> args;
    bool scanBlocks = false;
    bool scanChunks = false;

    for(int i = 1; i < argc; i++)
    {
        if(std::string(argv[i]) == "-e")
            if (i < argc-1)
            {
                extension = argv[++i];
            }
            else
            {
                usage();
                exit(1);
            }
        else if (std::string(argv[i]) == "-t")
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
        else if (std::string(argv[i]) == "-b")
            scanBlocks = true;
        else if (std::string(argv[i]) == "-c")
            scanChunks = true;
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
        std::cout << dtag.GetInfo(scanBlocks,scanChunks);
/*        if (scanChunks)
        {
            dtag.Reset();
            double startTime = dtag.GetStartTime();
            int currentBlock = -1;
            std::map<unsigned short,int> types;
            while(dtag.NextChunk())
            {
                types[dtag.GetChunk()->GetType()]++;
                if (currentBlock != dtag.GetCurrentBlock()->GetCurrentIndex())
                {
                    currentBlock = dtag.GetCurrentBlock()->GetCurrentIndex();
                    std::cout << dtag.GetCurrentDtgId() << ":" << currentBlock << " " << dtag.GetCurrentBlock()->GetTime() << " " << dtag.GetCurrentBlock()->GetTime()-currentBlock << " chunk: " << dtag.GetChunk()->GetTime()/1000.0 << std::endl;
                }
            }
            for(std::map<unsigned short,int>::iterator i = types.begin(); i != types.end(); i++)
                std::cout << "Chunk type: " << i->first << " count: " << i->second << std::endl;
        }*/
        std::cout << std::endl;
    }

    return 0;
}
