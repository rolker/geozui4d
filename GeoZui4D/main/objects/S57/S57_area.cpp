// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include"S57_area.h"
#include "engine/Scene3D.h"
#include<iostream>

using namespace std;

REGISTER_OBJECT(S57_area,"");

static const char restrictionName[][64] = {"<none>",
                                           "Anchoring prohibited",
                                           "Anchoring restricted",
                                           "Fishing prohibited",
                                           "Fishing restricted",    
                                           "Trawling prohibited",
                                           "Trawling restricted",
                                           "Entry prohibited",
                                           "Entry restricted",
                                           "Dredging prohibited",
                                           "Dredging restricted",
                                           "Diving prohibited",
                                           "Diving restricted",
                                           "No wake",
                                           "Area to be avoided",
                                           "Construction prohibited",
                                           "Discharging prohibited",
                                           "Discharging restricted",
                                           "Industrial or mineral exploration/development prohibited",
                                           "Industrial or mineral exploration/development restricted",
                                           "Drilling prohibited",
                                           "Drilling restricted",
                                           "Removal of historical artifacts prohibited",
                                           "Cargo transhipment (lightering) prohibited",
                                           "Dragging prohibited",
                                           "Stopping prohibited",
                                           "Landing prohibited",
                                           "Speed restricted"
                                          };

static const char platformCategoryName[][64] = {"<none>",
                                                "Oil derrick/rig",
                                                "Production platform",
                                                "Observation/research platform",
                                                "Articulated loading platform",
                                                "Single anchor leg mooring (SALM)",
                                                "Mooring tower",
                                                "Artificial island",
                                                "Floating production, storage and off-loading vessel (FPSO)",
                                                "Accommodation platform",
                                                "Navigation, communication and control buoy (NCCB)"
                                               };

static const char pylonCategoryName[][32] = {"<none>",
                                             "Power transmission pylon/pole",
                                             "Telephone/telegraph pylon/pole",
                                             "Aerial cableway/sky pylon",
                                             "Bridge pylon/tower",
                                             "Bridge pier"
                                            };

S57_area::S57_area()
{
    colour[0] = 0.8f;
    colour[1] = 0.2f;
    colour[2] = 0.9f;
    type = NONE;
    value = 0.0;
    restriction = 0;
}

S57_area::~S57_area()
{
}

void S57_area::InitializeFor(const char *acronym)
{
	if (!strcmp(acronym, "BERTHS")) //try brown?
	{
        colour[0] = 0.4f;
		colour[1] = 0.4f;
		colour[2] = 0.1f;
        pointSize = 2.0f;
        type = BERTHS;
	}
	if (!strcmp(acronym, "CTNARE")) //try orange-yellow?
	{
		colour[1] = 0.7f;
		colour[2] = 0.5f;
        type = CTNARE;
	}
    else if (!strcmp(acronym, "RESARE")) //red/orange--this is important
    {
        colour[0] = 0.9f;
        colour[1] = 0.6f;
        colour[2] = 0.3f;
        type = RESARE;
    }
	else if (!strcmp(acronym, "DRGARE")) //almost black
	{
		colour[0] = 0.1f;
		colour[1] = 0.1f;
		colour[2] = 0.1f;
		lineWidth = 2.0;
        type = DRGARE;
	}
	else if (!strcmp(acronym, "ISTZNE")) //try light green?
	{
		colour[0] = 0.7f;
		colour[1] = 0.9f;
		colour[2] = 0.7f;
        type = ISTZNE;
	}
	else if (!strcmp(acronym, "OFSPLF")) //Very dark brown
	{
		colour[0] = 0.3f;
		colour[1] = 0.3f;
		colour[2] = 0.1f;
		lineWidth = 2.0;
        pointSize = 4.0;
        type = OFSPLF;
	}
	else if (!strcmp(acronym, "PRCARE")) //Very dark grey
	{
		colour[0] = 0.2f;
		colour[1] = 0.2f;
		colour[2] = 0.2f;
		lineWidth = 2.0;
        type = PRCARE;
	}
	else if (!strcmp(acronym, "PYLONS")) //Very dark brown
	{
		colour[0] = 0.2f;
		colour[1] = 0.2f;
		colour[2] = 0.2f;
		lineWidth = 2.0;
        pointSize = 4.0;
        type = PYLONS;
	}
	else if (!strcmp(acronym, "UNSARE")) //try deep blue?
	{
		colour[0] = 0.1f;
		colour[1] = 0.2f;
		colour[2] = 1.0f;
        type = UNSARE;
	}
}

void S57_area::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{

    S57_object::SetAttributes(item, info, objectNumber);

    if (type == DRGARE || type == BERTHS)
    {
            value = item["DRVAL1"].GetValue<double>();
    }
    else if (type == OFSPLF)
    {
        category = item["CATOFP"].GetValue<int>();
    }
    else if (type == PYLONS)
    {
        category = item["CATPYL"].GetValue<int>();
    }
    if(item["RESTRN"])
        restriction = item["RESTRN"].GetValue<int>();
}

/*---------------------------------------------------------------------------*/
void S57_area::ShowS57Info(Scene3D *scene)
{
	char infoLine[512];

    if (type == DRGARE || type == BERTHS)
    {
        sprintf(infoLine, "Depth:\t%.1f m shoalest depth (%.1f ft)\n", value, value * 3.2808399);
        scene->appendInfo(infoLine);
    }
    if (type == OFSPLF && category != 0)
    {
        sprintf(infoLine, "Category:\t%s\n", platformCategoryName[category]);
        scene->appendInfo(infoLine);
    }
    if (type == PYLONS && category != 0)
    {
        sprintf(infoLine, "Category:\t%s\n", pylonCategoryName[category]);
        scene->appendInfo(infoLine);
    }
    if (restriction != 0)
    {
        sprintf(infoLine, "*** %s ***\n", restrictionName[restriction]);
        scene->appendInfo(infoLine);
    }
    S57_object::ShowS57Info(scene);
}
