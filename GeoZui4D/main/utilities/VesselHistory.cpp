/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
///////////////////////////////////////////////////////////////////////////////
// VesselHistory.cpp

///////////////////////////////////////////////////////////////////////////////
#include "utilities/VesselHistory.h"

#include "utilities/Utilities.h"
/** 
 *	Class for maintaining a history of locations, orientations, and any other
 *  NMEA values that should be saved in the timeline for proper playbacks.
 *	\note The VesselSnapshotNode data type is derived from the PoseSnapshotNode which
 *  is intended to be derived by child classes to facilitate the addition of 
 *	more information. The destructor for this class will attempt to destroy any
 *	data members left in the list as being of type PoseSnapshotNode.
 *
 *	\author briana sullivan (bs)
 *  \date 6/2/2005
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
VesselHistory::VesselHistory(Object * owner)
    :ObjectHistory(owner)
{
    line->drawTube(false);
    lastInserted = NULL;
}

/*---------------------------------------------------------------------------*/
VesselHistory::~VesselHistory()
{
	VesselSnapshotNode *doomed;

	iterator = start;
	while (iterator)
	{
		doomed = (VesselSnapshotNode*) iterator->data;
		if (doomed)
			delete doomed;
		iterator->data = NULL;
		iterator = iterator->next;
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void VesselHistory::insert(double t, float x, float y, float z, OrientationInfo &o)
{
	if (!end || end->time != t)
		lastInserted = new VesselSnapshotNode;
	ObjectHistory::insert(t, lastInserted, x, y, z, o, -1);
}

/*---------------------------------------------------------------------------*/
/*
void VesselHistory::insert(double t, VesselSnapshotNode *info, float x, float y,
						   float z, OrientationInfo &o)
{
	OrientationInfo *io = &info->orientation;
	io->type = o.type;
	io->angle = o.angle;
	io->axis[Yaw] = o.axis[Yaw];
	io->axis[Pitch] = o.axis[Pitch];
	io->axis[Roll] = o.axis[Roll];

	// info should be bringing back the values to lastInserted
	ObjectHistory::insert(t, info, x, y, z);
}
*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
VesselSnapshotNode::VesselSnapshotNode()
{
	rudderOffset[X] = rudderOffset[Y] = rudderOffset[Z] = 0;
	sounderDepth = 0;
	sounderOffset[X] = sounderOffset[Y] = sounderOffset[Z] = 0;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/**	returns the last VesselSnapshotNode inserted into VesselHistory
 */
VesselSnapshotNode* VesselHistory::getLast()
{
	return lastInserted;

}

void VesselSnapshotNode::setRudder(float rudder_angle, Point3f rudder_offset)
{
	if(!rudderOrientation)
	{
		rudderOrientation = std::shared_ptr<OrientationInfo>(new OrientationInfo);
	}
	rudderOrientation->axis[Yaw] = rudder_angle;
	rudderOffset[X] = rudder_offset[X];
	rudderOffset[Y] = rudder_offset[Y];
	rudderOffset[Z] = rudder_offset[Z];
}

void VesselSnapshotNode::setDepthSounder(float sounder_depth, Point3f sounder_offset)
{
	sounderDepth = sounder_depth;
	sounderOffset[X] = sounder_offset[X];
	sounderOffset[Y] = sounder_offset[Y];
	sounderOffset[Z] = sounder_offset[Z];
}

/*---------------------------------------------------------------------------*/
void VesselSnapshotNode::setCourseAndSpeed(float cog, float sog)
{
	courseOverGround = cog;
	speedOverGround = sog;
}

/*---------------------------------------------------------------------------*/
void VesselSnapshotNode::setCrosstrackError(float trackX, float trackY, float trackHeading)
{
	XTEx = trackX;
	XTEy = trackY;
	XTE = (float)sqrt(XTEx * XTEx + XTEy * XTEy);
	headingDiff = trackHeading;
}

/*---------------------------------------------------------------------------*/
bool VesselHistory::getInterpolatedRudderOrientation(double time, OrientationInfo &rudder_orientation)
{
	VesselSnapshotNode *vesselNode2;
	double weight;
	if(getInterpolatedRudderOrientation(time, weight, vesselNode2, rudder_orientation))
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
/**	returns rudder orientation at time t with support information to
 *	make interpolations on other data actually in the derived TimeLineNode
 *	data type.
 */
VesselSnapshotNode* VesselHistory::getInterpolatedRudderOrientation(double time, double &weight, VesselSnapshotNode *&vesselNode2, OrientationInfo &rudder_orientation)
{
    VesselSnapshotNode *vesselNode1;
    TimeLineNode *timeNode;
    std::shared_ptr<OrientationInfo> rudderOrient1, rudderOrient2;
    gz4d::Point<double> point; //not really being used

    vesselNode1 = (VesselSnapshotNode*)getPosition(time, weight, timeNode, point);
    if (!vesselNode1)
            return NULL;
    vesselNode2 = (VesselSnapshotNode*) timeNode;

    rudderOrient1 = vesselNode1->getRudderOrientation();
    rudderOrient2 = vesselNode2->getRudderOrientation();
    rudder_orientation.type = rudderOrient1->type;

    rudder_orientation.angle = (float)interpolateDegrees(rudderOrient1->angle,rudderOrient2->angle,weight);
    rudder_orientation.axis[Yaw] = (float)interpolateDegrees(rudderOrient1->axis[Yaw], rudderOrient2->axis[Yaw], weight); 
    rudder_orientation.axis[Pitch] = (float)interpolateDegrees(rudderOrient1->axis[Pitch], rudderOrient2->axis[Pitch], weight);
    rudder_orientation.axis[Roll] = (float)interpolateDegrees(rudderOrient1->axis[Roll], rudderOrient2->axis[Roll], weight);

    return vesselNode1;
}


std::shared_ptr<OrientationInfo const> VesselHistory::getRudderOrientation()
{
    if(iterator && iterator->data)
        return ((VesselSnapshotNode *)(iterator->data))->getRudderOrientation();
    return std::shared_ptr<OrientationInfo const>();
}


bool VesselHistory::getInterpolatedSounderDepth(double time, double &sounder_depth)
{
	VesselSnapshotNode *vesselNode2;
	double weight;
	if(getInterpolatedSounderDepth(time, weight, vesselNode2, sounder_depth))
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
/**	returns sounder depth at time t with support information to
 *	make interpolations on other data actually in the derived TimeLineNode
 *	data type.
 */
VesselSnapshotNode* VesselHistory::getInterpolatedSounderDepth(double time, double &weight,
																VesselSnapshotNode *&vesselNode2,
																double &sounder_depth)
{
	VesselSnapshotNode *vesselNode1;
	TimeLineNode *timeNode;
	float sounderDepth1, sounderDepth2;
	gz4d::Point<double> point; //not really being used

	vesselNode1 = (VesselSnapshotNode*)getPosition(time, weight, timeNode, point);
	if (!vesselNode1)
		return NULL;
	vesselNode2 = (VesselSnapshotNode*) timeNode;

	sounderDepth1 = vesselNode1->getSounderDepth();
	sounderDepth2 = vesselNode2->getSounderDepth();
	sounder_depth = interpolateValues(sounderDepth1,sounderDepth2,weight);

	return vesselNode1;
}


float VesselHistory::getSounderDepth()
{
	if(iterator && iterator->data)
		return ((VesselSnapshotNode *)(iterator->data))->getSounderDepth();
	//might need to make sounderDepth not protected...if this doesn't work. like orientation.
	return 0.0;
}

/*---------------------------------------------------------------------------*/
bool VesselHistory::getInterpolatedCourseAndSpeed(double time, float &cog, float &sog)
{
	VesselSnapshotNode *vesselNode2;
	double weight;
	if(getInterpolatedCourseAndSpeed(time, weight, vesselNode2, cog, sog))
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
/**	returns course and speed at time t with support information to
 *	make interpolations on other data actually in the derived TimeLineNode
 *	data type.
 */
VesselSnapshotNode* VesselHistory::getInterpolatedCourseAndSpeed(double time, double &weight,
																VesselSnapshotNode *&vesselNode2,
																float &cog, float &sog)
{
	VesselSnapshotNode *vesselNode1;
	TimeLineNode *timeNode;
	float val1, val2;
	gz4d::Point<double> point; //not really being used

	vesselNode1 = (VesselSnapshotNode*)getPosition(time, weight, timeNode, point);
	if (!vesselNode1)
		return NULL;
	vesselNode2 = (VesselSnapshotNode*) timeNode;

	val1 = vesselNode1->getSpeed();
	val2 = vesselNode2->getSpeed();
	sog = (float)interpolateValues(val1, val2, weight);

	val1 = vesselNode1->getCourse();
	val2 = vesselNode2->getCourse();
	cog = (float)interpolateDegrees(val1, val2, weight);

//	cerr << "c1: " << val1 << ",\tc2: " << val2 << ",\tcog: " << cog << endl;

	return vesselNode1;
}

/*---------------------------------------------------------------------------*/
float VesselHistory::getCourse()
{
	if(iterator && iterator->data)
		return ((VesselSnapshotNode *)(iterator->data))->getCourse();
	return 0.0;
}

/*---------------------------------------------------------------------------*/
float VesselHistory::getSpeed()
{
	if(iterator && iterator->data)
		return ((VesselSnapshotNode *)(iterator->data))->getSpeed();
	return 0.0;
}

/*---------------------------------------------------------------------------*/
bool VesselHistory::getInterpolatedCrosstrackErrorXY(double time, float &x, float &y)
{
	VesselSnapshotNode *vesselNode2;
	double weight;
	if(getInterpolatedCrosstrackErrorXY(time, weight, vesselNode2, x, y))
		return true;
	return false;
}

/*---------------------------------------------------------------------------*/
/**	returns x,y crostrack error locations at time t with support information to
 *	make interpolations on other data actually in the derived TimeLineNode
 *	data type.
 */
VesselSnapshotNode* VesselHistory::getInterpolatedCrosstrackErrorXY(double time, double &weight,
																VesselSnapshotNode *&vesselNode2,
																float &x, float &y)
{
	VesselSnapshotNode *vesselNode1;
	TimeLineNode *timeNode;
	float val1, val2;
        gz4d::Point<double> point; //not really being used

	vesselNode1 = (VesselSnapshotNode*)getPosition(time, weight, timeNode, point);
	if (!vesselNode1)
		return NULL;
	vesselNode2 = (VesselSnapshotNode*) timeNode;

	val1 = vesselNode1->getCrosstrackErrorX();
	val2 = vesselNode2->getCrosstrackErrorX();
	x = (float)interpolateValues(val1, val2, weight);

	val1 = vesselNode1->getCrosstrackErrorY();
	val2 = vesselNode2->getCrosstrackErrorY();
	y = (float)interpolateValues(val1, val2, weight);

//	cerr << "c1: " << val1 << ",\tc2: " << val2 << ",\tcog: " << cog << endl;

	return vesselNode1;
}

/*---------------------------------------------------------------------------*/
float VesselHistory::getCrosstrackError()
{
	if(iterator && iterator->data)
		return ((VesselSnapshotNode *)(iterator->data))->getCrosstrackError();
	return 0.0;
}

/*---------------------------------------------------------------------------*/
float VesselHistory::getCrosstrackErrorX()
{
	if(iterator && iterator->data)
		return ((VesselSnapshotNode *)(iterator->data))->getCrosstrackErrorX();
	return 0.0;
}

/*---------------------------------------------------------------------------*/
float VesselHistory::getCrosstrackErrorY()
{
	if(iterator && iterator->data)
		return ((VesselSnapshotNode *)(iterator->data))->getCrosstrackErrorY();
	return 0.0;
}

/*---------------------------------------------------------------------------*/
void VesselSnapshotNode::writeData(double time, FILE *logfile)
{

	fprintf(logfile,"%13.2f %7.3f %7.3f %8.3f %8.3f %8.3f %10.3f %10.3f %10.3f %7.2f %7.2f\n",
		time,courseOverGround,speedOverGround,XTE,XTEx,XTEy,
		lineNode->position[X],lineNode->position[Y],lineNode->position[Z],rudderOrientation->axis[Yaw], headingDiff);
	fflush(logfile);
}

/*---------------------------------------------------------------------------*/
void VesselSnapshotNode::writeDataHeader(FILE *logfile)
{

	fprintf(logfile,"time          cog       sog   XTE      XTEx     XTEy       X          Y          Z        Yaw     dHeading\n");

}
