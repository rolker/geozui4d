#include"S57_supplemental.h"
#include "engine/Scene3D.h"
#include "utilities/DottedAddress.h"

REGISTER_OBJECT(S57_supplemental,"");

#include <iostream>

static const char typeName[][16] = {"Supplemental", "Daymark", "Fog signal", "Light",
                                    "Radar reflector", "Retro-reflector", "Racon", "Topmark"
                                    };

static const char fogCategoryName[][16] = {"None",
                                           "Explosive",
                                           "Diaphone",
                                           "Siren",
                                           "Nautophone",
                                           "Reed",
                                           "Tyfon",
                                           "Bell",
                                           "Whistle",
                                           "Gong",
                                           "Horn"
                                          };

static const char fogSignalGeneration[][16] = {"None",
                                               "automatically",
                                               "by wave action",
                                               "by hand",
                                               "by wind"
                                              };

static const char topMarkShapeName[][48] = {"None",
                                            "Cone, point up",
                                            "Cone, point down",
                                            "Sphere",
                                            "2 spheres",
                                            "Cylinder (can)",
                                            "Board",
                                            "X-shape (St. Andrew's cross)",
                                            "Upright cross (St. George's cross)",
                                            "Cube, point up",
                                            "2 cones, point to point",
                                            "2 cones, base to base",
                                            "Rhombus (diamond)",
                                            "2 cones (points upward)",
                                            "2 cones (points downward)",
                                            "Besom, point up (broom or perch)",
                                            "Besom, point down (broom or perch)",
                                            "Flag",
                                            "Sphere over rhombus",
                                            "Square",
                                            "Rectangle, horizontal",
                                            "Rectangle, vertical",
                                            "Trapezium, up",
                                            "Trapezium, down",
                                            "Triangle, point up",
                                            "Triangle, point down",
                                            "Circle",
                                            "Two upright crosses (one over the other)",
                                            "T-shape",
                                            "Triangle pointing up over a circle",
                                            "Upright cross over a circle",
                                            "Rhombus over a circle",
                                            "Circle over a triangle pointing up",
                                            "Other shape (see information below)"
                                           };

static const char lightCategoryName[][24] = {"None",
                                             "directional function",
                                             "rear/upper light",
                                             "front/lower light",
                                             "leading light",
                                             "aero light",
                                             "air obstruction light",
                                             "fog detector light",
                                             "flood light",
                                             "strip light",
                                             "subsidiary",
                                             "spotlight",
                                             "front",
                                             "rear",
                                             "lower",
                                             "upper",
                                             "moire effect",
                                             "emergency",
                                             "bearing light",
                                             "horizontally disposed",
                                             "vertically disposed"
                                            };

static const char lightExhibitionName[][16] = {"None",
                                               "24 hours",
                                               "during the day",
                                               "during fog",
                                               "at night"
                                              };

static const char lightVisibilityName[][40] = {"None",
                                               "high intensity",
                                               "low intensity",
                                               "faint",
                                               "intensified",
                                               "unintensified",
                                               "visibility deliberately restricted",
                                               "obscured",
                                               "partially obscured"
                                              };

static const char lightCharacteristicName[][40] = {"None",
                                                   "fixed",
                                                   "flashing",
                                                   "long-flashing",
                                                   "quick-flashing",
                                                   "very quick-flashing",
                                                   "ultra quick-flashing",
                                                   "isophased",
                                                   "occulting",
                                                   "interrupted quick-flashing",
                                                   "interrupted very quick-flashing",
                                                   "interrupted ultra quick-flashing",
                                                   "morse",
                                                   "fixed/flash",
                                                   "flash/long-flash",
                                                   "occulting",
                                                   "fixed/long-flash",
                                                   "occulting",
                                                   "long-flash",
                                                   "flash",
                                                   "group",
                                                   "2 fixed (vertical)",
                                                   "2 fixed (horizontal)",
                                                   "3 fixed (vertical)",
                                                   "3 fixed (horizontal)",
                                                   "quick-flash",
                                                   "very quick-flash",
                                                   "ultra quick-flash",
                                                   "alternating",
                                                   "fixed and alternating flashing"
                                                  };

static const char raconCategoryName[][48] = {"None",
                                             "ramark, radar beacon transmitting continuously",
                                             "racon, radar transponder beacon",
                                             "leading racon/radar transponder beacon"
                                            };

/*---------------------------------------------------------------------------*/
S57_supplemental::S57_supplemental()
{
    type = NONE;
    parentS57 = NULL;
    sigPeriod = 0.0;
    range = 0.0;

    exhibition = 0;
    lightChar = 0;

    sector1 = sector2 = -1.0; //valid range is 0.0 to 360.0
}

/*---------------------------------------------------------------------------*/
S57_supplemental::~S57_supplemental()
{
}

