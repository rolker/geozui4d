/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
// NmeaSource.h
/*---------------------------------------------------------------------------*/


#ifndef NMEASOURCE_H
#define NMEASOURCE_H

#include<stdio.h>
#include "engine/RTSource.h"
#include "utilities/Addressable.h"
#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;
#include <gz4d/socket/Socket.h>

class Window3D;

/*---------------------------------------------------------------------------*/
/** 
 *  This is a base class that represents an RTSource that generates NMEA
 *  strings.
 *  
 *  \author Matt Plumlee (mdp) 
 *  \date 9/21/2004
 *  \note  <a href="../nmea_codes.htm"> nmea_codes.htm</a>
 *  shows the nmea codes available. (type 'n' to search for it following the given link)
 */
/*---------------------------------------------------------------------------*/
class NmeaSource: public RTSource, public Addressable
{
public:
    void init();
    void openPort(std::string a, int s, bool udp_flag);
    NmeaSource(std::string a, int s, bool udp_flag = 0, bfs::path log_filename = "");
    NmeaSource(std::string a, int s, bool udp_flag, NmeaSource *logfileSource);
    NmeaSource(std::string const &fn);
    ~NmeaSource();
    bool isConnected(){return connected;};
    virtual void Idle(int synchronize = 0);
    double getFileProgress();

    enum NMEA_ValueType {Skip, Char, Integer, Float, Double, TempString, CopyString};
    void watchFor(char designator, const char *type, int watchName, int slot = -1, NMEA_ValueType ltype = Skip, void *var = NULL); //single var in any column
    void watchFor(char designator, const char *type, int watchName, int nSlots, NMEA_ValueType *types, void **vars);

    enum FilterType {FilterOff, FilterOutPrefix, FilterOutAllExceptPrefix};
    void setFilterPrefix(std::string prefix = "");
    void setFilterType(FilterType f);

    enum {NOT_PROCESSED = -1, DONE_PROCESSING, GZTIME, GZTOFFSET, GENERIC_DATA};
    int lookup(char *s);

    void changeLogfile(bfs::path new_filename = "");
    void shareLogfileWith(NmeaSource *otherSource = NULL) {logfileController = otherSource ? otherSource : this;}

    void changeInputFile(std::string const &new_filename = "");

protected:
    struct WatchRecord
    {
        char designator;
        const char *type;
        int watchName;
        int nSlots;
        NMEA_ValueType *types;
        void **vars;
    } *watchRecords;
    int nWatchRecords, nWatchRecordsAlloced;

    WatchRecord *addWatchRecord(char designator, const char *type, int watchName, int nSlots);

    virtual int process(char * s);
    void printString(char * s);
    int isType(const char * s, const char * t);
    virtual void resetStates(); //reset things that should be reset at each new file open.

    gz4d::Socket socket;
    bool udp;

    FILE *infile, *log;
    bool connected;
    long flen;
    int shouldLogGzTime;
    int fieldsFound;
    char *bufferPosition;

    NmeaSource *logfileController;

    double lastTimeRead;
    double offsetTime;

    FilterType filterType;
    char filterPrefix[2];
};

#endif
