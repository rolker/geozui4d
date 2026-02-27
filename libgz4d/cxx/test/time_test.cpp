#include <gz4d/time.h>
#include <iostream>
#include <gz4d/time/ios.h>

int main(int argc, char * argv[])
{
    gz4d::Time t;
    if(argc == 2)
    {
        double ts = atof(argv[1]);
        std::cout << "input: " << ts << std::endl;
        t = gz4d::Time(ts);
    }
    else if(argc > 3 && argc < 8)
    {
        int year;
        int month;
        int day;
        int hour = 0;
        int minute = 0;
        double second = 0.0;
        year = atoi(argv[1]);
        month = atoi(argv[2]);
        day = atoi(argv[3]);
        std::cout << "input: year: " << year << " month: " << month << " day: " << day;
        
        if(argc > 4)
        {
            hour = atoi(argv[4]);
            std::cout << " hour: " << hour;
            if(argc > 5)
            {
                minute = atoi(argv[5]);
                std::cout << " minute: " << minute;
                if(argc > 6)
                {
                    second = atof(argv[6]);
                    std::cout << " second: " << second;
                }
            }
        }
        std::cout << std::endl;

        std::cout << "days in 400 year blocks " << (year/400)*DAYS_PER_400Y <<  "\t" << gz4d::Time(((year/400)*DAYS_PER_400Y- DAYS_TO_1970)*SECONDS_PER_DAY) <<  "\n";
        std::cout << "days in 100 year blocks " << ((year%400)/100)*DAYS_PER_NON_LEAP_100Y <<  "\t" << gz4d::Time(((year/400)*DAYS_PER_400Y+((year%400)/100)*DAYS_PER_NON_LEAP_100Y- DAYS_TO_1970)*SECONDS_PER_DAY) <<  "\n";
        std::cout << "days in 4 year blocks " << ((year%100)/4)*DAYS_PER_4Y <<  "\t" << gz4d::Time(((year/400)*DAYS_PER_400Y+((year%400)/100)*DAYS_PER_NON_LEAP_100Y+((year%100)/4)*DAYS_PER_4Y- DAYS_TO_1970)*SECONDS_PER_DAY) <<  "\n";
        std::cout << "days in non leap years " << (year%4)*DAYS_PER_NON_LEAP_YEAR <<  "\t" << gz4d::Time(((year/400)*DAYS_PER_400Y+((year%400)/100)*DAYS_PER_NON_LEAP_100Y+((year%100)/4)*DAYS_PER_4Y+(year%4)*DAYS_PER_NON_LEAP_YEAR- DAYS_TO_1970)*SECONDS_PER_DAY) <<  "\n";

        std::cout << "days to 1970 " << DAYS_TO_1970 <<  "\n" << gz4d::Time(1970,1,1,0,0,0) <<  "\n";
        t = gz4d::Time(year,month,day,hour,minute,second);
    }
    else
    {
        std::cerr << "usage: time_test timestamp | year month day [hours [minutes [seconds]]]" << std::endl;
        return 1;
    }

    std::cout << t << std::endl;
    std::cout << t.secondsSinceEpoch().asSeconds() << std::endl;
    
    return 0;
}