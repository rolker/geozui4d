// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// DynamicTides.cpp
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

#include "GZincludes.h"
#include "objects/DynamicTides.h"
#include <string.h>
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include "engine/PosableStateBase.h"
#include "utilities/DottedAddress.h"
#include "engine/ObjectCollection.h"


extern MainWindow *mainWindow;

using namespace std;

REGISTER_OBJECT(DynamicTides, "");

//const double DynamicTides::NaN = log(0.0);
GLboolean DynamicTides::tclInitialized = GL_FALSE;

/*---------------------------------------------------------------------------*/
DynamicTides::DynamicTides()
{
	currentGzTime = 0.0;
	cellSpacing = 0.0;
	tableWidth = tableHeight = numRawReadings = numPredictedValues = 0;
	timeOffset = amplitudeMultiplier = NULL;
	rawReadings = predictedValues = NULL;
	visible = GL_FALSE;
	showAsWireframe = true;
	daysAhead = daysBack = 2;

	if (!tclInitialized)
	{
		mainWindow->Tcl_Eval("global appdir; source [file join $appdir \"scripts/object/DynamicTides.tcl\"]");
	}
}

/*---------------------------------------------------------------------------*/
DynamicTides::~DynamicTides()
{
	if (timeOffset)
		delete []timeOffset;
	if (amplitudeMultiplier)
		delete []amplitudeMultiplier;
	if (rawReadings)
		delete []rawReadings;
	if (predictedValues)
		delete []predictedValues;
}

