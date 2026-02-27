#include <gz4d/dtag.h>
#include <iostream>
#include <cstdlib>

void usage()
{
    std::cerr << "Usage: dtg2ogg [options] dtagPrefix\n";
    std::cerr << "   options:\n";
    std::cerr << "     -o output.ogg  (Output file, defaults to dtagPrefix.ogg)\n";
    std::cerr << "     -e DTG         {Extension, defaults to dtg. This can be useful on a case sensative system!)\n";
    std::cerr << "     -t -5          (Time offet (hours) from UTC. Defaults to zero. Ex: -t -5 for EST)\n";
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        usage();
        exit(1);
    }


    float timeOffset = 0.0;
    std::string output;
    std::string extension="dtg";

    int cursor = 1;
    while(cursor < argc-1)
    {
        if(std::string(argv[cursor]) == "-o")
        {
            if (cursor < argc-2)
            {
                output = argv[++cursor];
            }
            else
            {
                usage();
                exit(1);
            }
        }
        else if (std::string(argv[cursor]) == "-e")
        {
            if (cursor < argc-2)
            {
                extension = argv[++cursor];
            }
            else
            {
                usage();
                exit(1);
            }
        }
        else if (std::string(argv[cursor]) == "-t")
        {
            if (cursor < argc-2)
            {
                timeOffset = atof(argv[++cursor]);
            }
            else
            {
                usage();
                exit(1);
            }
        }
        else
        {
            usage();
            exit(1);
        }
        cursor++;
    }

    if(output.empty())
        output = std::string(argv[argc-1])+".ogg";

    gz4d::dtag::DTAG dtag(argv[argc-1]);
    dtag.SetTimeOffset(timeOffset);
/*    gz4d::dtag::MultiAudioStream a(&dtag);

    if (!a.SaveOggVorbis(output))
        return 1;*/
    return 0;
}
