#include <gz4d/data/NMEA.h>
#include <iostream>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        gz4d::nmea::Sentence nmea(argv[i]);
        std::cerr << nmea();
        for(int j = 0; j < nmea.size(); ++j)
            std::cerr << nmea[j] << std::endl;
    }
    return 0;
}
