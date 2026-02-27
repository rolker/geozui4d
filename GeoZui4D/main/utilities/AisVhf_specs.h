/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// AisVhf_specs.h
/*---------------------------------------------------------------------------*/
#ifndef AISVHF_SPECS_H
#define AISVHF_SPECS_H

/** \file
 *	This file holds all the structure information for packets containing VHF
 *	information encoded according to ITU-R M.1371-1.  Page references refer
 *	to page numbers in this specification.
 *	
 *	As of 2/7/2006, this file is missing the definition for AIS_BinaryFields.
 *
 *	Created by mdp 8/19/2004
 */
/*---------------------------------------------------------------------------*/

///Message types as found on pages 40-41 of AIS specs.
///aisCommon, aisError, and aisLAST are here by artistic license.
enum AIS_MessageType {aisError = -1, aisCommon, aisPosition, aisPosition2, aisPosition3, aisTimeDate,
	  aisShip, aisAddressedBinary, aisBinaryAcknowledge, aisBroadcastBinary,
	  aisSARAircraft, aisUTCInquiry, aisUTC, aisSafetyMessage,
	  aisSafetyAcknowledge, aisSafetyBroadcast, aisInterrogation,
	  aisAssignedMode, aisGNSSBroadcast, aisClassBPosition,
	  aisClassBExtended, aisDataLink, aisATN, aisChannel, aisLAST = 22};

//These are lengths for some often-used fields.
enum {aisLonBits = 28, aisLatBits = 27, aisRotBits = 8, aisMmsiBits = 30}; //some useful names on often-used sizes

//There are a couple of different ways to define a com_state.
enum AIS_ComstateType {aisComstate_SOTDMA = 0, aisComstate_ITDMA = 1, aisComstate_Dynamic = 2};

///enum for navstat fields (messages 1, 2, 3, described on page 43)
enum AIS_NavigationalStatus {aisNavstat_underwayEngine = 0, aisNavstat_atAnchor,
	  aisNavstat_notUnderCommand, aisNavstat_restrictedManeuv, aisNavstat_constrainedByDraught,
	  aisNavstat_moored, aisNavstat_aground, aisNavstat_fishing, aisNavstat_underwaySail,
	  aisNavstat_futureUseHazardC, aisNavstat_futureUseHazardA, aisNavstat_futureUse1,
	  aisNavstat_futureUse2, aisNavstat_futureUse3, aisNavstat_futureUse4, aisNavstat_undefined};

///enum for pos_type fields (messages 4, 5, 19, and 21; best described on page 45)
enum AIS_PositionTypes {aisPosfix_undefined, aisPosfix_GPS, aisPosfix_GLONASS,
	  aisPosfix_combined_GPS_GLONASS, aisPosfix_LoranC, aisPosfix_Chayka,
	  aisPosfix_integrated, aisPosfix_surveyed
};

//enum for Tx/Rx mode (message 22; page 65)
enum AIS_TranamissionMode {aisDuplex, aisTransmitA, aisTransmitB}; //(always receive on both)

///enum for ship_cargo field (message 5; pages 47-48).
///Haz(ABCD) indicates dangerous goods, harmful substances, marine pollutants,
///or IMO hazards in that given category (A, B, C, or D).
enum AIS_CargoType {aisShip_unavailable = 0,
	  aisShip_WIG = 20, aisShip_WIG_HazA, aisShip_WIG_HazB, aisShip_WIG_HazC, aisShip_WIG_HazD, aisShip_WIG_Unknown = 29,
	  aisShip_FishingVessel = 30, aisShip_TowingVessel, aisShip_TowingWithLargeTow, aisShip_DredgingOrUnderwaterOps, aisShip_DivingOps, aisShip_MilitaryOps, aisShip_SailingVessel, aisShip_PleasureCraft,
	  aisShip_HSC = 40, aisShip_HSC_HazA, aisShip_HSC_HazB, aisShip_HSC_HazC, aisShip_HSC_HazD, aisShip_HSC_Unknown = 49,
	  aisShip_PilotVessel = 50, aisShip_SearchAndRescue, aisShip_Tug, aisShip_PortTender, aisShip_AntiPollution, aisShip_LawEnforcement, aisShip_LocalSpare1, aisShip_LocalSpare2, aisShip_MedicalTransport, aisShip_NonCombattant, //not sure if this last one is right (RR Resolution #18...Mob-83)
	  aisShip_Passenger = 60, aisShip_Passenger_HazA, aisShip_Passenger_HazB, aisShip_Passenger_HazC, aisShip_Passenger_HazD, aisShip_Passenger_Unknown = 69,
	  aisShip_Cargo = 70, aisShip_Cargo_HazA, aisShip_Cargo_HazB, aisShip_Cargo_HazC, aisShip_Cargo_HazD, aisShip_Cargo_Unknown = 79,
	  aisShip_Tanker = 80, aisShip_Tanker_HazA, aisShip_Tanker_HazB, aisShip_Tanker_HazC, aisShip_Tanker_HazD, aisShip_Tanker_Unknown = 89,
	  aisShip_Other = 90, aisShip_Other_HazA, aisShip_Other_HazB, aisShip_Other_HazC, aisShip_Other_HazD, aisShip_Other_Unknown = 99
};

