// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "NmeaSource.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <filesystem>
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "engine/TimeControl.h"
#include <cstring>

using namespace std;

extern MainWindow *mainWindow;

static TimeControl *gztime;

#define WATCH_RECORD_QUANTUM 5
#define IMPOSSIBLY_EARLY -1.0e50 //a bogus, "impossibly" negative time


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void NmeaSource::init()
{
	connected = false;
	infile = NULL;
	log = NULL;
	shouldLogGzTime = GL_TRUE;
	logfileController = this;

	gztime = mainWindow->getScene3D()->timeControl;
	resetStates();

	watchRecords = NULL;
	nWatchRecords = nWatchRecordsAlloced = 0;
	watchFor('@', "TIM", GZTIME, 0, Double, &lastTimeRead);
	watchFor('@', "TOF", GZTOFFSET, 0, Double, &offsetTime);

	filterType = FilterOff;
	filterPrefix[0] = filterPrefix[1] = '\0';
        udp = false;
}

/*---------------------------------------------------------------------------*/
void NmeaSource::setFilterPrefix(string prefix)
{
	if (prefix.empty())
	{
		filterPrefix[0] = filterPrefix[1] = '\0';
		filterType = FilterOff;
	}
	else
	{
		filterPrefix[0] = prefix[0];
		filterPrefix[1] = prefix[1];
		if (filterType == FilterOff)
			filterType = FilterOutAllExceptPrefix;
	}
}

/*---------------------------------------------------------------------------*/
void NmeaSource::setFilterType(FilterType f)
{
}

/*---------------------------------------------------------------------------*/
void NmeaSource::resetStates()
{
	lastTimeRead = IMPOSSIBLY_EARLY;
	offsetTime = 0.0;
}

/*---------------------------------------------------------------------------*/
void NmeaSource::openPort(string a, int s, bool udp_flag)
{
    if(udp_flag && a.empty())
        a = "ANY";
    if(!a.empty())
    {
        gz4d::IPSocket::Address addr(a,s);
        if(udp_flag)
        {
            socket = gz4d::UDPSocket();
            socket.SetNonBlocking();
            socket.Bind(addr);
            udp = true;
            connected = true;
        }
        else
        {
            socket = gz4d::TCPSocket();
            socket.SetNonBlocking();
            socket.Connect(addr);
            udp = false;
            connected = true;
        }
    }
}

/*---------------------------------------------------------------------------*/
/**	This constructor is used when you want to use a live data source from a
 *	socket.  If a log_filename is specified, all the incoming data is logged
 *	into a file of the specified name.
 */
NmeaSource::NmeaSource(string a, int s, bool udp_flag, fs::path log_filename)
{
	init();
	openPort(a, s, udp_flag);

	if(!log_filename.empty())
		changeLogfile(log_filename);
}

/*---------------------------------------------------------------------------*/
/**	This constructor is used when you want to use a live data source from a
 *	socket.  If a log_filename is specified, all the incoming data is logged
 *	into a file of the specified name.
 */
NmeaSource::NmeaSource(string a, int s, bool udp_flag, NmeaSource *logfileSource)
{
	init();
	openPort(a, s, udp_flag);

	if(logfileSource)
		shareLogfileWith(logfileSource);
}

/*---------------------------------------------------------------------------*/
/**	This constructor is used when you want to use a previously recorded file,
 *	such as the logfile created by the other constructor.
 */
NmeaSource::NmeaSource(std::string const &fn)
{
	init();
	changeInputFile(fn);
}

