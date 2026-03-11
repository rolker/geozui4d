// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// AIS_BinaryFields.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This file implements any functions required to make AIS_BinaryFields.h
 *	work.
 *
 *	Created by mdp 2/22/2006
 */
/*---------------------------------------------------------------------------*/
#include "utilities/Ais.h"
#include "AIS_BinaryFields.h"

/*---------------------------------------------------------------------------*/
/**	This function processes the bulk of the waypoint messages, since how many
	fields are expected depends on the first argument, and there is a text
	field after these waypoint fields.  This function does not handle the
	text field.
 */
bool AIS_decodeWaypoint(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
{
	int numWaypoints, i, waypointArg = resultIndex;

	//get the number of waypoints
	if (!CALL_UNPACK_BINARY_FIELD(aisNumWaypointBits))
		return false;
	numWaypoints = results[waypointArg];

	for (i = 0; i < numWaypoints; i++)
	{
		//get the Longitude of waypoint i
		if (!CALL_UNPACK_BINARY_FIELD(aisLonBits))
			return false;
		//get the Latitude of waypoint i
		if (!CALL_UNPACK_BINARY_FIELD(aisLatBits))
			return false;
		if (sz_or_op == aisIAIwaypointsVTS)
		{
			//get the advised turning radius of waypoint i
			if (!CALL_UNPACK_BINARY_FIELD(aisRadiusBits))
				return false;
		}
	}
	return true;
}

/*---------------------------------------------------------------------------*/
/**	This function processes the bulk of the waypoint messages, since how many
	fields are expected depends on the first argument, and there is a text
	field after these waypoint fields.  This function does not handle the
	text field.
 */
bool AIS_decodeTargetVTS(int sz_or_op, unsigned long *results, int &resultIndex, char *info, unsigned char &val, int &infoByte, int &infoBit, int infoLen)
{
	AIS_VTSidType type;
	int typeArg = resultIndex, nSpareBits;

	//get the field type
	if (!CALL_UNPACK_BINARY_FIELD(aisNumIdBits))
		return false;
	type = (AIS_VTSidType) results[typeArg];

	//get the target ID
	switch (type)
	{
		case aisVTS_callsign:
            if (!CALL_UNPACK_STRING_FIELD(aisCallsignChars))
				return false;
			nSpareBits = 46 - (aisCallsignChars * 6 /*bits per character*/);
			break;
		case aisVTS_MMSI:
            if (!CALL_UNPACK_BINARY_FIELD(aisMmsiBits))
				return false;
			nSpareBits = 46 - aisMmsiBits;
			break;
		case aisVTS_IMO:
		default: //doesn't really matter, field should be zero on default anyway
            if (!CALL_UNPACK_BINARY_FIELD(aisImoBits))
				return false;
			nSpareBits = 46 - aisImoBits;
			break;
	}

	//get the spare bits that fill against the target ID to bring the total to 46 bits in the two fields
	if (!CALL_UNPACK_BINARY_FIELD(nSpareBits))
		return false;

	//get the latitude in 1/1000 minutes
	if (!CALL_UNPACK_BINARY_FIELD(24))
		return false;

	//get the longitude in 1/1000 minutes
	if (!CALL_UNPACK_BINARY_FIELD(25))
		return false;

	//get the course over ground in integer degrees
	if (!CALL_UNPACK_BINARY_FIELD(9))
		return false;

	//get the timestamp as the seconds field from a UTC time
	if (!CALL_UNPACK_BINARY_FIELD(6))
		return false;

	//get the speed over ground in knots
	if (!CALL_UNPACK_BINARY_FIELD(8))
		return false;

	return true;
}