///enum to say how a sending station is synchronized in its sending of messages
enum AIS_SynchronizationState {aisSynch_UTCdirect = 0, aisSynch_UTCindirect,
							   aisSynch_BaseStation, aisSync_OtherStation};

///These macros help overload the bit-sizes arrays with other information.
///...pertaining to strings or binary substructures.  They still relate to bit
///sizes, so I figure it's ok.
//#define AIS_STRING(x) -x //changing this probably will break something...
#define AIS_STRING(x) x
#define AIS_OPTIONAL (unsigned long)1000 //I'm using 1000 because there are no fields that have this many bits
#define AIS_OPT(x) AIS_OPTIONAL + x
#define AIS_FIELDx4(x) x, x, x, x
#define AIS_FIELDx5(x) x, x, x, x, x
#define AIS_FIELDx6(x) x, x, x, x, x, x
#define AIS_FIELDx25(x) AIS_FIELDx5(x), AIS_FIELDx5(x), AIS_FIELDx5(x), AIS_FIELDx5(x), AIS_FIELDx5(x)

#define AIS_FIELD_TYPE (2 * AIS_OPTIONAL)
#define AIS_TYPE(x) AIS_FIELD_TYPE + x

#define AIS_BINARY_SUBFIELD (4 * AIS_OPTIONAL)
#define AIS_BINFLD(x) AIS_BINARY_SUBFIELD + x
struct AIS_BinaryFields;

///This gets really funky--if special processing needs to be done on one or more arguments, this
///gives you the opportunity to specify a pointer to that function.  It must be of the following
///form: bool f(int sz_or_op, unsigned *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
///This is captured in the AIS_ProcessingFunction typedef.
#define AIS_FUNCTION (8 * AIS_OPTIONAL)
#define AIS_FUNC(x, opcode) AIS_FUNCTION + opcode, (unsigned long)(void*)x
typedef bool (* AIS_ProcessingFunction)(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen);

///Here are a couple of macros for calling the basic binary and string unpacking functions from other files
extern AIS_ProcessingFunction AIS_ProcessComstate;
#define CALL_UNPACK_BINARY_FIELD(size) Ais::unpackBinaryField(size, results, resultIndex, info, val, infoByte, infoBit, infoLen)
#define CALL_UNPACK_STRING_FIELD(size) Ais::unpackStringField(size, results, resultIndex, info, val, infoByte, infoBit, infoLen)

///This struct is used in messages 1, 2, 3, 4, 9, and 18.  It encodes
///information about the communication environment in terms of slots
///and the number of stations the sender is receiving from.  It is
///always the last part of the messages that it is a part of.  See
///pages 36-38 for a description.  \note This struct is not used yet...
struct AIS_CommunicationState {
	AIS_ComstateType type; /// aisComstate_SOTDMA or ITDMA... this is only set dynamically from AIS on message 18
	union{
		struct { ///SOTDMA
			AIS_SynchronizationState sync_state;///< 2 bits
			unsigned timeout;	///< 3 bits (Frames remaining before a new slot is selected)
			union {
				unsigned offset;///< 14 bits (on timeout=0: relative jump to my slot in the next frame)
				struct {
					unsigned hour;	///< 5 bits
					unsigned minute;///< 7 bits
					unsigned spare;	///< 2 bits
				} UTCtime;///(on timeout=1: current UTC time if this station has access to it)
				unsigned slot;		///< 14 bits (on timeout=2,4,6: this slot number (between 0 and 2249))
				unsigned received;	///< 14 bits (on timeout=3,5,7: number of other stations I'm currently receiving)
			} submessage;		///< 14 bits total
		} sotdma;
		struct { ///ITDMA
			AIS_SynchronizationState sync_state;///< 2 bits
			unsigned increment;	///< 13 bits (offset to next slot to be used or 0 if no more transmissions)
			unsigned n_slots;	///< 3 bits (add 1 to get number of consecutive slots to allocate there (0->1 slot...4->5 slots))
			unsigned keep;		///< 1 bit (set to 1 only if the slot remains allocated for one additional frame)
		} itdma;
	} info;
};
#define AIS_COMSTATE_BITS(type) AIS_FUNC(AIS_ProcessComstate, type)

///This union is used in messages 5 and 19 for the dimensions of the ship
///and/or reference for position.  See the bottom of page 48 for details.
///Distances are in meters.  If bow and stern (A and C) are both zero,
///then we only know length and width of the reporting vessel.  Otherwise,
///we know the distance of the reported position from each location on the
///vessel (bow, stern, port, starboard).
union AIS_DimensionReference{
	struct { ///following gives dimensions relative to reported position
		unsigned bow;		///< 9 bits
		unsigned stern;		///< 9 bits
		unsigned port;		///< 6 bits
		unsigned starboard;	///< 6 bits
	} ref;
	struct { ///if A=C=0 implies dimensions known, but ref. point for position is not.
		unsigned A;			///< 9 bits
		unsigned length;	///< 9 bits
		unsigned C;			///< 6 bits
		unsigned width;		///< 6 bits
	} dim;
};
#define AIS_DIMENSION_BITS 9, 9, 6, 6

