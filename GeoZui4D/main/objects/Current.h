// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef CURRENT_H
#define CURRENT_H

#include "engine/ObjectCollection.h"
#include<gz4d/netcdf.h>

class TimeVaryingObject;


class Current : public ObjectCollection
{
public: 
	Current();
	~Current();
	DERIVE_OBJECT(Current)

	virtual void Idle(double time);
	Object * LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile);
	float getDepth(int r,int c);
	float getTide(int r, int c, int t);
	float getScale() {return scale;};
	float getSigma(int s) {return sigmas[s];};
	float getVSurfaceHeight(double x, double y, double t);
	float getVSurfaceHeight(double x, double y);
	int getSigmaLevels() {return sigmalevels;};

	float getCurrentDensity() {return currentDensity;}
	void setCurrentDensity(float);

	enum LengthType {As_Sqrt, As_Linear};
	int getCurrentLengthType() {return currentLengthType;}
	void setCurrentLengthType(int t);

	GLboolean getShowLabels() {return showLabels;}

	virtual int perform(int argc, const char **argv, Tcl_Interp *interp);
	virtual int list(Tcl_Interp *interp);

private:

	double currentGzTime;
	double * times;
	float * sigmas;
	float scale;
	int timesteps;
	int sigmalevels;
	GLboolean showLabels;
	
	int currentTimestep;

	float cellsize;
	int rows,cols;

	float ***tides; // x,y time.
	float **bathy;

	TimeVaryingObject * vsurface;
        std::string vsurfacelabel;

	float currentDensity;
	int currentLengthType;
};

#endif

