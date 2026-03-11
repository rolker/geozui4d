// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// Ais_BinaryFields.h
/*---------------------------------------------------------------------------*/
/** \file
 *	This file implements all the structures for the standard IFM binary data
 *	messages (the "Binary data" fields of messages 6 and 8) encoded according
 *	to ITU-R M.1371-1.  Page references refer to page numbers in this
 *	specification.  Hooks are provided for defining other binary messages,
 *	such as regional messages like the St. Lwarence Seaway specs.
 *	
 *	Created by mdp 2/14/2006
 */
/*---------------------------------------------------------------------------*/

#include "AisVhf_specs.h"

///Designated Area Codes (DAC): the regional authority that defines the format
///of binary messages with this code.  Supposedly the leading 3 decimal digits
///of the MMSI range for that authority (page 50).
#define AIS_DAC_INTERNATIONAL 1

///FI's (function identifiers) within the IAI (International Application Identifier) branch (pages 83-85).
///aisIAIcommon, aisIAIerror, and aisIAILAST are here by artistic license.
enum AIS_Intl_FI {aisIAIerror = -1, aisIAItext = 0, aisIAIappAcknowledge,
	  aisIAIinterrogateIFM, aisIAIinterrogateDAC, aisIAIintDACreply,
	  aisIAItargetsVTS = 16, aisIAIwaypointsShip, aisIAIwaypointsVTS,
	  aisIAIheightOverKeel, aisIAIpersonsAboard = 40, aisIAILAST = 40,
	  aisIAIcommon = 64};

enum {aisRadiusBits = 12, aisNumWaypointBits = 4, aisNumIdBits = 2, aisImoBits = 30, aisCallsignChars = 7}; //some useful names on often-used sizes

#define AIS_IFM_HEADER_SPEC \
	unsigned dac;		/* 10 bits, first 3 MMSI (decimal) digits of message definer*/ \
	unsigned fi;		/* 6 bits, function identifier (0-63) */
#define AIS_IFM_HEADER_BITS AIS_TYPE(10), AIS_TYPE(6)

///this enum goes with the ID type of the AIS_VTStarget structure.
enum AIS_VTSidType {aisVTS_MMSI = 0, aisVTS_IMO, aisVTS_callsign, aisVTS_other};

///This struct is used in IFM 16 for a VTS target
struct AIS_VTStarget {
	AIS_VTSidType	idType;		//2 bits, defaults to aisVTS_other, by the way
	union {
		char		*callsign;	//7 characters
		unsigned	mmsi;		//30 bits
		unsigned	imo;		//30 bits?  How big is an IMO number?
	} id;
	union {
		unsigned	callsign;	//4 bits
		unsigned	mmsi;		//16 bits
		unsigned	imo;		//16 bits?  46 - # bits anyway...
	} spare;					//brings total bits of id and spare to 46.
	unsigned		lat_x1000;	//24 bits, latitude in 1/1000 of a min
	unsigned		lon_x1000;	//25 bits, latitude in 1/1000 of a min
	unsigned		cog;		//9 bits, plain-old degrees for course
	unsigned		timestamp;	//6 bits, UTC second (0-59) or 60 if not available
	unsigned		sog;		//8 bits, speed over ground, 0-254 knots, 255 if not available
}; //120 bits
#define AIS_VTS_TARGET_BITS AIS_FUNC(AIS_decodeTargetVTS, 0)

///This struct is used in IFM 17 for a ship waypoint
struct AIS_waypoint {
	unsigned lon;	//28 bits
	unsigned lat;	//27 bits
}; //55 bits
#define AIS_WAYPOINT_BITS 28, 27

///This struct is used in IFM 18 for VTS waypoint advice
struct AIS_waypoint_VTS {
	unsigned lon;	//28 bits
	unsigned lat;	//27 bits
	unsigned radius;//12 bits, advised turning radius in meters (0-4095)
}; //67 bits
#define AIS_WAYPOINT_VTS_BITS 28, 27, 12

bool AIS_decodeWaypoint(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);
bool AIS_decodeTargetVTS(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);

/*---------------------------------------------------------------------------*/
/**	The IFM's below are defined similarly to the messages in AisVhf_specs.h.
*/
/*---------------------------------------------------------------------------*/
struct AIS_IFM_Common {
	AIS_IFM_HEADER_SPEC;
};
const unsigned long AIS_IFM_CommonSizes[] = {AIS_IFM_HEADER_BITS, 0};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
///IFM 0 (pages 86-87)
struct AIS_IAItext {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned ack_req;	//1 bit
	unsigned seq;		//11 bits
	unsigned text;		//up to 924 bits? up to 978 bits? spec is contradictory
}; //variable number of bits
const unsigned long AIS_IAItextSizes[] = {AIS_IFM_HEADER_BITS, 1, 11, AIS_STRING(163), 0};

/*---------------------------------------------------------------------------*/
///IFM 1 (page 87)
struct AIS_IAIappAcknowledge {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned ack_dac;	//10 bits
	unsigned seq;		//11 bits
	unsigned spare;		//3
}; //40 bits
const unsigned long AIS_IAIappAcknowledgeSizes[] = {AIS_IFM_HEADER_BITS, 10, 11, 3, 0};

