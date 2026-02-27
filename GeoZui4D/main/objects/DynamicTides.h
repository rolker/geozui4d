/* Copyright (c) 2004 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// DynamicTides.h
/*---------------------------------------------------------------------------*/
/** 
 *	This class dynamically calculates (and can display) tide information
 *	based on a static grid of time-offsets and multiplication factors, along
 *	with an array of predicted and/or measured tides from a particular tide
 *	station.
 *
 *	Created by mdp 7/9/2004
 */
/*---------------------------------------------------------------------------*/
#ifndef DYNAMICTIDES_H
#define DYNAMICTIDES_H

#include "engine/Object.h"
#include <boost/filesystem.hpp>

class DynamicTides: public Object
{
public:
//	static const double NaN;

	DERIVE_OBJECT(DynamicTides);
	
	DynamicTides();
	~DynamicTides();
	Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
	virtual void Idle(double time);
	void DrawLowRes(float degradation, Window3D * win);
	void Fuse(Object *other){}; //I'm not sure if we really want to do this, but ok for newly created class...
	float getTideOffset(double x, double y, double t = std::numeric_limits<double>::quiet_NaN());
	float getTideOffset(int xIndex, int yIndex, double t = std::numeric_limits<double>::quiet_NaN());
	float getTideOffsetGiven(float offset, float multiplier, double t = std::numeric_limits<double>::quiet_NaN());
	void getTimeOffsetAndMultiplier(double x, double y, float &offset, float &multiplier);
	void getTimeOffsetAndMultiplier(int xIndex, int yIndex, float &offset, float &multiplier);

	/*---Scripting functions---*/
	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

protected:
	int tableWidth, tableHeight, numRawReadings, numPredictedValues;
	int daysAhead, daysBack;
	float cellSpacing;
	float *timeOffset, *amplitudeMultiplier;
	float *rawReadings, *predictedValues;
	double rawStartTime, rawInterval, rawEndTime;
	double predictedStartTime, predictedInterval, predictedEndTime;

	bfs::path rawFilename, predictedFilename;

	double currentGzTime;
	int currentTimeIndex, currentTimeSerial;
	double timeDistance(BBox b,double t);
	void updateBBox();

	void loadOffsetFrom(std::string const &fname);
	void loadMultiplierFrom(std::string const &fname);
	void loadRawReadingsFrom(std::string const &fname);
	void loadPredictedReadingsFrom(std::string const &fname);

	static GLboolean tclInitialized;
};

#endif