///Here's a conversion macro for decodeing a rate-of-turn number (message 1, 2, 3)
#define AIS_GET_DEGREES_PER_MINUTE(x) (((char)x / 4.733) * ((char)x / 4.733) * ((x & 0x80)? -1 : 1))

#define AIS_HEADER_SPEC \
	int		n_fields;	/*< This is not part of the AIS spec, but helps disambiuguate situations involving optional info */ \
	AIS_MessageType mid; /*< 6 bits, indicates the type to be used to access the data */ \
	unsigned r_ind;		 /*< 2 bits, repeat indicator--how often this messages has appeared already */ \
	unsigned mmsi;		 /*< 30 bits, the MMSI number of the sender */
#define AIS_HEADER_BITS AIS_TYPE(6), 2, aisMmsiBits

/*---------------------------------------------------------------------------*/
/**	For the messages below, I wanted an easy way of filling the structure
	without having to know the names of the fields.  This works in a pretty
	straightforward manner for integer quantities 32 bits or smaller, but
	is a bit strange for characters.  Character strings are represented in
	the "bit size" arrays (below each structure) as a negative number.  The
	absolute value of this number indicates the number of *bytes* to pull
	out using the 6-bit encoding of ITU-R M.1371-1.  The strings are given
	their own separate storage and the pointer is inserted in the structure.
*/
/*---------------------------------------------------------------------------*/

struct AIS_Common {
	AIS_HEADER_SPEC;
};
const unsigned long AIS_CommonSizes[] = {AIS_HEADER_BITS, 0};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
///Message ID 1, 2, or 3 (Position reports, page 43)
struct AIS_PositionReport {
	AIS_HEADER_SPEC;	///< 38 bits in 3 fields
	AIS_NavigationalStatus navstat;	///< 4 bits (default=aisNavstat_undefined)
	unsigned rot;		///< 8 bits  (use AIS_GET_DEGREES_PER_MINUTE(rot) to get rate of turn in degrees per minute)
	unsigned sog;		///< 10 bits (speed over ground in tenths of knots. 1023 = unavailable, 1022 = 102.2 knots or greater)
	unsigned pos_acc;	///< 1 bit   (1=high: <10m, differential mode of e.g. DGNSS receiver; 0 = low: >10m)
	unsigned lon;		///< 28 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is East, - is West.  181 (0x6791AC0) = unavailable (default))
	unsigned lat;		///< 27 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is North, - is South.  91 (0x3412140) = unavailable (default))
	unsigned cog;		///< 12 bits (course over ground in 1/10th of a degree.  3600 (360.0) = unavailable (default))
	unsigned head;		///< 9 bits  (true heading in degrees, 511 = unavailable (default))
	unsigned timestamp;	///< 6 bits  (UTC timestamp of when report was generated.  60 = unavailable (default), 61 = positioning is in manual mode, 62 = electronic system in estimated (dead-reckoning) mode, 63 = positioning system inoperative)
	unsigned reg_app;	///< 4 bits  (Zero unless used by a regional authority to mean something else)
	unsigned spare;		///< 1 bit
	unsigned raim;		///< 1 bit	 (Receiver Autonomous Integrity Monitoring.  1 = in use, 0 = not in use)
	AIS_CommunicationState com_state;///< 19 bits (Broken down according to section 3.3.7.2.2 on page 36 for Messaged 1 and 2 or 3.3.7.3.2 on page 37 for Message 3.)
}; //168 bits
const unsigned long AIS_PositionReportSizes[] = {AIS_HEADER_BITS, 4, aisRotBits, 10, 1, aisLonBits, aisLatBits, 12, 9, 6, 4, 1, 1, AIS_COMSTATE_BITS(aisComstate_SOTDMA), 0};
const unsigned long AIS_PositionReport3Sizes[] = {AIS_HEADER_BITS, 4, aisRotBits, 10, 1, aisLonBits, aisLatBits, 12, 9, 6, 4, 1, 1, AIS_COMSTATE_BITS(aisComstate_ITDMA), 0};

/*---------------------------------------------------------------------------*/
///Message ID 4 (Base station report, page 44) or 11 (UTC and date response, p. 44)
struct AIS_TimeDateReport {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned year;		///< 14 bits (UTC year 1-9999.  0 = unavailable (default))
	unsigned month;		///< 4 bits  (UTC month 1-12.  ditto)
	unsigned day;		///< 5 bits  (UTC day 1-31.  ditto)
	unsigned hour;		///< 5 bits  (UTC hour 0-23. 24 = unavailable (default))
	unsigned minute;	///< 6 bits  (UTC minute 0-59. 60 = unavailable (default))
	unsigned second;	///< 6 bits  (UTC second 0-59. ditto)
	unsigned pos_acc;	///< 1 bit   (Position accuracy.  1 = high: <10m, differential mode of e.g. DGNSS receiver; 0 = low: >10m)
	unsigned lon;		///< 28 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is East, - is West.  181 (0x6791AC0) = unavailable (default))
	unsigned lat;		///< 27 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is North, - is South.  91 (0x3412140) = unavailable (default))
	AIS_PositionTypes pos_type;	///< 4 bits (default=aisPosfix_undefined)
	unsigned spare;		///< 10 bits
	unsigned raim;		///< 1 bit	 (Receiver Autonomous Integrity Monitoring.  1 = in use, 0 = not in use)
	AIS_CommunicationState com_state;///< 19 bits (Broken down according to section 3.3.7.2.2 on page 36.)
}; //168 bits
const unsigned long AIS_TimeDateReportSizes[] = {AIS_HEADER_BITS, 14, 4, 5, 5, 6, 6, 1, aisLonBits, aisLatBits, 4, 10, 1, AIS_COMSTATE_BITS(aisComstate_SOTDMA), 0};