/*---------------------------------------------------------------------------*/
///IFM 2 & 3 (page 87, 88) (int_fi is spare in IFM 3)
struct AIS_IAIintIFM {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned int_dac;	//10 bits
	unsigned int_fi;	//6 bits
}; //32 bits
const unsigned long AIS_IAIintIFMSizes[] = {AIS_IFM_HEADER_BITS, 10, 6, 0};

/*---------------------------------------------------------------------------*/
///IFM 4 (page 88)
struct AIS_IAIintDACreply {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned int_dac;	//10 bits
	unsigned has_fi[64];//128 bits (64 x 2-bit quantities: first bit says if indexed FI is available, second looks useless but may have a purpose some day.
	unsigned spare;		//6 bits
}; //160 bits (spec is contradictory... their total assumes has_fi is composed of 1-bit quantities
const unsigned long AIS_IAIintDACreplySizes[] = {AIS_IFM_HEADER_BITS, 10,
								AIS_FIELDx25(2), AIS_FIELDx25(2), AIS_FIELDx5(2),
								AIS_FIELDx5(2), AIS_FIELDx4(2), 6, 0};

/*---------------------------------------------------------------------------*/
///IFM 16 (page 88-89)
struct AIS_IAItargetsVTS {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	AIS_VTStarget target[7]; //up to 7x120 bits (8 fields in each 120 bit segment)
}; //856 bits maximum
const unsigned long AIS_IAItargetsVTSSizes[] = {AIS_IFM_HEADER_BITS, AIS_VTS_TARGET_BITS,
								AIS_FIELDx6(AIS_OPT(AIS_VTS_TARGET_BITS)), 0};

/*---------------------------------------------------------------------------*/
///IFM 17 (page 90)
struct AIS_IAIwaypoints {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned	n;			//4 bits, number of waypoints
	AIS_waypoint waypoint[14];//up to 14x55 bits (2 fields in each 55 bit segment)
	char *		route_name;	//120 bits (20 characters)
	unsigned	spare;		//0-7 bits
}; //144-912 bits
const unsigned long AIS_IAIwaypointsSizes[] = {AIS_IFM_HEADER_BITS, AIS_FUNC(AIS_decodeWaypoint, aisIAIwaypointsShip), AIS_STRING(20), 0};

/*---------------------------------------------------------------------------*/
///IFM 18 (pages 91-92)
struct AIS_IAIwaypointsVTS {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned	n;			//4 bits, number of waypoints
	AIS_waypoint_VTS waypoint[12];//up to 12x67 bits (3 fields in each 67 bit segment)
	char *		route_name;	//120 bits (20 characters)
	unsigned	spare;		//0-7 bits
}; //144-944 bits
const unsigned long AIS_IAIwaypointsVTSSizes[] = {AIS_IFM_HEADER_BITS, AIS_FUNC(AIS_decodeWaypoint, aisIAIwaypointsVTS), AIS_STRING(20), 0};

/*---------------------------------------------------------------------------*/
///IFM 19 (page 92)
struct AIS_IAIheight {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned height_x10;//11 bits, in 1/10 meters (0.1-204.7 meters, 0=not available)
	unsigned spare;		//5 bits
}; //32 bits
const unsigned long AIS_IAIheightSizes[] = {AIS_IFM_HEADER_BITS, 11, 5, 0};

/*---------------------------------------------------------------------------*/
///IFM 40 (page 92)
struct AIS_IAIpersons {
	AIS_IFM_HEADER_SPEC;//16 bits in 2 fields
	unsigned persons;	//13 bits (1-8190 people including crew; 0=not available; 8191=at least 8191 people)
	unsigned spare;		//3 bits
}; //32 bits
const unsigned long AIS_IAIpersonsSizes[] = {AIS_IFM_HEADER_BITS, 13, 3, 0};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
///This array holds the bit-size specs of each IFM as it is deciphered.
///These are selected by having a DAC of 1 and the appropriate FI.
const unsigned long *const AIS_BinaryPacketSpecs[] = {
						  AIS_IAItextSizes,				//FI=0
						  AIS_IAIappAcknowledgeSizes,	//FI=1...
						  AIS_IAIintIFMSizes,			//2
						  AIS_IAIintIFMSizes,			//3
						  AIS_IAIintDACreplySizes,		//4
						  AIS_IAItargetsVTSSizes,		//16 (slot 5)
						  AIS_IAIwaypointsSizes,		//17 (slot 6)
						  AIS_IAIwaypointsVTSSizes,		//18 (slot 7)
						  AIS_IAIheightSizes,			//19 (slot 8)
						  AIS_IAIpersonsSizes,			//40 (slot 9)
						  AIS_IFM_CommonSizes,			//slot 10
						  NULL
						 };

///This union allows us to unpack everything into the generic array a[].
///...but then access the data according to the structure of the message
///indicated by common.mid.
union AIS_BinarayFields {
	unsigned long a[66];	//maximum number of fields is 66 in any one of these messages.
	AIS_IFM_Common			common;
	AIS_IAItext				text;
	AIS_IAIappAcknowledge	appAck;
	AIS_IAIintIFM			interrogateIFMorDAC;
	AIS_IAIintDACreply		capabilityReply;
	AIS_IAItargetsVTS		targetsVTS;
	AIS_IAIwaypoints		waypoints;
	AIS_IAIwaypointsVTS		waypointsVTS;
	AIS_IAIheight			height;
	AIS_IAIpersons			persons;
};
