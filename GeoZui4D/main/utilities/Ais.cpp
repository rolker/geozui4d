/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Ais.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This RTSource is used to monitor an AIS stream, and potentially create
 *	objects to represent messages coming in.
 *
 *	Created by mdp 8/19/2004
 *	Rederived from NmeaSource by mdp 9/21/2004
 */
/*---------------------------------------------------------------------------*/
#include "utilities/Ais.h"
//#include "AisVhf_specs.h"
#include "AIS_BinaryFields.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <filesystem>
//#include <time.h>
#include "objects/BasicAisVessel.h"
#include "MainWindow.h"
#include "engine/Scene3D.h"
#include "engine/ObjectCollection.h"
#include "engine/TimeControl.h"

#define MAX_TARGETS 100
#define MAX_PACKET_STRING_LEN 162

#define MAKE_VESSELS

using namespace std;

extern MainWindow *mainWindow;

ObjectCollection *Ais::targets = NULL;
char *Ais::packetStrings[NUM_PACKET_STRINGS];
int Ais::nextPacketString = 0;
int Ais::lastDAC = 0;

///\bug these are her just because of the way VDOM_vars is created in the init... probably a bad thing to do it this way.
char *Ais::info = NULL;
int Ais::numPackets = 0, Ais::packetNum = 0;

AIS_ProcessingFunction AIS_ProcessComstate = Ais::unpackComstateSubfields;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Ais::init()
{
    static NMEA_ValueType VDOM_types[] = {Integer,      Integer,   Skip, Skip, TempString};
    static void *VDOM_vars[] = {&numPackets, &packetNum, NULL, NULL, &info};

    int i;

    if(!targets)
    {
        targets = new ObjectCollection;
        targets->SetLabel("AIS_Targets");
        mainWindow->getScene3D()->AddObject(targets,NULL);
    }

    for (i = 0; i < NUM_PACKET_STRINGS; i++)
    {
        packetStrings[i] = new char[MAX_PACKET_STRING_LEN];
    }
    resetStates(); //although already called by parent constructor, it was before our virtual table was updated to get the call right.

    watchFor('!', "VDO", AIS_VDO, 5, VDOM_types, VDOM_vars);
    watchFor('!', "VDM", AIS_VDM, 5, VDOM_types, VDOM_vars);
    watchFor('$', "ALR", AIS_ALR);
}

/*---------------------------------------------------------------------------*/
void Ais::resetStates()
{
    vhfStringLen = 0;
    nextPacketString = 0;
    NmeaSource::resetStates();
}

/*---------------------------------------------------------------------------*/
Ais::Ais(string a, int s, bool udp_flag, fs::path log_filename)
    : NmeaSource(a, s, udp_flag, log_filename)
{
    init();
}

/*---------------------------------------------------------------------------*/
Ais::Ais(string a, int s, bool udp_flag, NmeaSource *logfileSource)
    : NmeaSource(a, s, udp_flag, logfileSource)
{
    init();
}

/*---------------------------------------------------------------------------*/
Ais::Ais(std::string const &fn)
    : NmeaSource(fn)
{
    init();
}

