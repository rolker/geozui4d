#ifndef GZ4D_TIME_TIMECONSTANTS_H
#define GZ4D_TIME_TIMECONSTANTS_H

#define SECONDS_PER_DAY   86400.0
#define SECONDS_PER_WEEK 604800.0

// days per year 365
#define DAYS_PER_NON_LEAP_YEAR 365
// days per leap year 366
// days per 4 years 4*365+1 = 1461
#define DAYS_PER_4Y 1461
// years divisable by 100 are not leap except those divisable by 400
// days per 400 years = 1461*100-3 = 146097 
#define DAYS_PER_400Y 146097

// days from y0 to y1970 (epoch)
// 4*400Y+370Y = 4*400Y+300Y+70Y = 4*400Y+300Y+17*4Y+2Y
// 4*400Y = 584388d
// 100Y = 25*4Y = 25*1461 -1d = 36524d
#define DAYS_PER_NON_LEAP_100Y 36524
// 300Y = 3*100Y = 3*36524d = 109572d
// 17*4Y = 17*1461d = 24837d
// 2Y = 2*365 = 730d
// 584388d + 109572d + 24837d + 730d = 719527
//#define DAYS_TO_1970 719517
#define DAYS_TO_1970 719527

#define EPOCH_AS_NT_TIMESTAMP 0x019db1ded53e8000ULL


#endif