/*---------------------------------------------------------------------------*/
///Message ID 5 (Ship static and voyage related data, page 45)
struct AIS_ShipReport {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned ais_ver;	///< 2 bits  (station compliant with AIS editions up to this value, currently 0)
	unsigned imo_num;	///< 30 bits (IMO number, 0 = unavailable (default))
	char *callsign;		///< 42 bits = 7x6 bits
	char *name;			///< 120 bits = 20x6 bits
	AIS_CargoType ship_cargo;///< 8 bits (default=aisShip_unavailable)
	AIS_DimensionReference dimensions; ///< (30 bits)
	AIS_PositionTypes pos_type;	///< 4 bits (default=aisPosfix_undefined)
	struct {
		unsigned month;		///< 4 bits (ETA month 1-12, 0 = unavailable (default))
		unsigned day;		///< 5 bits (ETA day 1-31, 0 = unavailable (default))
		unsigned hour;		///< 5 bits (ETA hour 0-23, 24 = unavailable (default))
		unsigned minute;	///< 6 bits (ETA minute 0-59, 60 = unavailable (default))
	} eta;				//(20 bits)
	unsigned draught;	///< 8 bits (Maximum present static draught in 1/10th meters; 255 = 25.5 m or greater, 0 = unavailable)
	char *destination;	///< 120 bits = 20x6 bits
	unsigned dte;		///< 1 bit  (Data terminal ready: 0 = available, 1 = not available (default))
	unsigned spare;		///< 1 bit
}; //424 bits
const unsigned long AIS_ShipReportSizes[] = {AIS_HEADER_BITS, 2, 30, AIS_STRING(7), AIS_STRING(20), 8, AIS_DIMENSION_BITS, 4, 4, 5, 5, 6, 8, AIS_STRING(20), 1, 1, 0};

/*---------------------------------------------------------------------------*/
///Message ID 6 (Addressed binary message, page 49).
///This is very complicated and requires additional structures to define,
///(using pointers to AIS_BinaryFields).
struct AIS_AddressedBinary {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned sequence;	///< 2 bits  (sequence number explained in section 5.3.1 on pages 76-77.  Basically a tag for ack-ing back to, I think.)
	unsigned dest_mmsi;	///< 30 bits (MMSI of the target station or vessel)
	unsigned retransmit;///< 1 bit   (1 = retransmitted, 0 = original)
	unsigned spare;		///< 1 bit
	AIS_BinaryFields *bf;///< 16-936 bits (check out AIS_BinaryFields.h)
}; //88-1008 bits
const unsigned long AIS_AddressedBinarySizes[] = {AIS_HEADER_BITS, 2, 30, 1, 1, AIS_BINFLD(936), 0};

/*---------------------------------------------------------------------------*/
///Message ID 7 (Binary acknowledge, page 51) or ID 13 (Safety related acknowledge, p. 51)
struct AIS_Acknowledge {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned mmsi_ack1;	///< 30 bits (I got your message, #1...)
	unsigned seqn_ack1;	///< 2 bits  (...and it's sequence number was this)
	unsigned mmsi_ack2;	///< 30 bits (optional) (I got your message, #2...)
	unsigned seqn_ack2;	///< 2 bits  (...and it's sequence number was this)
	unsigned mmsi_ack3;	///< 30 bits (optional) (I got your message, #3...)
	unsigned seqn_ack3;	///< 2 bits  (...and it's sequence number was this)
	unsigned mmsi_ack4;	///< 30 bits (optional) (I got your message, #4...)
	unsigned seqn_ack4;	///< 2 bits  (...and it's sequence number was this)
}; //72-168 bits
const unsigned long AIS_AcknowledgeSizes[] = {AIS_HEADER_BITS, 2, 30, 2, AIS_OPT(30), 2, AIS_OPT(30), 2, AIS_OPT(30), 2, 0};

/*---------------------------------------------------------------------------*/
///Message ID 8 (Broadcast binary message, page 52).
///This is very complicated and requires additional structures to define,
///(using pointers to AIS_BinaryFields).
struct AIS_BroadcastBinary {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	AIS_BinaryFields *bf;///< 16-968 bits (check out AIS_BinaryFields.h)
}; //56-1008 bits
const unsigned long AIS_BroadcastBinarySizes[] = {AIS_HEADER_BITS, 2, AIS_BINFLD(968), 0};