/*---------------------------------------------------------------------------*/
Ais::~Ais()
{
    int i;

    for (i = 0; i < NUM_PACKET_STRINGS; i++)
        delete [] packetStrings[i];

    delete targets;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/** The process function decompresses the passed NMEA string into a more
 *	managable AIS_Packet.
 */
int Ais::process(char * s)
{
    int message;
	
    message = NmeaSource::process(s);
    switch (message)
    {
        case NOT_PROCESSED:
            cerr << s << endl; //let's see if we get anything besides alarms.
            break;
        default:
            break;
        case AIS_VDO:
        case AIS_VDM:
            bool isNew = false;
            char *nul;
            AIS_Packet packet;
            int aisType, len;
			//AisTargetSpec *currentTarget;
            BasicAisVessel * currentTarget = NULL;
            double lon, lat;
            float tmp;
            char name[11];
			//const double *gref = *mainWindow->getScene3D()->GetGeoRef();
			//BasicVessel *vessel;

            if (fieldsFound < 5)
                break;

            nul = info;
            len = 0;
            while (*nul != '\0')
                len++, nul++; //fast-forward to end

			//How many NMEA packets is this AIS message spread across?
            if (numPackets > 1) //skip the copying if it all fit in one info string
            {
				//copy the thing into vhfString until we get the whole thing.
                if (packetNum == 1)
                {
                    strcpy(vhfString, info);
                    vhfStringLen = len;
                }
                else
                {
                    strcpy(&vhfString[vhfStringLen], info);
                    vhfStringLen += len;
                }
            }
			//Once we get the whole thing, do the actual processing,
			//otherwise, skip it!
            if (packetNum == numPackets)
            {
				//This is where the actual conversion happens.
                if (numPackets == 1)
                    aisType = convertAisVhf(info, &packet);
                else
                    aisType = convertAisVhf(vhfString, &packet);

#ifdef MAKE_VESSELS
                if (message != AIS_VDO)
                {
                    if(targets->Reset())
                    {
                        currentTarget = NULL;
                    }
                    else
                    {
                        currentTarget = (BasicAisVessel*)(targets->GetObjectPtr());
                        while(currentTarget && currentTarget->mmsi != packet.common.mmsi)
                            currentTarget = (BasicAisVessel*)(targets->Next());
                    }
					
                    if(!currentTarget)
                    {
                        currentTarget = new BasicAisVessel;
                        isNew = 1;
                    }

                }

                if(!currentTarget)
                {
                    currentTarget = new BasicAisVessel;
                    isNew = 1;
                }
                currentTarget->mmsi = packet.positionReport.mmsi;

                if (aisType >= 1 && aisType <= 3)
                {
                    lat = currentTarget->lat = convertAisSigned(packet.positionReport.lat, aisLatBits) / 600000.0;
                    lon = currentTarget->lon = convertAisSigned(packet.positionReport.lon, aisLonBits) / 600000.0;
                    if (!isNew)
                    {
                        currentTarget->lastlat = currentTarget->lat;
                        currentTarget->lastlon = currentTarget->lon;
                    }
                    currentTarget->lat = lat;
                    currentTarget->lon = lon;
                    if (isNew)
                    {
                        sprintf(name, "%d", currentTarget->mmsi);
                        currentTarget->SetLabel(name);
                        currentTarget->calculateOffsets();
                        currentTarget->setWireframe();
                        currentTarget->lastlat = currentTarget->lat;
                        currentTarget->lastlon = currentTarget->lon;
						//if (currentTarget->danglingInfo.common.mid == 5)
							//updateVesselInfo(currentTarget, currentTarget->danglingInfo);
                    }
                    currentTarget->sog = packet.positionReport.sog * 0.1f; //&&& need to convert from knots to meters/s!
                    currentTarget->cog = packet.positionReport.cog * 0.1f;
                    tmp = packet.positionReport.rot / 4.733f;
                    currentTarget->rot = tmp * tmp;
                    currentTarget->lastContact = mainWindow->getScene3D()->timeControl->getGZTime();
                    if (message == AIS_VDM) //don't set own stuff (AIS should be getting it from another source anyway...)
                    {
                        currentTarget->SetLatLon(lat,lon,packet.positionReport.head,currentTarget->lastContact);
                        if (isNew)
                            targets->AddObject(currentTarget);
                        currentTarget->setCourse(currentTarget->cog, GL_TRUE);
                        currentTarget->setSpeed(currentTarget->sog);
                    }
                }
                else if (aisType == 5)
                {
                    /*
                    if (packet.shipReport.eta.month != 0)
                    {
                        cerr << "(" << packet.shipReport.mmsi << ") ";
                        cerr << "ETA: " << packet.shipReport.eta.month;
                        cerr << "/" << packet.shipReport.eta.day;
                        cerr << " @" << packet.shipReport.eta.hour;
                        cerr << ":" << packet.shipReport.eta.minute << endl;
                    }
                    */
                    if (isNew)
                        currentTarget->danglingInfo = packet; //&&& really need to create new strings for those in packet...
					//else //if (vessel)
						//updateVesselInfo(currentTarget, packet);
                }
				//else
#endif
				//if ((aisType < 0 || aisType > 5) && (aisType < 10 || aisType > 15) && aisType != 18 && aisType != 19)
					//cerr << "aisType = " << aisType << endl;
            }
            break;
    }
//	NmeaSource::printString(s);
    return message;
}

/*---------------------------------------------------------------------------*/
/**	The convertAisVhf function takes a complete AIS string (which may have
 *	been spread across a few NMEA packets) into an AIS_Packet data structure
 *	to simplify later processing.  The AIS data from the NMEA string is
 *	encoded in ASCII, where each 8-bit character encodes 6 bits of binary data.
 *
 *  Here's the strategy for doing the conversion.  Since the actual type of
 *	AIS_Packet is unknown until we read the first portion of the message, 
 *	we first assume a generic message format, aisCommon.  Every AIS message
 *	starts off looking like aisCommon, so this works fine... but it makes the
 *	logic a bit difficult to understand because the conditional in the main
 *	for-loop changes somewhat unexpectedly.  The "sizes" array initially has
 *	the 0-terminated list of field sizes (usually either in bits or
 *	characters) that is associated with aisCommon.  However, when a type
 *	field is processed, the "sizes" array can change (look for the "if"
 *	statement at the bottom of the loop).  If we have a packet specification
 *	for the AIS message we've got, the "aisType" is updated to hold the
 *	enumeration for that type, and the "sizes" array is updated to tell us
 *	how big each of the fields are in this newly recognized packet type.
 *
 *  OK, so that's the strangeness that happens at the outer level.  The next
 *	bit of strangeness has to do with the sizes ("sz") in the "sizes" arrays.
 *	This is handled by decodeSize(), which returns a lot of info by reference.
 *	Any other strangeness is taken care of in the unpack*() functions.
 */
AIS_MessageType Ais::convertAisVhf(char *info, AIS_Packet *packet)
{
//	static FILE *f = NULL;
    int len;  ///< the length of the incoming AIS info string (from NMEA string), in bytes
    int byte; ///< the index of the byte we're currently on in info
    int bit;  ///< the bit that we're on in that byte (0-5)
    int fld;  ///< which field we're currently unpcaking to in the packet->a array
    unsigned long sz;   ///< the number of bits or characters we're unpacking into the current field
    int szi;  ///< the current index into the size array of the message type we're unpacking
    const unsigned long *sizes;  ///< pointer to the size array we're using to unpack the message
    unsigned char val; ///< (converted) value of the byte we're currently on in info
    AIS_MessageType aisType; ///< the type (mid/message ID) we're treating this packet as
    bool isOptional, isString, isType=false; ///< flags set on call to decodeSize()
    AIS_ProcessingFunction func; ///< processing function set on call to decodeSize()

    len = strlen(info);
    byte = 0;
    bit = 5;
    fld = 0;
    aisType = aisCommon;
    sizes = AIS_PacketSpecs[aisCommon];

	//first, grab the character, and fit it into the bottom 6 bits appropriately
    val = convertBinaryByte(info[byte]);
    if (len == 0 || val >= 64)
    {
        if (len == 0)
            cerr << "Empty AIS packet!" << endl;
        else
            cerr << "Invalid AIS byte " << (int)info[byte] << endl;
        return aisError;
    }

    for (szi = 0; sizes[szi] != 0; szi++) //unpack the fields
    {
        decodeSize(sz, sizes, szi, isString, isOptional, isType, func);
        if (!func(sz, packet->generic.a, fld, info, val, byte, bit, len))
        {
            if (isOptional || isString)
            {
                if (isString)
                {
                    if (packet->generic.a[fld])
                        fld++; //we got something, anyway: short string.  Advance the field counter
                    else
                        return aisError; //missing string, not optional!!
                }
                break; //ok, no optional arg.  We're done!
            }
            else
                return aisError; //mandatory field is missing!  error!
        }
        if (isType)
        {
            aisType = (AIS_MessageType)packet->generic.a[fld - 1];
            if (aisType > aisLAST)
            {
                cerr << "Type " << aisType << " encountered\n";
                return aisError;
            }
            assert (aisType <= aisLAST); //non-standard message, or corrupted type value
            sizes = AIS_PacketSpecs[aisType];
            assert(sizes); //this should only be NULL if the AIS standard has been expanded and we've only partially prepared.
			//If we got here, we should be fillin' it up right
        }
    }
    packet->generic.n_fields = fld;
    return aisType;
}

/*---------------------------------------------------------------------------*/
int Ais::convertAisSigned(unsigned int i, int nbits)
{
    int retval = 0;
    long int mask =0x7FFFFFFF;

    mask >>= (32 - nbits); //grab all the bits we want except sign bit (can't grab sign bit because I need to shift in a 0)
    retval = i & mask;
    if (i & (1 << (nbits - 1))) //check the signbit
        retval |= (-1 ^ mask);

    return retval;
}

/*---------------------------------------------------------------------------*/
/**	This function decodes the funky AIS_whatever macros that are used in the
 *	size arrays of AisVhf_specs.h and AIS_BinaryFields.h.
 *	If sz is positive, it indicates that the field we're currently filling is
 *	a binary field.  If sz is greater than or equal to AIS_FUNCTION, then we
 *	process an extra size value and use that as a pointer to the processing
 *	function (passed back through func).  We then decrement sz by AIS_FUNCTION.
 *	If sz is now greater than or equal to AIS_BINARY_SUBFIELD, we set our
 *	processing function to unpackBinarySubfields() and knock AIS_BINARY_SUBFIELD
 *	off of sz.  If sz is now greater than or equal to AIS_FIELD_TYPE, we set
 *	isType to true and knock sz down by AIS_FIELD_TYPE.  The point of the isType
 *	flag is to let the caller know that it should re-evaluate which size array
 *	it is using based on this upcoming field.  If sz is now greater
 *	than AIS_OPTIONAL, we set isOptional to true and knock sz down to size.  Now,
 *	we know we're filling a field with sz-number of bits.  The point of isOptional is
 *	let the calling function know whether we should generate an error for having
 *	too little information for the packet (returning aisError indicating an error),
 *	or recognize that the packet is	fine ending where it did and returning
 *	normally (with the packet type enumeration).  
 *	If, on the other hand, sz is negative to start with, then it indicates
 *	that we're filling a textual field with -sz characters in it, isString
 *	is set to true, and func is set to unpackStringField.
 *	Unless otherwise specified above, the boolean flags are set to false and
 *	the func is unpackBinaryField.
 */
void Ais::decodeSize(unsigned long &sz, const unsigned long *sizes, int &szi, bool &isString, bool &isOptional, bool &isType, AIS_ProcessingFunction &func)
{
    sz = sizes[szi];
    if (sz > 0) //normal bit-moving
    {
        isString = false;
        if (sz >= AIS_FUNCTION)
        {
            sz -= AIS_FUNCTION;
            func = (AIS_ProcessingFunction)(void*)(sizes[++szi]);
			//sz may now holds an opcode rather than a size
        }
        else if (sz >= AIS_BINARY_SUBFIELD)//&&&note, untested
        {
            sz -= AIS_BINARY_SUBFIELD;
            func = unpackBinarySubfields;
        }
        else
            func = unpackBinaryField;
        if (sz >= AIS_FIELD_TYPE)
        {
            sz -= AIS_FIELD_TYPE;
            isType = true;
        }
        else
            isType = false;
        if (sz >= AIS_OPTIONAL)
        {
            isOptional = true; //&&&note, this hasn't been tested yet...
            sz -= AIS_OPTIONAL;
        }
        else
            isOptional = false;
    }
    else //string conversion
    {
        sz = AIS_STRING(sz);
        isString = true;
        isOptional = false;
        func = unpackStringField;
    }
}

/*---------------------------------------------------------------------------*/
///	This function unpacks a chunk of sz_or_op bits of binary data from info into
//	the results array at position resultIndex.  It uses infoByte, infoBit, and
//	val to determine where to start from in info, and updates these values so
//	they are updated for the next call to this or a related function.  Returns
//	true and increments the resultIndex if the whole field was obtained, and
//	simply returns false otherwise.
/*	Since we're filling a binary field, we simply unpack the bits from
 *	6-bit bins into 8-bit bins.  "infoByte" tells us which 6-bit byte
 *	we're on in	the passed NMEA string, while "infoBit" tells us which
 *	bit we're on in that byte.  The loop that fills the AIS_Packet data
 *	structure is built around the 8-bit byte of that target structure
 *	(our place in which is kept track of using "resultIndex" and "resultBit").
 *	One strange thing here is that we're not filling the structure as
 *	if we know what each data item in the structure is, but we instead
 *	fill it as if it is composes of generic 32-bit fields (results).  This makes
 *	the user's life easy (since all the binary data is in chunks of 32
 *	bits or less anyway), but makes this code look a bit obscure.  It
 *	works through the magic of C/C++ unions.
 */
bool Ais::unpackBinaryField(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
{
    int resultBit;
    unsigned int res;

    results[resultIndex] = 0;
    for (resultBit = sz_or_op - 1; resultBit >= 0; resultBit--)
    {
        res = ((val & (1 << infoBit)) != 0); //now res holds the value of the current bit
        results[resultIndex] |= res << resultBit;
        infoBit--;
        if (infoBit < 0)
        {
            infoBit = 5;
            if (++infoByte > infoLen)
            {
                if (resultBit == 0) //We may not need any more info, so don't panic!
                    break;
                return false; //we ran out of bytes... must have been a transmission error or an optional arg!
            }
            val = convertBinaryByte(info[infoByte]);
            if (val >= 64 && info[infoByte] != 0)
            {
                cerr << "Invalid AIS byte " << (int)info[infoByte] << endl;
                return false;
            }
        }
    }
    resultIndex++;
    return true;
}

/*---------------------------------------------------------------------------*/
///	This function unpacks a chunk of slen bytes of 6-bit binary data from info into
//	the results array at position resultIndex.  It uses infoByte, infoBit, and
//	val to determine where to start from in info, and updates these values so
//	they are updated for the next call to this or a related function.  Returns
//	true and increments the resultIndex if the whole field was obtained, and
//	simply returns false otherwise.
/*	We're filling a text field here, so we make the results[resultIndex]
 *	hold a pointer to the text we're about to create.  Because most
 *	text we expect will be short, we're using a temprorary store of
 *	already-allocated strings to handle strings shorter than
 *	MAX_PACKET_STRING_LEN.  THIS IS A BAD HACK.  The user doesn't really
 *	have a good way of telling whether the string needs to be copied
 *	or can be used as is.  It needs to be copied if it is one of these
 *	shorter temp strings, but needs to be freed if it is longer and
 *	not being used!  A better strategy needs to be determined once the
 *	way it will be used is better understood.  If the user doesn't copy
 *	the strings over but uses them as-is, they'll collide when processing
 *	brings us back around to the same spot in the ring-buffer.
 *	Unpacking of the string
 *	happens similarly to binary data, except that any 8-bit byte
 *	that ends up of lesser numerical value than a space (' ') gets
 *	the '@' character added to it.  Finally, once the string has been
 *	assembled, all trailing '@'s are truncated and replaced with a
 *	null-terminator, which is my understanding of their convention.
 */
bool Ais::unpackStringField(int slen, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
{
    char *string;
    int i, charBit;
    unsigned int res;
    bool retval = true;

    results[resultIndex] = 0;
    if (slen < MAX_PACKET_STRING_LEN)
    {
        string = packetStrings[nextPacketString++]; //grab a temp string...
        if (nextPacketString >= NUM_PACKET_STRINGS)
            nextPacketString = 0;
    }
    else
        string = new char[slen + 1];

    for (i = 0; (i < slen) && retval; i++) //don't want to re-use resultIndex, because we're still using it
    {
        string[i] = '\0';
        for (charBit = 5; charBit >= 0; charBit--)
        {
            res = ((val & (1 << infoBit)) != 0); //now res holds the value of the current bit
            string[i] |= res << charBit;
            infoBit--;
            if (infoBit < 0)
            {
                infoBit = 5;
                if (++infoByte > infoLen) //we ran out of bytes... must have been a transmission error or an optionally short string!
                {
                    if (i == slen - 1) //We may not need any more info, so don't panic!
                        break;
                    retval = false;
                    break;
                }
                val = convertBinaryByte(info[infoByte]);
                if (val >= 64 && info[infoByte] != 0)
                {
                    cerr << "Invalid AIS byte " << (int)info[infoByte] << endl;
                    return false;
                }
            }
        }
        if (string[i] < ' ') //convert back into 8-bit ASCII (' ' and above are already there)
            string[i] += '@';
    }
    string[i] = '\0';
    results[resultIndex] = (unsigned long)string;
    for (i--; i >= 0; i--) //convert trailing @ symbols into nulls
    {
        if (string[i] == '@')
            string[i] = '\0';
        else
            break;
    }
    resultIndex++;
    return retval;
}

/*---------------------------------------------------------------------------*/
/**	AIS NMEA strings include binary data encoded in a particular way.  This
 *	function takes a plain-text character from a "binary" NMEA string and
 *	converts it into an unsigned char with the bottom 6 bits filled as is
 *	appropriate for binary data.  One should concatenate the bits returned
 *	into a contiguous bit stream to properly interpret it.
 */
unsigned char Ais::convertBinaryByte(char c)
{
    int val;

    val = c - 48; //they skip some non-printables
    if (val >= 40) //and also some funky characters, so NMEA doesn't misinterpret
        val -= 8;

    return val;
}

/*---------------------------------------------------------------------------*/
/**	This function processes a group of binary subfields in a way similar to
	convertAisVhf, but more generically (to support nested calls).  It uses
	infoByte, infoBit, and val to determine where to start from in info, and
	updates these values so they are updated for the next call to this or a
	related function.  Returns true and increments the resultIndex if the whole
	field was obtained, and	simply returns false with resultIndex updated to
	the last full field otherwise.  This function creates a new AIS_BinaryFields
	structure every time it is called, and returns the pointer to it through
	the current location it the results array.  REMEMBER TO DELETE IT when done!
 */
bool Ais::unpackBinarySubfields(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
{
    const unsigned long *local_sizes;
    int local_szi = 0;
    unsigned long sz;
    int local_index = 0;
    bool isString, isOptional, isType = false;
    AIS_ProcessingFunction func;
    int messageType;
    AIS_BinarayFields *binFields = new AIS_BinarayFields;
    unsigned long *local_results = binFields->a;

    local_sizes = AIS_IFM_CommonSizes;
    results[resultIndex++] = (unsigned long) binFields;

    for (local_szi = 0; local_sizes[local_szi] != 0; local_szi++) //unpack the fields
    {
        decodeSize(sz, local_sizes, local_szi, isString, isOptional, isType, func);
        if (!func(sz, local_results, local_index, info, val, infoByte, infoBit, infoLen))
        {
            if (isOptional || isString)
            {
                if (isString)
                {
                    if (local_results[local_index])
                        local_index++; //we got something, anyway: short string.  Advance the field counter
                    else
                        return false;//got nothin!
                }
                break; //ok, no optional arg.  We're done!
            }
            else
                return false; //mandatory field is missing!  error!
        }
        if (isType)
        {
            messageType = local_results[local_index - 1];
            local_sizes = getSizeArrayFor(local_szi, messageType);
            if (!local_sizes)
            {
				//if (local_index <= 2)
					//cerr << "Unrecognized binary message (DAC = " << local_results[0] << "; FI = " << local_results[1] << ")\n";
                return false; //nuttin' we can do!
            }
			//assert(local_sizes); //unhandled message... don't bother trying to parse the rest of it out.
			//If we got here, we should be fillin' it up right
        }
    }
    return true;
}

/*---------------------------------------------------------------------------*/
/**	This function processes com_state fields.  If sz_or_op is aisComstate_Dynamic,
	then the choice of what type of com_state to read is based on the next
        bit in the info stream.  If sz_or_op is aisComstate_SOTDMA, then a SOTDMA
	com_state is expected.  If sz_or_op is aisComstate_ITDMA, then an ITDMA
	com_state is expected.
 */
bool Ais::unpackComstateSubfields(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
{
    AIS_ComstateType type = (AIS_ComstateType) sz_or_op;

    if (type == aisComstate_Dynamic)
    {
        unsigned typeArg = resultIndex;
        if (!CALL_UNPACK_BINARY_FIELD(1))
            return false;
        type = (AIS_ComstateType)results[typeArg];
    }
    else
        results[resultIndex++] = type; //we didn't read this in, but we need this to fill the AIS_CommunicationState structure properly.

	//at this point, we have either SOTDMA or ITDMA.  In either case, we start with a 2-bit sync state.
    if (!CALL_UNPACK_BINARY_FIELD(2))
        return false;

	//now it gets interesting.  Let's do ITDMA first.  It's easier.
    if (type == aisComstate_ITDMA)
    {
        if (!CALL_UNPACK_BINARY_FIELD(13)) //get the increment
            return false;
        if (!CALL_UNPACK_BINARY_FIELD(3)) //get n_slots
            return false;
        if (!CALL_UNPACK_BINARY_FIELD(1)) //get keep
            return false;
    }
    else //assume SOTDMA.
    {
        unsigned long timeout, timeoutArg;
        timeoutArg = resultIndex;
		
		//first, get the timeout value
        if (!CALL_UNPACK_BINARY_FIELD(3))
            return false;
        timeout = results[timeoutArg];

		//now, get the remaining args based on what timeout is.
        if (timeout == 1) //this guy has 3 fields.
        {
            if (!CALL_UNPACK_BINARY_FIELD(5)) //hour
                return false;
            if (!CALL_UNPACK_BINARY_FIELD(7)) //minute
                return false;
            if (!CALL_UNPACK_BINARY_FIELD(2)) //spare
                return false;
        }
        else //all the rest of the fields (though different) are 14 bits
        {
            if (!CALL_UNPACK_BINARY_FIELD(14)) // either offset, slot, or received
                return false;
        }
    }
    return true;
}

/*---------------------------------------------------------------------------*/
/** For binary subfield processing (mid=6 or 8 at the top level), we need to
 *	determine which size array to use by combining information from two fields.
 *	The first is the DAC.  When currentLocalIndex is 0, we're dealing with a
 *	DAC and we store that in a static class variable.  When the DAC is 1, we're
 *	dealing with an IAI subfield that we know how to deal with, so if we later
 *	get called with currentLocalIndex=1, we can return the appropriate size
 *	array to the caller.
 *	\bug This function should really be set up to pass off processing of
 *	other DAC's to another getSizeArrayFor-like function.
 *	Example DAC's other than 1: 366 for the US or 316 for Canada.
 */
const unsigned long *Ais::getSizeArrayFor(int currentLocalIndex, int typeValue)
{
    int index;

	//on currentLocalIndex 0, we just hold onto the typeValue as the
	//DAC code for the message, and return the standard array sizes
	//for binary sub-packets
    if (currentLocalIndex == 0)
    {
        lastDAC = typeValue;
        return AIS_IFM_CommonSizes;
    }

	//If currentLocalIndex is 1, we should already have a DAC.  If
	//that DAC is 1, then we know to use one of the IAI array
	//size lists for binary sub-packets
    if (currentLocalIndex == 1 && lastDAC == 1)
    {
        if (typeValue < aisIAItargetsVTS)
            index = typeValue;
        else if (typeValue < aisIAIpersonsAboard)
            index = typeValue - 9;
        else if (typeValue < aisIAIcommon)
            index = typeValue - 31;
        else
            index = 10; //AIS_IFM_CommonSizes
        return AIS_BinaryPacketSpecs[index];
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/
/*	This procedure removes any old targets (or if there are too many, the one
 *	least recently updated) and returns the position to add the next target.
 */
/*int Ais::garbageCollectTargets()
{
	int i, minslot = -1;
	double now, tdiff, maxLag = 0.0;
	AisTargetSpec *currentTarget, *otherTarget;

	now = mainWindow->getScene3D()->timeControl->getGZTime();

	for (i = 1; i < nTargets; i++)
{
		currentTarget = &targets[i];
		tdiff = now - currentTarget->lastContact;
		if (tdiff > 600)
{
			maxLag = tdiff;
			minslot = -1;
			if (currentTarget->vessel)
{
				currentTarget->vessel->Unload();
				currentTarget->vessel = NULL;
}
			if (i < nTargets - 1) //put last target in dead spot, assume that target is live
{
				otherTarget = &targets[nTargets - 1];
				*currentTarget = *otherTarget;
				nTargets--;
				i--;
				continue;
}
			else
				return --nTargets;
}
		else
{
			if (tdiff > maxLag)
{
				maxLag = tdiff;
				minslot = i;
}
}
}
	if (minslot >= 0)
{
		targets[minslot].vessel->Unload();
		return minslot;
}

	return nTargets;
}*/

/*---------------------------------------------------------------------------*/
/*void Ais::drawTargets(Window3D *win, const float *pos)
{
	int i;
	AisTargetSpec *currentTarget;
	float xOffset = 0.0f; //pos[X]
	float yOffset = 0.0f; //pos[Y]
	const double zheight = -(*mainWindow->getScene3D()->GetGeoRef())[Z];

	glDisable(GL_LIGHTING);
	for (i = 0; i < nTargets; i++)
{
		currentTarget = &targets[i];
		glPointSize(5.0f);
		glBegin(GL_POINTS);
			glColor3f(0.5f, 0.5f, 0.5f);
			glVertex3f(currentTarget->laste - xOffset, currentTarget->lastn - yOffset, zheight);
			glColor3f(0.2f, 0.2f, 0.2f);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, zheight);
		glEnd();
		glPointSize(7.0f);
		glBegin(GL_POINTS);
			glColor3f(0.8f, 0.8f, 0.8f);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, zheight);
		glEnd();
		glLineWidth(1.0f);
		glBegin(GL_LINES);
			glColor3f(0.8f, 0.8f, 0.8f);
			glVertex3f(currentTarget->laste - xOffset, currentTarget->lastn - yOffset, zheight);
			glVertex3f(currentTarget->laste - xOffset, currentTarget->lastn - yOffset, -5000.0);
			glVertex3f(currentTarget->laste - xOffset, currentTarget->lastn - yOffset, zheight);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, zheight);
			glColor3f(0.2f, 0.2f, 0.2f);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, zheight);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, -5000.0);
		glEnd();
		glLineWidth(3.0f);
		glBegin(GL_LINES);
			glColor3f(0.8f, 0.8f, 0.8f);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, 0);
			glVertex3f(currentTarget->e - xOffset, currentTarget->n - yOffset, -5000.0);
		glEnd();
		glPointSize(1.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
}
	glEnable(GL_LIGHTING);
}*/

/*---------------------------------------------------------------------------*/
void Ais::updateVesselInfo(BasicAisVessel *vessel, AIS_Packet &packet)
{
    if (!vessel)
        return;

    vessel->setCallsign(packet.shipReport.callsign);
    vessel->setAisName(packet.shipReport.name);
    vessel->setDestination(packet.shipReport.destination);
    packet.common.mid = aisCommon;
}