/*---------------------------------------------------------------------------*/
NmeaSource::~NmeaSource()
{
	if (log)
		fclose(log);
	if (watchRecords)
		delete [] watchRecords;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/**	This function gets another NMEA string from the source.  If we're reading
 *	from a log file and synchronize is 1, we keep reading new data until
 *	we've caught up with the current notion of GZTime.
 */
void NmeaSource::Idle(int synchronize)
{
    char buffer[1024], *startBuffer;

    std::string data(1024,0);

    try
    {
        if(udp)
        {
            gz4d::IPSocket::Address a;
            socket.RecvFrom(data,a);
        }
        else
            socket.Recv(data);

        strcpy(buffer,data.c_str());
        startBuffer = bufferPosition = buffer;
        while (bufferPosition)
        {
            process(bufferPosition);
            NmeaSource::printString(startBuffer);
            while (*bufferPosition != '\0')
            {
                if (*bufferPosition == '$')
                        break; //get out to processing again.
                bufferPosition++;
            }
            if (*bufferPosition == '\0')
                    bufferPosition = NULL;
            startBuffer = bufferPosition;
        }
    }
    catch(gz4d::Socket::WouldBlockException){}
    //catch(gz4d::Socket::Exception){}

    if(infile)
    {
        if (lastTimeRead == IMPOSSIBLY_EARLY)
                synchronize = GL_FALSE;

        while (!synchronize || (lastTimeRead <= gztime->getGZTime())) //synchronize with gzTime
        {
            if(fgets(buffer, 1024, infile))
            {
                    startBuffer = bufferPosition = buffer;
                    //sometimes we get multiple strings on one line.  This will separate them out.
                    while (bufferPosition)
                    {
                            process(bufferPosition);
                            NmeaSource::printString(startBuffer);
                            while (*bufferPosition != '\0')
                            {
                                    if (*bufferPosition == '$')
                                            break; //get out to processing again.
                                    bufferPosition++;
                            }
                            if (*bufferPosition == '\0')
                                    bufferPosition = NULL;
                            startBuffer = bufferPosition;
                    }
            }

            if(feof(infile))
            {
                    fclose(infile);
                    infile = NULL;
                    connected = false;
                    return;
            }

            if (!synchronize) //if we're not synchronizing, just do it once.
                    return;
        }
    }
}

/*---------------------------------------------------------------------------*/
/**	This virtual function is what is called on a NmeaSource to interpret the
 *	passed NMEA string.  
 *  \note This base function handles the lookup processing.
 *  \sa NmeaSource::printString()
 *  \author briana Sullivan (bs)
 *  \date 8/12/05
 */
int NmeaSource::process(char * s)
{
	int watchName;

	watchName = lookup(s);
	switch (watchName)
	{
		case GZTIME:
			if (fieldsFound)
				lastTimeRead += offsetTime;
			//fall through
		case GZTOFFSET:
			return DONE_PROCESSING;
	}
	return watchName;
}

/*---------------------------------------------------------------------------*/
/**	This function is what is called on a NmeaSource to print the
 *	passed NMEA string in the logfile.  
 *  \note This base function handles the logging capability,
 *	tagging each output string with a "@GZTIM" entry if shouldLogGzTime==GL_TRUE.
 *	When reading from a logfile, it looks for "@GZTIM" and "@GZTOF" strings
 *	to do appropriate synchronization of NMEA streams.
 *  \author briana Sullivan (bs)
 *  \date 8/12/05
 */
void NmeaSource::printString(char * s)
{
	int i;
	char *c = s;

	if(logfileController->log)
	{
		if (logfileController->shouldLogGzTime)
			fprintf(logfileController->log, "@GZTIM,%.3f\n", gztime->getGZTime() - logfileController->offsetTime);
		for (i = 0; i < fieldsFound; i++)//restore the commas
		{
			while(*c != '\0')
				c++; //fast-forward to next NULL character
			*c = ',';
		}
		fprintf(logfileController->log,"%s\n",s);
		*c = '\0';
		fflush(logfileController->log);
	}
}

/*---------------------------------------------------------------------------*/
/** This function closes the current logfile and opens a new one.  If the
 *	new_filename is NULL, it just closes the current logfile without opening
 *	a new one.  If we were sharing a logfile before, we no longer are.
 *  \author Matt Plumlee (mdp)
 *  \date 8/15/2005
 */
void NmeaSource::changeLogfile(fs::path new_filename)
{
	if (log)
	{
		fclose(log);
		log = NULL;
	}

	if(!new_filename.empty())
	{
		log = fopen(new_filename.string().c_str(),"a");
		offsetTime = gztime->getGZTime();
		fprintf(log, "@GZTOF,%f\n", offsetTime); //put in the current time as offset
	}
	logfileController = this;
}

/*---------------------------------------------------------------------------*/
/** This function closes the current input file and opens a new one.  If the
 *	new_filename is NULL, it just closes the current input file without opening
 *	a new one.
 *  \author Matt Plumlee (mdp)
 *  \date 8/31/2005
 */
void NmeaSource::changeInputFile(const string& new_filename)
{
	if (infile)
	{
		fclose(infile);
		infile = NULL;
		connected = false;
		flen = 0;
	}

	if(!new_filename.empty())
	{
		infile = fopen(new_filename.c_str(), "r");
		if(!infile)
			cerr << "Error opening NMEA source file: " << new_filename << endl;
		else
		{
			connected = true;
			fseek(infile,0,SEEK_END);
			flen = ftell(infile);
			rewind(infile);
			resetStates();
		}
	}
}

/*---------------------------------------------------------------------------*/
int NmeaSource::isType(const char * s, const char * t)
{
    if((s[3] == t[0]) && (s[4] == t[1]))
        if(!t[2] || (s[5]==t[2]))
            return 1;
    return 0;
}

/*---------------------------------------------------------------------------*/
double NmeaSource::getFileProgress()
{
	if(!infile)
		return 1.0;
	return ((double)(ftell(infile)))/((double)(flen));
}

/*---------------------------------------------------------------------------*/
/**	This watchFor function creates a new WatchRecord for grabbing a single
 *	quantity from a particular NMEA string.  The watchFor functions allow
 *	the lookup() function to match the pattern indicated by the designator
 *	and type, then fill the var (of type ltype) from data in the indicated
 *	NMEA slot (0 is the first field after the first comma), and
 *	then return the associated watchName.
 *  Example: depthSounder->watchFor('$', "DBS", NmeaSource::GENERIC_DATA, 2, NmeaSource::Float, &sounderDepth);
 */
void NmeaSource::watchFor(char designator, const char *type, int watchName,
						  int slot, NMEA_ValueType ltype, void *var)
{
	int i;
	WatchRecord *record;

	record = addWatchRecord(designator, type, watchName, slot + 1);
	if (slot < 0)
		return;

	record->types = new NMEA_ValueType[slot + 1];
	record->vars = new void*[slot + 1];

	for (i = 0; i < slot; i++)
		record->types[i] = Skip;

	record->types[slot] = ltype;
	record->vars[slot] = var;
}

/*---------------------------------------------------------------------------*/
/**	This watchFor function creates a new WatchRecord for grabbing multiple
 *	quantities from a particular NMEA string.  The watchFor functions allow
 *	the lookup() function to match the pattern indicated by the designator
 *	and type, then fill the vars (of type types) from data in the NMEA slots
 *	(0 is the first field after the first comma), and then return the
 *	associated watchName.  nSlots should be equal to the number of items
 *	in types and vars, but need not be reflective of the number of slots
 *	actually appearing in the particular NMEA string.
 */
void NmeaSource::watchFor(char designator, const char *type, int watchName,
						  int nSlots, NMEA_ValueType *types, void **vars)
{
	WatchRecord *record;

	record = addWatchRecord(designator, type, watchName, nSlots);
	record->types = types;
	record->vars = vars;
}

/*---------------------------------------------------------------------------*/
/**	This protected function simply makes sure the watchRecords array is
 *	big enough for a new record, and then fills the new record with the
 *	passed info.
 */
NmeaSource::WatchRecord *NmeaSource::addWatchRecord(char designator,
								const char *type, int watchName, int nSlots)
{
	WatchRecord *newRecords, *record;
	int i;

	if (nWatchRecords >= nWatchRecordsAlloced)
	{
		nWatchRecordsAlloced += WATCH_RECORD_QUANTUM;
		newRecords = new WatchRecord[nWatchRecordsAlloced];
		for (i = 0; i < nWatchRecords; i++)
		{
			newRecords[i].designator = watchRecords[i].designator;
			newRecords[i].type = watchRecords[i].type;
			newRecords[i].watchName = watchRecords[i].watchName;
			newRecords[i].nSlots = watchRecords[i].nSlots;
			newRecords[i].types = watchRecords[i].types;
			newRecords[i].vars = watchRecords[i].vars;
		}
		if (watchRecords)
			delete [] watchRecords;
		watchRecords = newRecords;
	}
	record = &watchRecords[nWatchRecords++];
	record->designator = designator;
	record->type = type;
	record->watchName = watchName;
	record->nSlots = nSlots;
	return record;
}

/*---------------------------------------------------------------------------*/
/**	This function tries to match the passed NMEA string to the patterns
 *	set up by calls to watchFor().  If a match is found, the variables listed
 *	in the associated watchFor() call are filled using the information in the
 *	NMEA string, and the associated watchName is returned.
 */
int NmeaSource::lookup(char *s)
{
	char *info;
	int i, nSlots, done = GL_FALSE;
	NMEA_ValueType *types;
	void **vars;
	char *newString;
	bool match;

	bufferPosition = NULL;
	fieldsFound = 0;
	if (filterType != FilterOff)
	{
		match = (s[1] == filterPrefix[0] && s[2] == filterPrefix[1]);
		if ((filterType == FilterOutAllExceptPrefix && !match) ||
			(filterType == FilterOutPrefix && match))
		{
			bufferPosition = ++s;
			return NOT_PROCESSED;
		}
	}
	for (i = 0; i < nWatchRecords; i++)
	{
		if (watchRecords[i].designator == s[0] && isType(s, watchRecords[i].type))
		{
			//the code in this block only gets executed once per call (at most)
			types = watchRecords[i].types;
			vars = watchRecords[i].vars;
			nSlots = watchRecords[i].nSlots;
			//fast-forward in s to the next comma
			while (*s != ',' && *s != '\0')
				s++;
			if (*s == '\0')
				done = GL_TRUE;

			for (fieldsFound = 0; (fieldsFound < nSlots) && !done; fieldsFound++)
			{
				info = ++s; //get past the comma position
				if (!info || !*info) //the silly thing ended in a comma
				{
					done = GL_TRUE;
					break;
				}	

				//fast-forward in s to the next comma and put a terminator there
				while (*s != ',' && *s != '\0')
					s++;
				if (*s == '\0')
					done = GL_TRUE;
				else
					*s = '\0';

				switch (types[fieldsFound])
				{
					case Char:
						*((char *)vars[fieldsFound]) = *info;
						break;
					case Integer:
						*((int *)vars[fieldsFound]) = atoi(info);
						break;
					case Float:
						*((float *)vars[fieldsFound]) = (float)atof(info);
						break;
					case Double:
						*((double *)vars[fieldsFound]) = atof(info);
						break;
					case TempString:
						*((char **)vars[fieldsFound]) = info;
						break;
					case CopyString:
						newString = new char[strlen(info) + 1];
						strcpy(newString, info);
						*((char **)vars[fieldsFound]) = newString;
						break;
					default:
						break;
				}
			}
			bufferPosition = ++s;
			return watchRecords[i].watchName;
		}
	}
	bufferPosition = ++s;
	return NOT_PROCESSED;
}