/*---------------------------------------------------------------------------*/
void S57_supplemental::InitializeFor(const char *acronym)
{
    if (!strcmp(acronym, "DAYMAR"))
	{
        type = DAYMAR;
	}
    else if (!strcmp(acronym, "FOGSIG"))
	{
        SetInvisible();
        type = FOGSIG;
	}
    else if (!strcmp(acronym, "LIGHTS"))
	{
    	pointSize = 6;
    	lineWidth = 2;
        type = LIGHTS;
	}
    else if (!strcmp(acronym, "RADRFL"))
    {
        SetInvisible();
        type = RADRFL;
    }
    else if (!strcmp(acronym, "RETRFL"))
    {
        SetInvisible();
        type = RETRFL;
    }
    else if (!strcmp(acronym, "RTPBCN"))
    {
        SetInvisible();
        type = RTPBCN;
    }
    else if (!strcmp(acronym, "TOPMAR"))
    {
        SetInvisible();
        type = TOPMAR;
    }
}

/*---------------------------------------------------------------------------*/
void S57_supplemental::setParentS57(S57_object *newParent)
{
    parentS57 = newParent;
}

/*---------------------------------------------------------------------------*/
void S57_supplemental::DrawLowRes(float degradation, Window3D * win)
{
    if (parentS57 && parentS57->isSelected())
    {
        selectedFlag = true;
        S57_object::DrawLowRes(degradation, win);
        selectedFlag = false;
    }
    else
        S57_object::DrawLowRes(degradation, win);
}

/*---------------------------------------------------------------------------*/
void S57_supplemental::ShowInfo(GLuint *names, int numNames)
{
    if (parentS57)
        parentS57->ShowInfo(names, numNames);
    else
        S57_object::ShowInfo(names, numNames);
}

/*---------------------------------------------------------------------------*/
void S57_supplemental::ShowS57Info(Scene3D *scene)
{
	char infoLine[512];
    int i, size, val;
    bool hasParts;

    if (parentS57) //we need to say what we are if we have a parent
    {
        if (type >=0 && type <= TOPMAR)
        {
            if (type == LIGHTS && exhibition > 0 && exhibition <= 4)
                sprintf(infoLine, "---%s characteristics (exhibited %s)---\n", typeName[type], lightExhibitionName[exhibition]);
            else
                sprintf(infoLine, "---%s characteristics---\n", typeName[type]);
            scene->appendInfo(infoLine);
        }
        switch (type)
        {
            case FOGSIG:
                if (categoryList.size())
                {
                    val = categoryList[0];
                    if (val > 0 && val <= 10)
                    {
                        sprintf(infoLine, "Category:\t%s", fogCategoryName[val]);
                        scene->appendInfo(infoLine);
                    }
                }
                if (sigGen > 0 && sigGen <= 4)
                {
                    sprintf(infoLine, " (generated %s)\n", fogSignalGeneration[sigGen]);
                    scene->appendInfo(infoLine);
                }
                else
                    scene->appendInfo("\n");
                break;
            case RTPBCN:
                if (categoryList.size())
                {
                    val = categoryList[0];
                    if (val > 0 && val <= 3)
                    {
                        sprintf(infoLine, "Category:\t%s\n", raconCategoryName[val]);
                        scene->appendInfo(infoLine);
                    }
                }
                if (!radarWavelength.empty())
                {
                    sprintf(infoLine, "Wavelength:\t%s\n", radarWavelength.c_str());
                    scene->appendInfo(infoLine);
                }
                break;
            case DAYMAR:
                if (generateSpecialPurposeString(infoLine))
                    scene->appendInfo(infoLine);
                //fall through
            case TOPMAR:
                if (shape > 0 && shape <= 33)
                {
                    sprintf(infoLine, "Shape:\t%s\n", topMarkShapeName[shape]);
                    scene->appendInfo(infoLine);
                }
                break;
            case LIGHTS:
                //CATLIT
                size = categoryList.size();
                hasParts = false;
                for (i = 0; i < size; i++)
                {
                    val = categoryList[i];
                    if (val > 0 && val <= 20)
                    {
                        if (hasParts) //not first time through
                            strcat(infoLine, ", ");
                        else //first time through
                        {
                            strcpy(infoLine, "Category:\t");
                            hasParts = true;
                        }
                        strcat(infoLine, lightCategoryName[val]);
                    }
                }
                if (hasParts)
                    scene->appendInfo(infoLine, true);

                //COLOR, LITCHR, SIGGRP, & LITVIS (we suppress COLOR later)
                if (colorCode > 0 && colorCode <= 13)
                {
                    sprintf(infoLine, "Color:\t%s", colorName[colorCode]);
                    scene->appendInfo(infoLine);
                    if (lightChar > 0 && lightChar <= 29)
                    {
                        sprintf(infoLine, ", %s%s%s", lightCharacteristicName[lightChar], sigGroup.empty()? "" : " ", sigGroup.empty()? "":sigGroup.c_str());
                        scene->appendInfo(infoLine);
                    }
                    size = lightVisibilityList.size();
                    for (i = 0; i < size; i++)
                    {
                        val = lightVisibilityList[i];
                        if (val > 0 && val <= 8)
                        {
                            sprintf(infoLine, ", %s", lightVisibilityName[val]);
                            scene->appendInfo(infoLine);
                        }
                    }
                    scene->appendInfo("\n");
                }
                break;
        }

        if (type == FOGSIG || type == LIGHTS || type == RTPBCN)
        {
            if ((type==FOGSIG || type == RTPBCN) && !sigGroup.empty())
            {
                sprintf(infoLine, "Grouping:\t%s\n", sigGroup.c_str());
                scene->appendInfo(infoLine);
            }
            if (sigPeriod)
            {
                sprintf(infoLine, "Period:\t%.1f s", sigPeriod);
                scene->appendInfo(infoLine);
                if (!sigSequence.empty())
                    scene->appendInfo(",    ");
                else
                    scene->appendInfo("\n");
            }
            if (!sigSequence.empty())
            {
                sprintf(infoLine, "Sequence: %s\n", sigSequence.c_str());
                scene->appendInfo(infoLine);
            }
            if (range > 0.0)
            {
                sprintf(infoLine, "Range:\t%.1f naut. mi. (%.1f km) %s\n", range, (range * 1.852), (type == FOGSIG? "max." : "nominal"));
                scene->appendInfo(infoLine);
            }
        }
    }

    if (type == LIGHTS || type == RTPBCN) //suppress showing of color, because we do it first for lights.
    {
        if (sector1 >= 0.0 && sector2 >= 0.0)
        {
            sprintf(infoLine, "Sector:\t%.2f\\xb0 to %.2f\\xb0 (%.2f\\xb0 total)\n", sector1, sector2, sector2 - sector1 + ((sector1 > sector2)? 360.0: 0.0));
            scene->appendInfo(infoLine);
        }
        val = colorCode;
        colorCode = 0;
        S57_object::ShowS57Info(scene);
        colorCode = val;
    }
    else
        S57_object::ShowS57Info(scene);
}