/*---------------------------------------------------------------------------*/
///Message ID 9 (Standard Search-and-Rescue aircraft position report, page 53)
struct AIS_SARAircraftReport {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned altitude;	///< 12 bits (GNSS-derived altitude in meters. 4094 = 4094 meters or higher, 4095 = unavailable)
	unsigned sog;		///< 10 bits (speed over ground in tenths of knots. 1023 = unavailable, 1022 = 102.2 knots or greater)
	unsigned pos_acc;	///< 1 bit   (1=high: <10m, differential mode of e.g. DGNSS receiver; 0 = low: >10m)
	unsigned lon;		///< 28 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is East, - is West.  181 (0x6791AC0) = unavailable (default))
	unsigned lat;		///< 27 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is North, - is South.  91 (0x3412140) = unavailable (default))
	unsigned cog;		///< 12 bits (course over ground in 1/10th of a degree.  3600 (360.0) = unavailable (default))
	unsigned timestamp;	///< 6 bits  (UTC timestamp of when report was generated.  60 = unavailable (default), 61 = positioning is in manual mode, 62 = electronic system in estimated (dead-reckoning) mode, 63 = positioning system inoperative)
	unsigned reg_app_8;	///< 8 bits  (Zero unless used by a regional authority to mean something else)
	unsigned dte;		///< 1 bit  (Data terminal ready: 0 = available, 1 = not available (default))
	unsigned spare;		///< 5 bit
	unsigned raim;		///< 1 bit	 (Receiver Autonomous Integrity Monitoring.  1 = in use, 0 = not in use)
	AIS_CommunicationState com_state;///< 19 bits (Broken down according to section 3.3.7.2.2 on page 36.)
}; //168 bits
const unsigned long AIS_SARAircraftReportSizes[] = {AIS_HEADER_BITS, 12, 10, 1, aisLonBits, aisLatBits, 12, 6, 8, 1, 5, 1, AIS_COMSTATE_BITS(aisComstate_SOTDMA), 0};

/*---------------------------------------------------------------------------*/
///Message ID 10 (UTC and date inquiry, page 54)
struct AIS_UTCInquiry {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned dest_mmsi;	///< 30 bits (MMSI of station to which this message is inquiring)
	unsigned spare2;	///< 2 bits
}; //72 bits
const unsigned long AIS_UTCInquirySizes[] = {AIS_HEADER_BITS, 2, 30, 2, 0};

/*---------------------------------------------------------------------------*/
///Message ID 12 (Addressed safety related message, page 54)
struct AIS_SafetyMessage {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned sequence;	///< 2 bits  (sequence number explained in section 5.3.1 on pages 76-77.  Basically a tag for ack-ing back to, I think.)
	unsigned dest_mmsi;	///< 30 bits (MMSI of the target station or vessel)
	unsigned retransmit;///< 1 bit   (1 = retransmitted, 0 = original)
	unsigned spare;		///< 1 bits
	char *text;			///< Max of 936 bits (= 156x6 bits)
}; //max of 1008 bits
const unsigned long AIS_SafetyMessageSizes[] = {AIS_HEADER_BITS, 2, 30, 1, 1, (156), 0};

/*---------------------------------------------------------------------------*/
///Message ID 14 (Safety related broadcast message, page 55)
struct AIS_SafetyBroadcast {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	char *text;			///< Max of 968 bits (= 161x6 bits plus 2 stuffing bits as needed)
}; //max of 1008 bits
const unsigned long AIS_SafetyBroadcastSizes[] = {AIS_HEADER_BITS, 2, AIS_STRING(161), 0};

/*---------------------------------------------------------------------------*/
///Message ID 15 (Interrogation, page 56)
struct AIS_Interrogation {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned mmsi_int1;	///< 30 bits (Hey, #1...)
	unsigned mid_int1_1;///< 6 bits  (...please send me a message of this type...)
	unsigned slot_int1_1;///< 12 bits (...and send it this many slots from here (0=when you please))
	unsigned spare2;	///< 2 bits (optional)
	unsigned mid_int1_2;///< 6 bits (optional) (...Oh, and #1 send me a message of this type as well (0=nevermind))
	unsigned slot_int1_2;///< 12 bits (...and send it this many slots from here (0=when you please))
	unsigned spare3;	///< 2 bits (optional)
	unsigned mmsi_int2;	///< 30 bits (optional) (Hey, #2...)
	unsigned mid_int2;	///< 6 bits  (...please send me a message of this type...)
	unsigned slot_int2;	///< 12 bits (...and send it this many slots from here (0=when you please))
	unsigned spare4;	//2 bits
}; //88-160 bits
const unsigned long AIS_InterrogationSizes[] = {AIS_HEADER_BITS, 2, 30, 6, 12, AIS_OPT(2), AIS_OPT(6), 12, AIS_OPT(2), AIS_OPT(30), 6, 12, 2, 0};

