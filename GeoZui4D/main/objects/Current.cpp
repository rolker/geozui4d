/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
#include "GZincludes.h"
#include "Current.h"
#include "CurrentLayer.h"
#include "TimeVaryingObject.h"
#include "objects/gutm.h"
#include <gz4d/util/Utilities.h>

using namespace std;

REGISTER_OBJECT(Current, "current")

extern MainWindow * mainWindow;
extern int dominantEye;

/*---------------------------------------------------------------------------*/
Current::Current()
{
	currentTimestep = 0;
	scale = 1.0;
	currentDensity = 1.5;
	currentLengthType = As_Linear;
	showLabels = GL_TRUE;
}

/*---------------------------------------------------------------------------*/
Current::~Current()
{
}

/*---------------------------------------------------------------------------*/
Object * Current::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    int i,j;
    CurrentLayer * cl;
    Point3d gr;

    scale = xml.get("<xmlattr>.scale",scale);

    SetLabel("Current");

    timesteps = ncfile.GetDim("time").Size();

    sigmalevels = 0;
    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.hassigma"))
        if(gz4d::IsTrue(*attribute))
            sigmalevels = ncfile.GetDim("sigma").Size();

    times = new double[timesteps];

    ncfile.GetVar("time").Get(times,timesteps);
	
    if(sigmalevels)
    {
        sigmas = new float[sigmalevels];
        ncfile.GetVar("sigma").Get(sigmas,sigmalevels);
    }

    tides = NULL;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.hastides"))
        if(gz4d::IsTrue(*attribute))
            tides = new float**[timesteps];

    gr[0] = ncfile.GetAtt("localx").As<double>(0);
    gr[1] = ncfile.GetAtt("localy").As<double>(0);
    gr[2] = ncfile.GetAtt("localz").As<double>(0);
    SetPosition(gr);

    cellsize = ncfile.GetAtt("cellsize").As<float>(0);

    cols = ncfile.GetDim("col").Size();
    rows = ncfile.GetDim("row").Size();

    if(tides)
    {
        for(i=0;i<timesteps;i++)
        {
            tides[i] = new float*[rows];
            for(j=0;j<rows;j++)
            {
                tides[i][j] = new float[cols];
                ncfile.GetVar("tides").SetCur(i,j,0);
                ncfile.GetVar("tides").Get(tides[i][j],1,1,cols);
            }
        }
    }


    bathy = NULL;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.surface"))
    {
        bathy = new float*[rows];
        for(j=0;j<rows;j++)
        {
            bathy[j] = new float[cols];
            ncfile.GetVar(*attribute).SetCur(j,0);
            ncfile.GetVar(*attribute).Get(bathy[j],1,cols);
        }
    }

    vsurface = NULL; // variable surface - TVGutm
    vsurfacelabel = xml.get("<xmlattr>.vsurface",vsurfacelabel);

    if(!localBBox)
        localBBox = new BBox;

    localBBox->start = times[0];
    localBBox->end = times[timesteps-1];

    localBBox->w = 0;
    localBBox->e = cols*cellsize;
    localBBox->s = 0;
    localBBox->n = rows*cellsize;
    if(parent)
        parent->setBBoxStale();

    for(bpt::ptree::const_iterator n = xml.begin(); n != xml.end(); ++n)
        if(n->first == "Layer")
        {
            cl = new CurrentLayer(this);
            if(cl->LoadXML(n->second,path,ncfile))
            {
                AddObject(cl);
            }
            else
            {
                delete cl;
            }
        }

    return this;
}

/*---------------------------------------------------------------------------*/
void Current::Idle(double time)
{
	int i;
	int oldCurrent;
	oldCurrent = currentTimestep;
	
	currentTimestep = 0;
	for(i=0;i<timesteps;i++)
	{
		if (times[i] <= time)
		currentTimestep = i;
	}
	currentGzTime = time;

	if(!vsurfacelabel.empty())
	{
		vsurface = (TimeVaryingObject *) mainWindow->getScene3D()->GetObjectPtr(vsurfacelabel);
		vsurfacelabel = "";
	}

	ObjectCollection::Idle(time);

	if(currentTimestep != oldCurrent)
	{
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
}

/*---------------------------------------------------------------------------*/
float Current::getDepth(int r, int c)
{
	return (float)(GetPosition(2)+bathy[r][c]);
}

/*---------------------------------------------------------------------------*/
float Current::getTide(int r, int c, int t)
{
 	return (float)(GetPosition(2)+tides[t][r][c]);
}

/*---------------------------------------------------------------------------*/
float Current::getVSurfaceHeight(double x, double y, double t)
{
	if(vsurface)
	{
		return (float)((Gutm *) vsurface->GetCurrent(t))->GetValue(x,y);
	}
	else
	{
		return 0.0;
	}
}

/*---------------------------------------------------------------------------*/
float Current::getVSurfaceHeight(double x, double y)
{
	if(vsurface)
	{
		return (float)((Gutm *) vsurface->GetCurrent())->GetValue(x,y);
	}
	else
	{
		return 0.0;
	}
}

/*---------------------------------------------------------------------------*/
void Current::setCurrentDensity(float d)
{
	if (d < 0.00001)
		return;

	currentDensity = d;
	globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
void Current::setCurrentLengthType(int t)
{
	if (currentLengthType == t)
		return;

	currentLengthType = t;
	globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/*---------------------------------------------------------------------------*/
int Current::perform(int argc, const char * * argv, Tcl_Interp * interp)
{
	char retval[64];
	
	if (argc >= 1)
	{
		if(strcmp(argv[0],"currentDensity")==0)
		{
			if (argc >= 2)
				setCurrentDensity((float)atof(argv[1]));

			sprintf(retval, "%f", currentDensity);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if (strcmp(argv[0], "showLabels")==0)
		{
			if (argc >= 2)
				showLabels = atoi(argv[1]);

			sprintf(retval, "%i", showLabels);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
		if(strcmp(argv[0],"lengthType")==0)
		{
			if (argc >= 2)
				setCurrentLengthType(atoi(argv[1]));

			sprintf(retval, "%i", currentLengthType);
			Tcl_SetResult(interp, retval, TCL_VOLATILE);
			return TCL_OK;
		}
	}
	return ObjectCollection::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Current::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "currentDensity lengthType showLabels ", NULL);
	return ObjectCollection::list(interp);
}
