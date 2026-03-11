// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// ProjectionSpec.h
/*---------------------------------------------------------------------------*/

class ProjectionSpec;

#ifndef PROJECTIONSPEC_H
#define PROJECTIONSPEC_H

#include <gz4d/gz4d.h>
#include <math.h>
#include "utilities/Addressable.h"
#include <gz4d/geo/Projection.h>

#include "SDV_types.h"

/** 
 *	This class wraps the projection mechanisms of libproj with addressing.
 *
 *	\author Matt Plumlee (mdp)
 *  \date 12/7/2001
 */
/*---------------------------------------------------------------------------*/
class ProjectionSpec : public Addressable
{
  public:
	/*---Constructors/Destructors---*/
      ProjectionSpec(std::string name = "", bool readonly = true);
	virtual ~ProjectionSpec();
	static void initializeProjections();

	/*---Accessors---*/
	const Projection getProjection() {return proj;}
	GLboolean isReadOnly() {return readOnly;}
	ProjType getType();
	int getZone();

	/*---Mutators---*/
	void initialize(_known_types type, f64 cen_lon, f64 cen_lat,
					f64 x0 = 0.0, f64 y0 = 0.0);

	/*---Commands---*/
	GLboolean makeCurrent(GLboolean forceCurrent = GL_FALSE);
	void project(double lon, double lat, double &easting, double &northing);
	void invert(double easting, double northing, double &lon, double &lat);

	/*---Utilities****/
	void formatDecimalDegrees(double decimalD, char *value, int type);
	//void formatDecimalDegrees(double decimalD, char *val, char *value, int type);
	void formatDecimalDegrees(double decimalD, int &val, char *value, int type);
	void formatDecimalDegrees(double decimalD, int &d, int &m, float &s, int &dir, int type);

	double calculateDistance(double lat1, double lon1, double lat2, double lon2);
	double deg2rad(double value) {return value*M_PI/180;};
	double rad2deg(double value) {return value*180/M_PI;};
	
        virtual void getHtmlInfo(std::stringstream &html, short level);

  protected:
	Projection proj;
	GLboolean readOnly;
	static ProjectionSpec *lastCurrent;
};

extern DottedAddress *ProjectionNeighborhood;
extern DottedAddress *UTMProjectionNeighborhood;
extern DottedAddress *MercatorProjectionNeighborhood;
extern DottedAddress *PolarProjectionNeighborhood;
extern ProjectionSpec *UTMzones[61];
extern ProjectionSpec *MercatorDefault;
extern ProjectionSpec *PolarNorth;
extern ProjectionSpec *PolarSouth;

#endif