/*---------------------------------------------------------------------------*/
///Message ID 16 (Assigned Mode, page 57)
struct AIS_AssignedMode {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned mmsi_A;	///< 30 bits (Hey Station A...)
	unsigned offset_A;	///< 12 bits (...this is your slot interval...)
	unsigned increment_A;///< 10 bits (...starting this many slots from here)
	unsigned mmsi_B;	///< 30 bits (optional) (Hey Station B...)
	unsigned offset_B;	///< 12 bits (...this is your slot interval...)
	unsigned increment_B;///< 10 bits (...starting this many slots from here)
//	unsigned spare;		//0 or 4 bits... 0 bits if _B items are used, doesn't matter otherwise.
}; //96 or 144 bits
const unsigned long AIS_AssignedModeSizes[] = {AIS_HEADER_BITS, 2, 30, 12, 10, AIS_OPT(30), 12, 10, 0};

/*---------------------------------------------------------------------------*/
///Message ID 17 (GNSS broadcast binary message, page 58)
struct AIS_GNSSBroadcast {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned lon_minx10;///< 18 bits (intended-to-be-signed longitude of DGNSS station in 1/10th of a minute. + is East, - is West.  181 (0x1A838) = unavailable (default))
	unsigned lat_minx10;///< 17 bits (intended-to-be-signed latitude of DGNSS station in 1/10th of a minute. + is North, - is South.  91 (0xD548) = unavailable (default))
	unsigned spare2;	///< 5 bits
	unsigned DGNSS_type;///< 6 bits  (according to ITU-R M.823--I have no idea.)
	unsigned DGNSS_ID;	///< 10 bits (ITU-R M.823 station identifier)
	unsigned DGNSS_Z;	///< 13 bits (time value in 0.6 s increments)
	unsigned DGNSS_seq;	///< 3 bits  (message sequence number)
	unsigned DGNSS_N;	///< 5 bits  (Number of DGNSS data words following the two-word header: 0-29)
	unsigned DGNSS_health;///< 3 bits (Reference station health specified in ITU-R M.823 again)
	unsigned DGNSS_data[29];///< 0-696 bits (DGNSS_N message data words excluding parity (each is 24 bits))
}; //80-816 bits
const unsigned long AIS_GNSSBroadcastSizes[] = {AIS_HEADER_BITS, 2, 18, 17, 5, AIS_OPT(6), 10, 13, 3, 5, 3,
							AIS_FIELDx25(AIS_OPT(24)), AIS_OPT(24), AIS_OPT(24), AIS_OPT(24), AIS_OPT(24), 0};

/*---------------------------------------------------------------------------*/
///Message ID 18 (Standard Class B equipment position reports, page 59)
struct AIS_ClassBPositionReport {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned reg_app_8;	///< 8 bits  (Zero unless used by a regional authority to mean something else)
	unsigned sog;		///< 10 bits (speed over ground in tenths of knots. 1023 = unavailable, 1022 = 102.2 knots or greater)
	unsigned pos_acc;	///< 1 bit   (1=high: <10m, differential mode of e.g. DGNSS receiver; 0 = low: >10m)
	unsigned lon;		///< 28 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is East, - is West.  181 (0x6791AC0) = unavailable (default))
	unsigned lat;		///< 27 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is North, - is South.  91 (0x3412140) = unavailable (default))
	unsigned cog;		///< 12 bits (course over ground in 1/10th of a degree.  3600 (360.0) = unavailable (default))
	unsigned head;		///< 9 bits  (true heading in degrees, 511 = unavailable (default))
	unsigned timestamp;	///< 6 bits  (UTC timestamp of when report was generated.  60 = unavailable (default), 61 = positioning is in manual mode, 62 = electronic system in estimated (dead-reckoning) mode, 63 = positioning system inoperative)
	unsigned reg_app;	///< 4 bits  (Zero unless used by a regional authority to mean something else)
	unsigned spare;		///< 4 bits
	unsigned raim;		///< 1 bit	 (Receiver Autonomous Integrity Monitoring.  1 = in use, 0 = not in use)
	AIS_CommunicationState com_state;///< 20 bits (first bit: 0 = SOTDMA com_state (Section 3.3.7.2.2, p. 36), 1 = ITDMA com_state (Section 3.3.7.3.2 p.37).  Remaining bits are normal com_state)
}; //168 bits
const unsigned long AIS_CLassBPositionReportSizes[] = {AIS_HEADER_BITS, 8, 10, 1, aisLonBits, aisLatBits, 12, 9, 6, 4, 4, 1, AIS_COMSTATE_BITS(aisComstate_Dynamic), 0};