/*---------------------------------------------------------------------------*/
void S57_supplemental::SetAttributes(gz4d::ogr::Feature item, S57_LayerInfo const &info, int objectNumber)
{
    std::string field;

    S57_object::SetAttributes(item, info, objectNumber);

    //get miscellaneous fields, set "field" variable for category. (NULL if not set)
    switch(type)
    {
        case FOGSIG:
            field = "CATFOG";
            sigGen = item["SIGGEN"].GetValue<int>();
            break;
        case RTPBCN:
            field = "CATRTB";
            radarWavelength = item["RADWAL"].GetValue<std::string>();
            goto get_sectors;
            break;
        case DAYMAR:
            field = "CATSPM";
            //fall through
        case TOPMAR:
            shape = item["TOPSHP"].GetValue<int>();
            break;
        case LIGHTS:
            field = "CATLIT";

            exhibition = item["EXCLIT"].GetValue<int>();
            lightChar = item["LITCHR"].GetValue<int>();
            lightVisibilityList = item["LITVIS"].GetValue<std::vector<int> >();
get_sectors:
        if (!item["SECTR1"].GetValue<std::string>().empty()) //since zero is a valid value, we need to see if a value exists first...
                sector1 = item["SECTR1"].GetValue<double>();
            if (!item["SECTR2"].GetValue<std::string>().empty())
                sector2 = item["SECTR2"].GetValue<double>();
            break;
    }
    
    if (!field.empty())
        categoryList = item[field].GetValue<std::vector<int> >();

    if (type == FOGSIG || type == LIGHTS || type == RTPBCN)
    {
        // SIGGRP
        sigGroup = item["SIGGRP"].GetValue<std::string>();

        // SIGSEQ
        sigSequence = item["SIGSEQ"].GetValue<std::string>();

        // SIGPER
        sigPeriod = item["SIGPER"].GetValue<int>();

        // VALMXR or VALNMR
        if (type == FOGSIG || type == RTPBCN)
            field = "VALMXR";
        else
            field = "VALNMR";
        range = item[field].GetValue<double>();
    }
}

/*---------------------------------------------------------------------------*/
int S57_supplemental::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    std::string message = argv[0];

    if (argc >= 1)
    {
        if (message == "getParentS57")
        {
            if (parentS57)
                Tcl_SetResult(interp, (char *)parentS57->getAddress()->getAddress().c_str(), TCL_VOLATILE);
            else
                Tcl_SetResult(interp, const_cast<char *>(""), TCL_VOLATILE);
            return TCL_OK;
        }
    }

    return Object::perform(argc, argv, interp);

}

/*---------------------------------------------------------------------------*/
int S57_supplemental::list(Tcl_Interp *interp)
{
	Tcl_AppendResult(interp, "getParentS57 ", NULL);
	return Object::list(interp);
}

