/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Ais.h

#ifndef AIS_H
#define AIS_H

#include <stdio.h>
#include "AisVhf_specs.h"
#include "NmeaSource.h"
#include <filesystem>
#include <gz4d/geo/Projection.h>

class Socket;
class Window3D;
//class BasicVessel;
class ObjectCollection;

//struct AisTargetSpec;
class BasicAisVessel;

#define NUM_PACKET_STRINGS 10
#define MAX_VHF_STRING_SIZE 169 //includes null termination
/*---------------------------------------------------------------------------*/
/** 
 *	This RTSource is used to monitor an AIS stream, and potentially create
 *	objects to represent messages coming in.
 *  \par Purpose: 
 *  To have a separate class that keeps track of specific AIS data for each vessel that transmits to us.
 *  \note Vessel contains the AIS data for our boat.
 *	 Rederived from NmeaSource by mdp 9/21/2004
 *	\author Matt Plumlee (mdp)
 *  \date 8/19/2004
 */
/*---------------------------------------------------------------------------*/
class Ais: public NmeaSource
{
    public:
        Ais(std::string a, int s, bool udp_flag = 0, fs::path log_filename = "");
        Ais(std::string a, int s, bool udp_flag, NmeaSource *logfileSource);
        Ais(std::string const &fn);
        ~Ais();

	//These functions fit the generic pattern for converting AIS data to
	//C++ struct members (generally ints of some sort or strings).
        static bool unpackBinaryField(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);
        static bool unpackStringField(int slen, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);
        static bool unpackBinarySubfields(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);
        static bool unpackComstateSubfields(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);
        static unsigned char convertBinaryByte(char c);
        static void decodeSize(unsigned long &sz, const unsigned long *sizes, int &szi, bool &isString, bool &isOptional, bool &isType, AIS_ProcessingFunction &func);
	//void drawTargets(Window3D *win, const float *pos);

    private:
        enum AIS_WatchNames {AIS_VDO = 10, AIS_VDM, AIS_ALR};
        void init();
        virtual int process(char * s);
        AIS_MessageType convertAisVhf(char *info, AIS_Packet *packet);
        int convertAisSigned(unsigned int i, int nbits);
	//int garbageCollectTargets();
        void updateVesselInfo(BasicAisVessel *vessel, AIS_Packet &packet);
        virtual void resetStates(); //reset things that should be reset at each new file open.

        gz4d::geo::Projection proj;

        static ObjectCollection *targets;

        static char *info;
        static int numPackets, packetNum;

        static const unsigned long *getSizeArrayFor(int currentLocalIndex, int typeValue);
        static int lastDAC; //for remembering the DAC portion of a binary subheader, used in getSizeArrayFor

        char vhfString[MAX_VHF_STRING_SIZE];
        int vhfStringLen;
        static char *packetStrings[NUM_PACKET_STRINGS];
        static int nextPacketString;
};


#endif