/*---------------------------------------------------------------------------*/
Object * DynamicTides::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    fs::path fspath(path);
    char filename[256];

    Object::LoadXML(xml,path, ncfile);

    tableWidth = xml.get("<xmlattr>.width",tableWidth);
    tableHeight = xml.get("<xmlattr>.height",tableHeight);
    cellSpacing = xml.get("<xmlattr>.spacing",cellSpacing);

    if (tableWidth <= 0.0 || tableHeight <= 0.0 || cellSpacing <= 0.0)
    {
        cerr << "Invalid or missing width, hieght, or spacing field in DynamicTides specification" << endl;
        delete this;
        return NULL;
    }
    timeOffset = new float[tableHeight * tableWidth];
    amplitudeMultiplier = new float[tableHeight * tableWidth];

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.offset_file"))
    {
        loadOffsetFrom((fspath/(*attribute)).string());
    }
    else
    {
        cerr << "No offset_file given!" << endl;
        delete this;
        return NULL;
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.multiplier_file"))
    {
        loadMultiplierFrom((fspath/(*attribute)).string());
    }
    else
    {
        cerr << "No multiplier_file given!" << endl;
        delete this;
        return NULL;
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.raw_file"))
    {
        loadRawReadingsFrom((fspath/(*attribute)).string());
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.predicted_file"))
    {
        loadPredictedReadingsFrom((fspath/(*attribute)).string());
    }

    updateBBox();

	//create new filenames for later use in updates
    sprintf(filename, "auto_raw_%s.txt", getAddress()->getName().c_str());
    rawFilename = fspath/filename;
    sprintf(filename, "auto_predicted_%s.txt", getAddress()->getName().c_str());
    predictedFilename = fspath/filename;

    return this;
}

/*---------------------------------------------------------------------------*/
void DynamicTides::loadOffsetFrom(std::string const &fname)
{
	int i, j;
	FILE *file;
	float minutes;

	file = fopen(fname.c_str(), "r");
	if (!file)
	{
		cerr << "Could not open " << fname << endl;
		return;
	}
	for (i = tableHeight - 1; i >= 0; i--)
	{
		for (j = 0; j < tableWidth; j++)
		{
			if(fscanf(file, "%f", &minutes)==1)
			 timeOffset[i * tableWidth + j] = minutes * 60;
		}
	}
	fclose(file);
}

/*---------------------------------------------------------------------------*/
void DynamicTides::loadMultiplierFrom(std::string const &fname)
{
	int i, j;
	FILE *file;

	file = fopen(fname.c_str(), "r");
	if (!file)
	{
		cerr << "Could not open " << fname << endl;
		return;
	}
	for (i = tableHeight - 1; i >= 0; i--)
	{
            for (j = 0; j < tableWidth && fscanf(file, "%f", &amplitudeMultiplier[i * tableWidth + j]) == 1; j++);
	}
	fclose(file);
}

/*---------------------------------------------------------------------------*/
void DynamicTides::loadRawReadingsFrom(std::string const &fname)
{
	int i;
	FILE *file;

	file = fopen(fname.c_str(), "r");
	if (!file)
	{
		cerr << "Could not open " << fname << endl;
		return;
	}
	if(fscanf(file, "%d\n", &numRawReadings) == 1)
        {
	if (rawReadings)
		delete rawReadings;
	rawReadings = new float[numRawReadings];
	if(fscanf(file, "%lf", &rawStartTime)!=1)
            rawStartTime = 0;
	if(fscanf(file, "%lf", &rawInterval)!=1)
            rawInterval = 0;
	for (i = 0; i < numRawReadings; i++)
	{
		if (feof(file))
			break;
		if(fscanf(file, "%f\n", &rawReadings[i])!= 1);
                    break;
	}
        }
	fclose(file);
	numRawReadings = i;
	rawEndTime = rawStartTime + rawInterval * numRawReadings;
}

/*---------------------------------------------------------------------------*/
void DynamicTides::loadPredictedReadingsFrom(std::string const &fname)
{
	int i=0;
	FILE *file;

	file = fopen(fname.c_str(), "r");
	if (!file)
	{
		cerr << "Could not open " << fname << endl;
		return;
	}
	if(fscanf(file, "%d", &numPredictedValues)==1)
        {
	if (predictedValues)
		delete predictedValues;
	predictedValues = new float[numPredictedValues];
	if(fscanf(file, "%lf", &predictedStartTime)!=1)
            predictedStartTime = 0;
	if(fscanf(file, "%lf", &predictedInterval)!=1)
            predictedInterval = 0;
	for (i = 0; i < numPredictedValues; i++)
	{
		if (feof(file))
			break;
		if(fscanf(file, "%f", &predictedValues[i])!=1)
                    break;
	}
        }
	fclose(file);
	numPredictedValues = i;
	predictedEndTime = predictedStartTime + predictedInterval * numPredictedValues;
}

/*---------------------------------------------------------------------------*/
float DynamicTides::getTideOffset(double x, double y, double t)
{
	int i, j;

        gz4d::Point<double> pos = pose->getPosition();
	j = int((x - pos[X]) / cellSpacing);
	i = int((y - pos[Y]) / cellSpacing);
	return getTideOffset(j, i, t);
}

/*---------------------------------------------------------------------------*/
float DynamicTides::getTideOffset(int xIndex, int yIndex, double t)
{
	float multiplier;
	float offset, lookupTime;

	if (xIndex < 0 || xIndex >= tableWidth || yIndex < 0 || yIndex >= tableHeight)
		return 0.0;

	offset = timeOffset[yIndex * tableWidth + xIndex];
	multiplier = amplitudeMultiplier[yIndex * tableWidth + xIndex];

    if (gz4d::IsNan(t))
		t = currentGzTime;

	lookupTime = t - offset;

	if (rawStartTime <= lookupTime && lookupTime <= rawEndTime)
		return rawReadings[int((lookupTime - rawStartTime) / rawInterval)] * multiplier;
	else if (predictedStartTime <= lookupTime && lookupTime <= predictedEndTime)
		return predictedValues[int((lookupTime - predictedStartTime) / predictedInterval)] * multiplier;

	return 0.0;
}

/*---------------------------------------------------------------------------*/
void DynamicTides::getTimeOffsetAndMultiplier(double x, double y, float &offset, float &multiplier)
{
	int i, j;

        gz4d::Point<double> pos = pose->getPosition();
	j = int((x - pos[X]) / cellSpacing);
	i = int((y - pos[Y]) / cellSpacing);
	getTimeOffsetAndMultiplier(j, i, offset, multiplier);
}

/*---------------------------------------------------------------------------*/
void DynamicTides::getTimeOffsetAndMultiplier(int xIndex, int yIndex, float &offset, float &multiplier)
{
	if (xIndex < 0 || xIndex >= tableWidth || yIndex < 0 || yIndex >= tableHeight)
	{
		multiplier = 0.0; //not in the table, multiplier of 0 will nullify any effect
		offset = 0.0;
	}

	offset = timeOffset[yIndex * tableWidth + xIndex];
	multiplier = amplitudeMultiplier[yIndex * tableWidth + xIndex];
}

/*---------------------------------------------------------------------------*/
float DynamicTides::getTideOffsetGiven(float offset, float multiplier, double t)
{
	float lookupTime;

	if (gz4d::IsNan(t))
		t = currentGzTime;

	lookupTime = t - offset;

	if (rawStartTime <= lookupTime && lookupTime <= rawEndTime)
		return rawReadings[int((lookupTime - rawStartTime) / rawInterval)] * multiplier;
	else if (predictedStartTime <= lookupTime && lookupTime <= predictedEndTime)
		return predictedValues[int((lookupTime - predictedStartTime) / predictedInterval)] * multiplier;

	return 0.0;
}

/*---------------------------------------------------------------------------*/
/** Here, we should pick the closes object, time wise */
void DynamicTides::Idle(double time)
{
	int oldTimeSerial = currentTimeSerial;

	Object::Idle(time);
	
	currentGzTime = time;
	currentTimeSerial = (int)((time - rawStartTime) / rawInterval);

	if(currentTimeSerial != oldTimeSerial)
	{
		globalInputMediator->Trigger(Scene3D::objectChangedEvent());
	}
}

/*---------------------------------------------------------------------------*/
/** Draws the current object, in low res */
void DynamicTides::DrawLowRes(float degradation, Window3D * win)
{
	int i, j;

	EnableTexture();
	glNormal3f(0.0f, 0.0f, 1.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (i = 0; i < tableHeight - 1; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < tableWidth; j++)
		{
			glVertex3f(j * cellSpacing, i * cellSpacing, getTideOffset(j, i));
			glVertex3f(j * cellSpacing, (i + 1) * cellSpacing, getTideOffset(j, i + 1));
		}
		glEnd();
	}
	DisableTexture();
}

/*---------------------------------------------------------------------------*/
/** How far away, time wise from this bbox, if in bbox, returns 0 */
double DynamicTides::timeDistance(BBox b,double t)
{
	if(b.start<=t && b.end>=t)
		return 0.0;
	if(t<b.start)
	{
		return b.start-t;
	}
	else
	{
		return t-b.end;
	}
}

/*---------------------------------------------------------------------------*/
void DynamicTides::updateBBox()
{
	if (rawStartTime < predictedStartTime)
		bbox.start = rawStartTime;
	else
		bbox.start = predictedStartTime;

	if (rawEndTime > predictedEndTime)
		bbox.end = rawEndTime;
	else
		bbox.end = predictedEndTime;

	if (parent)
		parent->setBBoxStale();
}

/*---------------------------------------------------------------------------*/
int DynamicTides::perform(int argc, const char **argv, Tcl_Interp *interp)
{
	char retstr[16] = "";
	char command[256];
	GLboolean badSyntax = GL_FALSE, doPredicted = GL_TRUE, doRaw = GL_TRUE;

	if (argc >= 1)
	{
		if (!strcmp(argv[0], "config"))
		{
			if (argc >= 2)
			{
				if (!strcmp(argv[1], "daysAhead"))
				{
					if (argc >= 3)
						daysAhead = atoi(argv[2]);
					sprintf(retstr, "%i", daysAhead);
					Tcl_SetResult(interp, (char *) retstr, TCL_VOLATILE);
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "daysBack"))
				{
					if (argc >= 3)
						daysBack = atoi(argv[2]);
					sprintf(retstr, "%i", daysBack);
					Tcl_SetResult(interp, (char *) retstr, TCL_VOLATILE);
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "predicted"))
				{
					if (argc >= 3)
					{
						predictedFilename = fs::path(predictedFilename.parent_path()/argv[2]);
					}
					Tcl_SetResult(interp, (char *)predictedFilename.string().c_str(), TCL_VOLATILE);
					return TCL_OK;
				}
				else if (!strcmp(argv[1], "raw"))
				{
					if (argc >= 3)
					{
						rawFilename = fs::path(rawFilename.parent_path()/argv[2]);
					}
					Tcl_SetResult(interp, (char *)rawFilename.string().c_str(), TCL_VOLATILE);
					return TCL_OK;
				}
				else
					badSyntax = GL_TRUE;
			}
			if (argc < 2 || badSyntax)
			{
				Tcl_AppendResult(interp, "Usage: config {daysAhead [<days>] | daysBack [<days>] | predicted [<file>] | raw [<file>]}", NULL);
				return TCL_OK;
			}
		}
		if (!strcmp(argv[0], "update"))
		{
			if (argc >= 2)
			{
				if (!strcmp(argv[1], "predicted"))
					doRaw = GL_FALSE;
				else if (!strcmp(argv[1], "raw"))
					doPredicted = GL_FALSE;
				else if (!strcmp(argv[1], "web"))
				{
					sprintf(command, "dtGetRawTides %s %d", getAddress()->getAddress().c_str(), daysBack);
					mainWindow->Tcl_Eval(command);
					sprintf(command, "dtGetPredictedTides %s %d %d", getAddress()->getAddress().c_str(), daysBack, daysAhead);
					mainWindow->Tcl_Eval(command);
					return TCL_OK;
				}
				else
					badSyntax = GL_TRUE;
			}
			if (badSyntax)
			{
				Tcl_AppendResult(interp, "Usage: update {predicted | raw | web}", NULL);
				return TCL_OK;
			}
			if (doPredicted)
				loadPredictedReadingsFrom(predictedFilename.string());
			if (doRaw)
				loadRawReadingsFrom(rawFilename.string());
			updateBBox();
			return TCL_OK;
		}
	}
	return Object::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int DynamicTides::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "update config ", NULL);
	return Object::list(interp);
}


