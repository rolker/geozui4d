/* Copyright (c) 2001 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// ProjectionSpec.cpp
/*---------------------------------------------------------------------------*/
/** 
 *	This class wraps the projection mechanisms of libproj with addressing.
 *
 *	Created by mdp 12/7/2001
 */
/*---------------------------------------------------------------------------*/
#include "widgets/Sv_Standards.h"
#include "utilities/Addressable.h"
#include "utilities/DottedAddress.h"
#include "engine/ProjectionSpec.h"
#include <iostream>

using namespace std;

Addressable ProjectionNeighborhoodAnchor;
Addressable UTMProjectionNeighborhoodAnchor;
Addressable MercatorProjectionNeighborhoodAnchor;
Addressable PolarProjectionNeighborhoodAnchor;
DottedAddress *ProjectionNeighborhood = new DottedAddress(&ProjectionNeighborhoodAnchor, "PROJECTION");
DottedAddress *UTMProjectionNeighborhood = new DottedAddress(&UTMProjectionNeighborhoodAnchor, "UTM");
DottedAddress *MercatorProjectionNeighborhood = new DottedAddress(&MercatorProjectionNeighborhoodAnchor, "MERCATOR");
DottedAddress *PolarProjectionNeighborhood = new DottedAddress(&PolarProjectionNeighborhoodAnchor, "POLAR");
ProjectionSpec *UTMzones[61] = {NULL};
ProjectionSpec *MercatorDefault = NULL;
ProjectionSpec *PolarNorth = NULL;
ProjectionSpec *PolarSouth = NULL;
ProjectionSpec *ProjectionSpec::lastCurrent = NULL;
ProjectionSpec *IdentityProjection = NULL;

/// Typedef for the projection function as defined in gctp.c
typedef long (*ProjFunc)(f64, f64, f64p, f64p);

/// Paramaters used for projections
typedef struct _proj_params {
	f64 cen_lon;				///< Center lat for projection area of interest
	f64 cen_lat;				///< Center lat for projection area of interest
	f64 x0;						///< X Offset (Mercator and Polar Stereographic) 
	f64 y0;						///< Y Offset (Mercator and Polar Stereographic) 
} ProjParam;

/// A projection with its transformation functions and parameters.
typedef struct _projection {
	ProjType	type;			///< The type of projection.
	ProjFunc	forward;		///< Function that projects a geographic coordinate to the current projection
	ProjFunc	inverse;		///< Function that reverses a projected point to obtain geographic coordinates
	ProjParam	params;	///< Parameters of this projection
} ProjStruct;

/*-------------------------Constructors/Destructors--------------------------*/
/*---------------------------------------------------------------------------*/
ProjectionSpec::ProjectionSpec(string name, bool readonly)
{
	if (!name.empty())
		publishName(name);

	proj = new struct _projection;
	readOnly = readonly;
}

/*---------------------------------------------------------------------------*/
ProjectionSpec::~ProjectionSpec()
{
	delete proj;
}