/*---------------------------------------------------------------------------*/
///Message ID 19 (Extended Class B equipment position reports, page 61)
struct AIS_ClassBExtendedReport {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned reg_app_8;	///< 8 bits  (Zero unless used by a regional authority to mean something else)
	unsigned sog;		///< 10 bits (speed over ground in tenths of knots. 1023 = unavailable, 1022 = 102.2 knots or greater)
	unsigned pos_acc;	///< 1 bit   (1=high: <10m, differential mode of e.g. DGNSS receiver; 0 = low: >10m)
	unsigned lon;		///< 28 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is East, - is West.  181 (0x6791AC0) = unavailable (default))
	unsigned lat;		///< 27 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is North, - is South.  91 (0x3412140) = unavailable (default))
	unsigned cog;		///< 12 bits (course over ground in 1/10th of a degree.  3600 (360.0) = unavailable (default))
	unsigned head;		///< 9 bits  (true heading in degrees, 511 = unavailable (default))
	unsigned timestamp;	///< 6 bits  (UTC timestamp of when report was generated.  60 = unavailable (default), 61 = positioning is in manual mode, 62 = electronic system in estimated (dead-reckoning) mode, 63 = positioning system inoperative)
	unsigned reg_app;	///< 4 bits  (Zero unless used by a regional authority to mean something else)
	char *name;			//120 bits = 20x6 bits
	AIS_CargoType ship_cargo;///< 8 bits (default=aisShip_unavailable)
	AIS_DimensionReference dimensions; ///< (30 bits)
	AIS_PositionTypes pos_type;	///< 4 bits (default=aisPosfix_undefined)
	unsigned raim;		///< 1 bit	 (Receiver Autonomous Integrity Monitoring.  1 = in use, 0 = not in use)
	unsigned dte;		///< 1 bit   (Data terminal ready: 0 = available, 1 = not available (default))
	unsigned spare;		///<5 bits
}; //312 bits
const unsigned long AIS_CLassBExtendedReportSizes[] = {AIS_HEADER_BITS, 8, 10, 1, aisLonBits, aisLatBits, 12, 9, 6, 4, AIS_STRING(20), 8, AIS_DIMENSION_BITS, 4, 1, 1, 5, 0};

/*---------------------------------------------------------------------------*/
///Message ID 20 (Data link management, page 62--this is sent by a base station to
///announce the slots reserved for 1 or more nearby base-stations.)
struct AIS_DataLink {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned offset1;	///< 12 bits (Starting this many slots from here...)
	unsigned n_slots1;	///< 4 bits (...n consecutive slots are reserved...)
	unsigned timeout1;	///< 3 bits (...for this many minutes...)
	unsigned increment1;///< 11 bits (...and this will be trure for every subsequent cycle of this many slots (or 0 if it doesn't cycle).)
	unsigned offset2;	///< 12 bits (optional) (Starting this many slots from here...)
	unsigned n_slots2;	///< 4 bits (...(a potentially different) n consecutive slots are reserved...)
	unsigned timeout2;	///< 3 bits (...for this many minutes...)
	unsigned increment2;///< 11 bits (...and this will be trure for every subsequent cycle of this many slots (or 0 if it doesn't cycle).)
	unsigned offset3;	///< 12 bits (optional) (Starting this many slots from here...)
	unsigned n_slots3;	///< 4 bits (...(a potentially different) n consecutive slots are reserved...)
	unsigned timeout3;	///< 3 bits (...for this many minutes...)
	unsigned increment3;///< 11 bits (...and this will be trure for every subsequent cycle of this many slots (or 0 if it doesn't cycle).)
	unsigned offset4;	///< 12 bits (optional) (Starting this many slots from here...)
	unsigned n_slots4;	///< 4 bits (...(a potentially different) n consecutive slots are reserved...)
	unsigned timeout4;	///< 3 bits (...for this many minutes...)
	unsigned increment4;///< 11 bits (...and this will be trure for every subsequent cycle of this many slots (or 0 if it doesn't cycle).)
//	unsigned spare2;	//0, 2, 4, or 6 bits, but doesn't matter here.
}; //72-160 bits
const unsigned long AIS_DataLinkSizes[] = {AIS_HEADER_BITS, 2, 12, 4, 3, 11, AIS_OPT(12), 4, 3, 11, AIS_OPT(12), 4, 3, 11, AIS_OPT(12), 4, 3, 11, 0};

/*---------------------------------------------------------------------------*/
///Message ID 21 (Aid-to-navigation report)
struct AIS_ATNReport {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned type;		///< 5 bits (fixed aid is 1-15, floating is 16-31, see IALA)
	char *name;			///< 120 bits = 20x6 bits
	unsigned pos_acc;	///< 1 bit   (1=high: <10m, differential mode of e.g. DGNSS receiver; 0 = low: >10m)
	unsigned lon;		///< 28 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is East, - is West.  181 (0x6791AC0) = unavailable (default))
	unsigned lat;		///< 27 bits (intended-to-be-signed longitude in 1/10,000 of a minute. + is North, - is South.  91 (0x3412140) = unavailable (default))
	AIS_DimensionReference dimensions; ///< (30 bits)
	AIS_PositionTypes pos_type;	///< 4 bits (default=aisPosfix_undefined)
	unsigned timestamp;	///< 6 bits  (UTC timestamp of when report was generated.  60 = unavailable (default), 61 = positioning is in manual mode, 62 = electronic system in estimated (dead-reckoning) mode, 63 = positioning system inoperative)
	unsigned off_pos;	///< 1 bit (only valid for floating aids hen timestamp <= 59: 1=off position, 0=in-the-right-place)
	unsigned reg_app_8;	///< 8 bits  (Zero unless used by a regional authority to mean something else)
	unsigned raim;		///< 1 bit	 (Receiver Autonomous Integrity Monitoring.  1 = in use, 0 = not in use)
	unsigned spare;		///< 3 bits
}; //272 bits
const unsigned long AIS_ATNReportSizes[] = {AIS_HEADER_BITS, 5, AIS_STRING(20), 1, aisLonBits, aisLatBits, AIS_DIMENSION_BITS, 4, 6, 1, 8, 1, 3, 0};

