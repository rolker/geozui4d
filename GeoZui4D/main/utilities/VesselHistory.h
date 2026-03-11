// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
///////////////////////////////////////////////////////////////////////////////
// VesselHistory.h
///////////////////////////////////////////////////////////////////////////////
#ifndef	VESSELHISTORY_H
#define VESSELHISTORY_H
class VesselHistory;

#include "utilities/ObjectHistory.h"

class VesselSnapshotNode: public PoseSnapshotNode
{
	public:
		VesselSnapshotNode();
		virtual ~VesselSnapshotNode(){}
		//Rudder related functions
		void setRudder(float rudder_angle, Point3f rudder_offset);
		//bool getInterpolatedRudderOrientation(double time, OrientationInfo &rudder_orientation);
		virtual std::shared_ptr<OrientationInfo> &getRudderOrientation(){return rudderOrientation;};

		//Depth sounder related function
		void setDepthSounder(float sounder_depth, Point3f sounder_offset);
		virtual float getSounderDepth(){return sounderDepth;};
		//bool getInterpolatedDepthSounderInfo(double time, float &sounder_value);
		//bool getDepthSounderInfo(double time, float &sounder_value);

		//Course and speed related functions
		void setCourseAndSpeed(float course, float speed);
		virtual float getCourse() {return courseOverGround;}
		virtual float getSpeed() {return speedOverGround;}

		//Cross track error-related functions
		void setCrosstrackError(float trackX, float trackY, float trackHeading);
		float getCrosstrackErrorX() {return XTEx;}
		float getCrosstrackErrorY() {return XTEy;}
		float getCrosstrackError() {return XTE;}

		//Write vessel data to file
		static void writeDataHeader(FILE *logfile);
		void writeData(double time, FILE *logfile);

	protected:
		//comes from RSA (rudder sensor angle)
		std::shared_ptr<OrientationInfo> rudderOrientation;
		Point3f rudderOffset;
		
		//comes from 
		float sounderDepth;
		Point3f sounderOffset;
		
		float courseOverGround;
		float speedOverGround;

		//XTEx and XTEy come from calculations outside this class, XTE calculated against position.
		float XTEx, XTEy, XTE, headingDiff;
};

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
class VesselHistory: public ObjectHistory
{
	public:
            VesselHistory(Object * owner);
		~VesselHistory();
	
		//need an insert function here that adds the needed info into the timeline
		virtual void insert(double t, float x, float y, float z, OrientationInfo &o);
		VesselSnapshotNode* getLast(); //returns the last VesselSnapshotNode inserted
		
		//Rudder related functions
		bool getInterpolatedRudderOrientation(double time, OrientationInfo &rudder_orientation);
		VesselSnapshotNode* getInterpolatedRudderOrientation(double time, double &weight,VesselSnapshotNode *&vesselNode2,OrientationInfo &rudder_orientation);
                std::shared_ptr<OrientationInfo const> getRudderOrientation();

		//Depth sounder related function
		bool getInterpolatedSounderDepth(double time, double &sounder_value);
		VesselSnapshotNode* getInterpolatedSounderDepth(double time, double &weight,VesselSnapshotNode *&vesselNode2,double &sounder_depth);
		float getSounderDepth();

		//Course and speed related functions
		bool getInterpolatedCourseAndSpeed(double time, float &cog, float &sog);
		VesselSnapshotNode* getInterpolatedCourseAndSpeed(double time, double &weight,VesselSnapshotNode *&vesselNode2,float &cog, float &sog);
		float getCourse();
		float getSpeed();

		//Cross-track error related functions
		bool getInterpolatedCrosstrackErrorXY(double time, float &x, float &y);
		VesselSnapshotNode* getInterpolatedCrosstrackErrorXY(double time, double &weight,VesselSnapshotNode *&vesselNode2,float &x, float &y);
		float getCrosstrackError();
		float getCrosstrackErrorX();
		float getCrosstrackErrorY();

	protected:
		//virtual void insert(double t, VesselSnapshotNode *info, float x, float y, float z, OrientationInfo &o);
		VesselSnapshotNode *lastInserted;
};

#endif