/*---------------------------------------------------------------------------*/
void ProjectionSpec::initialize(_known_types type, f64 cen_lon, f64 cen_lat,
								f64 x0, f64 y0)
{
	switch (type)
	{
		case PROJECTION_UTM:
			projection_init_utm(proj, cen_lon, cen_lat);
			break;
		case PROJECTION_MERCATOR:
			projection_init_mercator(proj, cen_lon, cen_lat, x0, y0);
			break;
		case PROJECTION_POLAR_STEREO:
			projection_init_polar_stereo(proj, cen_lon, cen_lat, x0, y0);
			break;
		default:
			proj->type = type;
			proj->params.cen_lon = cen_lon;
			proj->params.cen_lat = cen_lat;
			proj->params.x0 = x0;
			proj->params.y0 = y0;
			break;
	}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void ProjectionSpec::initializeProjections()
{
	int zone;
	ProjectionSpec *newProj;
	char *name;

	if (MercatorDefault != NULL)
		return;

	IdentityProjection = new ProjectionSpec("IDENTITY");
	IdentityProjection->initialize(PROJECTION_NONE, 0.0, 0.0);
	ProjectionNeighborhood->addChild(IdentityProjection->getAddress());

	ProjectionNeighborhood->addChild(UTMProjectionNeighborhood);
	ProjectionNeighborhood->addChild(MercatorProjectionNeighborhood);
	ProjectionNeighborhood->addChild(PolarProjectionNeighborhood);
	for (zone = 1; zone <= 60; zone++)
	{
		if (zone < 10)
			name = new char[6];
		else
			name = new char[7];
		sprintf(name, "ZONE%d", zone);
		newProj = new ProjectionSpec(name, GL_FALSE);
		newProj->initialize(PROJECTION_UTM, -183.0 + zone * 6, 0.0);
		UTMProjectionNeighborhood->addChild(newProj->getAddress());
		UTMzones[zone] = newProj;
	}

	MercatorDefault = new ProjectionSpec("DEFAULT");
	MercatorDefault->initialize(PROJECTION_MERCATOR, 0.0, 0.0);
	MercatorProjectionNeighborhood->addChild(MercatorDefault->getAddress());

	PolarNorth = new ProjectionSpec("NORTH");
	PolarNorth->initialize(PROJECTION_POLAR_STEREO, 0.0, 90.0);
	PolarProjectionNeighborhood->addChild(PolarNorth->getAddress());

	PolarSouth = new ProjectionSpec("SOUTH");
	PolarSouth->initialize(PROJECTION_POLAR_STEREO, 0.0, -90.0);
	PolarProjectionNeighborhood->addChild(PolarSouth->getAddress());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
ProjType ProjectionSpec::getType()
{
	return (proj ? proj->type : PROJECTION_UNKNOWN);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
GLboolean ProjectionSpec::makeCurrent(GLboolean forceCurrent)
{
	if (this != lastCurrent || forceCurrent)
	{
		lastCurrent = this;
		if (proj->type)
			return projection_make_current(proj);
		else
			return GL_TRUE;
	}
	return GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void ProjectionSpec::project(double lon, double lat,
							 double &easting, double &northing)
{
	if (proj->type)
	{
		makeCurrent();
		projection_ll_to_en_deg(proj, lon, lat, &easting, &northing);
	}
	else
	{
		easting = lon;
		northing = lat;
	}
}

/*---------------------------------------------------------------------------*/
void ProjectionSpec::invert(double easting, double northing,
							double &lon, double &lat)
{
	if (proj->type)
		projection_en_to_ll_deg(proj, easting, northing, &lon, &lat);
	else
	{
		lon = easting;
		lat = northing;
	}
}

/*---------------------------------------------------------------------------
 * This one returns the decimal degree in one formatted string value. -bs 4/1/05
 *---------------------------------------------------------------------------*/
void ProjectionSpec::formatDecimalDegrees(double decimalD, char* value, int type)
{
	int degree, min;
	float sec;

	degree = (int)decimalD;
	sec = (float) (decimalD - degree);
	if(sec < 0)
		sec = -sec;
	min = (int)(sec * 60);
	sec = sec * 60;
	sec = (sec - min) * 60;

    //insert stuff and second bit to type added by mdp 4/30/2008 to make
    //it so quotes could be escaped for use in Tcl commands
    char const *insert;
    if (type & 2)
        insert = "\\";
    else
        insert = "";
	
	if(type & 1)
	{
		//latitude
		if(abs(degree) <= 90)
		{
			if(degree > 0)
				sprintf(value, "%02d`%02d'%05.2f%s\"N", degree,min,sec, insert);
			else
			{
				degree = degree*(-1);
				sprintf(value, "%02d`%02d'%05.2f%s\"S", degree,min,sec, insert);
			}
		}
		else
			sprintf(value, "%02d`%02d'%02d.00%s\"N", 0,0,0, insert);
	}
	else
	{
		//longitude
		if(abs(degree) <= 180)
		{
			if(degree > 0)
				sprintf(value, "%03d`%02d'%05.2f%s\"E", degree,min,sec, insert);
			else
			{
				degree = degree*(-1);
				sprintf(value, "%03d`%02d'%05.2f%s\"W", degree,min,sec, insert);
			}
		}
		else
			sprintf(value, "%03d`%02d'%02d.00%s\"W", 0,0,0, insert);
	}
}

/*------------------------------------------------------------------------------------------------
 * This one returns the decimal degree in two formatted values one int and the rest as a string. - bs 4/21/05
 *------------------------------------------------------------------------------------------------*/
void ProjectionSpec::formatDecimalDegrees(double decimalD, int &val, char *value, int type)
{
	int degree, min;
	float sec;

	degree = val = (int)decimalD;
	sec = (float) (decimalD - degree);
	if(sec < 0)
		sec = sec*(-1);
	min = (int)(sec * 60);
	sec = sec * 60;
	sec = (sec - min) * 60;
	
    //insert stuff and second bit to type added by mdp 4/30/2008 to make
    //it so quotes could be escaped for use in Tcl commands
    char const *insert;
    if (type & 2)
        insert = "\\";
    else
        insert = "";

    if(type & 1)
	{
		//latitude
		if(abs(degree) <= 90)
		{
			if(degree > 0)
				sprintf(value,"%02d'%05.2f%s\"N", min,sec, insert);
			else
			{
				degree = val = degree*(-1);
				sprintf(value,"%02d'%05.2f%s\"S", min,sec, insert);
			}
			
		}
		else
		{
			sprintf(value,"%02d'%02d.00%s\"N",0,0, insert);
			degree = val = 0;
		}
	}
	else
	{
		//longitude
		if(abs(degree) <= 180)
		{
			if(degree > 0)
				sprintf(value,"%02d'%05.2f%s\"E",min,sec, insert);
			else
			{
				degree = val = degree*(-1);
				sprintf(value,"%02d'%05.2f%s\"W", min,sec, insert);
			}
		}
		else
		{
			sprintf(value,"%02d'%02d.00%s\"W",0,0, insert);
			degree = val = 0;
		}
	}
	//sprintf(val,"%02d",degree);
}

/*------------------------------------------------------------------------------------------------
 * This one returns the decimal degree in separated in degree, min, and sec numerical values - bs 4/21/05
 *------------------------------------------------------------------------------------------------*/
void ProjectionSpec::formatDecimalDegrees(double decimalD, int &d, int &m, float &s, int &dir, int type)
{
	int degree, min;
	float sec;
//cout<<"decimalD = "<<decimalD<<endl;
	degree = d = (int)decimalD;
	sec = (float) (decimalD - degree);
	if(sec < 0)
		sec = -sec;
	min = m =(int)(sec * 60);
	sec = sec * 60;
	sec = s = (sec - min) * 60;
	
    if(type & 1)
	{
		//latitude
		if(abs(degree) <= 90)
		{
			if(degree > 0)
				dir = 'N';
			else
			{
				dir = 'S';
				degree = d = degree*(-1);
				min = m = min*(-1);
				sec = s = sec*(-1);
			}
		}
		else
		{
			degree = d = 0;
			min = m = 0;
			sec = s = 0;
			dir = 'N';
		}
	}
	else
	{
		//longitude
		if(abs(degree) <= 180)
		{
			if(degree > 0)
				dir = 'E';
			else
			{
				degree = d = degree*(-1);
				min = m = min*(-1);
				sec = s = sec*(-1);
				dir = 'W';
			}
		}
		else
		{
			degree = d = 0;
			min = m = 0;
			sec = s = 0;
			dir = 'W';
		}
	}
}

/*------------------------------------------------------------------------------------------------
 * Calculates the decimal degree of two locations - bs 4/21
 *------------------------------------------------------------------------------------------------*/
double ProjectionSpec::calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
	double theta, distance, miles;

	theta = lon1 - lon2;
	distance = sin((deg2rad(lat1))) * sin((deg2rad(lat2))) +
			   cos((deg2rad(lat1))) * cos((deg2rad(theta)));
	distance = acos(distance);
	distance = rad2deg(distance);
	miles = distance * 60 * 1.1515;
	
	//return nautical miles
	//return miles*.08684;

	//return kilometers
	//return miles*1.609344;

	//return meters
	return miles*1609.344;

	//return feet
	//return miles*5280;
}

// If we have a UTM projection, return its zone, else return 0. rja Oct 25, 2005
int ProjectionSpec::getZone()
{
	if(proj && proj->type == PROJECTION_UTM)
		return longToZone(proj->params.cen_lon);
	return 0;
}

void ProjectionSpec::getHtmlInfo(stringstream &html, short level)
{
	html << "<h" << level << ">ProjectionSpec:</h" << level << ">";
	html << "Projection type: ";
	switch(proj->type)
	{
	case PROJECTION_UTM:
		html << "UTM<br>";
		html << "Zone: "<< longToZone(proj->params.cen_lon) << "<br>";
		html << "Center longitude: " << proj->params.cen_lon << "<br>";
		
		break;
	case PROJECTION_NONE:
		html << "None<br>";
		break;
	case PROJECTION_MERCATOR:
		html << "Mercator<br>";
		html << "Center longitude,latitude: " << proj->params.cen_lon << "," << proj->params.cen_lat << "<br>";
		html << "Offset x0,y0: " << proj->params.x0 << "," << proj->params.y0 << "<br>";
		break;
	case PROJECTION_POLAR_STEREO:
		html << "Polar Stereo<br>";
		html << "Center longitude,latitude: " << proj->params.cen_lon << "," << proj->params.cen_lat << "<br>";
		html << "Offset x0,y0: " << proj->params.x0 << "," << proj->params.y0 << "<br>";
		break;
	case PROJECTION_UNKNOWN:
		html << "Unknown<br>";
		break;
	default:
		break;
	}
}