/*---------------------------------------------------------------------------*/
///Message ID 22 (Channel Management)
struct AIS_ChannelManagement {
	AIS_HEADER_SPEC;	//38 bits in 3 fields
	unsigned spare;		///< 2 bits
	unsigned chan_a;	///< 12 bits (channel number according to ITU-R M.1084)
	unsigned chan_b;	///< 12 bits (channel number according to ITU-R M.1084)
	AIS_TranamissionMode tx_rx;	///< 4 bits (defulat=aisDuplex)
	unsigned power;		///< 1 bit (0 = high, 1 = low)
	///note: following 4 items may also be addresses for 2 stations... :p
	unsigned lon_ne_minx10;//18 bits (intended-to-be-signed longitude of region's NE corner in 1/10 min; or 18 MSB of station 1's address)
	unsigned lat_ne_minx10;//17 bits (intended-to-be-signed latitude of region's NE corner in 1/10 min; or 12 LSB of station 1's address followed by 5 zeros)
	unsigned lon_sw_minx10;//18 bits (intended-to-be-signed longitude of region's SW corner in 1/10 min; or 18 MSB of station 2's address)
	unsigned lat_sw_minx10;//17 bits (intended-to-be-signed latitude of region's SW corner in 1/10 min; or 12 LSB of station 2's address followed by 5 zeros)
	unsigned addressed;	///< 1 bit (0 = geographical region, 1 = addressed; either way, specified by four previous fields)
	unsigned bandwidth_a;///< 1 bit (0 = default as specified by chan_a, 1 = 12.5kHz)
	unsigned bandwidth_b;///< 1 bit (0 = default as specified by chan_b, 1 = 12.5kHz)
	unsigned t_zone_size;///< 3 bits (transitioal zone size in nautical miles plus 1 (0 is 1nm, 7 is 8nm), default is 4 (5 nm))
	unsigned spare2;	///< 23 bits
}; //168 bits
const unsigned long AIS_ChannelManagementSizes[] = {AIS_HEADER_BITS, 2, 12, 12, 4, 1, 18, 17, 18, 17, 1, 1, 1, 3, 23, 0};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
///This array holds the bit-size specs of each AIS message as it is deciphered.
///The message ID (mid) for a given message selects the appropriate size array.
const unsigned long *const AIS_PacketSpecs[] = {AIS_CommonSizes,	//0
						  AIS_PositionReportSizes,		//1 (mid = 1)
						  AIS_PositionReportSizes,		//2 (mid = 2)
						  AIS_PositionReport3Sizes,		//3 (etc.)
						  AIS_TimeDateReportSizes,		//4
						  AIS_ShipReportSizes,			//5
						  AIS_AddressedBinarySizes,		//6
						  AIS_AcknowledgeSizes,			//7
						  AIS_BroadcastBinarySizes,		//8
						  AIS_SARAircraftReportSizes,	//9
						  AIS_UTCInquirySizes,			//10
						  AIS_TimeDateReportSizes,		//11
						  AIS_SafetyMessageSizes,		//12
						  AIS_AcknowledgeSizes,			//13
						  AIS_SafetyBroadcastSizes,		//14
						  AIS_InterrogationSizes,		//15
						  AIS_AssignedModeSizes,		//16
						  AIS_GNSSBroadcastSizes,		//17
						  AIS_CLassBPositionReportSizes,//18
						  AIS_CLassBExtendedReportSizes,//19
						  AIS_DataLinkSizes,			//20
						  AIS_ATNReportSizes,			//21
						  AIS_ChannelManagementSizes,	//22
						  NULL
						 };

///This union allows us to unpack everything into the generic array a[].
///..but then access the data according to the structure of the message
///indicated by common.mid.
union AIS_Packet {
	struct {int n_fields;
			unsigned long a[42];	//maximum number of fields is 42 in any one of these messages.
	}generic;
	AIS_Common				common;
	AIS_PositionReport		positionReport;
	AIS_TimeDateReport		timeDateReport;
	AIS_ShipReport			shipReport;
	AIS_AddressedBinary		addressedBinary;
	AIS_Acknowledge			acknowledgement;
	AIS_BroadcastBinary		broadcastBinary;
	AIS_SARAircraftReport	sarAircraftReport;
	AIS_UTCInquiry			utcInquiry;
	AIS_SafetyMessage		safetyMessage;
	AIS_SafetyBroadcast		safetyBroadcast;
	AIS_Interrogation		interrogation;
	AIS_AssignedMode		assignedMode;
	AIS_GNSSBroadcast		gnssBroadcast;
	AIS_ClassBPositionReport classBpositionReport;
	AIS_ClassBExtendedReport classBextendedReport;
	AIS_DataLink			dataLink;
	AIS_ATNReport			atnReport;
	AIS_ChannelManagement	channelManagement;
};

#endif
