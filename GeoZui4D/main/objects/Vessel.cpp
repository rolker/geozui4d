/* Copyright (c) 2003 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// Vessel.cpp
/*---------------------------------------------------------------------------*/
/** 
 *  This object is used to visualize ships, potentially with shadows.
 *  
 *
 *  Created by mdp 1/28/2003
 *  Rederived from BasicVessel 9/8/2004
 */
/*---------------------------------------------------------------------------*/
#include "Vessel.h"
#include "utilities/VesselHistory.h"
#include "GZincludes.h"
#include "objects/GeoData/DataLines.h"
#include "engine/ObjectPoseState.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "engine/ViewManager.h"
#include "utilities/DottedAddress.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include "BasicVessel.h"
#include <string.h>
#include <stdlib.h>
#include "utilities/Gps.h"
#include "utilities/Ais.h"
#include <filesystem>
#include "utilities/TimePieces.h"
#include "engine/TimeControl.h"
#include "VesselPredictor.h"
#include "engine/gzCommandInterface.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <gz4d/glo/PushAttrib.h>
#include <gz4d/glo/Translate.h>

extern int dominantEye;
const int LoopCount = 50;

extern MainWindow *mainWindow;

REGISTER_OBJECT(Vessel, "vessel");

static const char *VesselNames[] = {BASICVESSEL_NAMES, "GPS_ADDRESS",
                                    "GPS_FILENAME", "GPS_PORT", "GPS_OFFSET",
                                    "ANCHOR_OFFSET", "GPS_UDP", ""};

/*---------------------------------------------------------------------------*/
Vessel::Vessel()
{
    SetTypeAndVersion("VESSEL", 1, 0);
    SetKeywordTable(VesselNames);
    
    vesselColor[0] = 0.7f;
    vesselColor[1] = 0.98f;
    vesselColor[2] = 0.5f;

    drawTail = false;
    predictor = NULL;
    showPredictor = GL_TRUE;

    latestVesselNode = NULL;

    currentDirection = 0;
    currentStrength = 0;

    priorSpeed = 0;

    showCrosstrack = 0;
    crosstrackPath = NULL;
    crosstrackIndex = 0;
    xError = yError = 0.0;

    dumplogFile = NULL;
}

/*--------------------------------------------------------------------------*/
Vessel::~Vessel()
{
}

/*--------------------------------------------------------------------------*/
int Vessel::processField(int option, char **fields)
{
    int i = 1;

    switch(option)
    {
        case ANCHOR_OFFSET:
            ConvertFloatPoint(anchor_offset,i);
            break;
        default:
            return BasicVessel::processField(option, fields);
            break;
    }
    return 0;
}

/** Load the XML information that is specific to vessel objects:AIS, GPS, sounder, and rudder info
 * @param[in] doc a pointer to an XML formatted document 
 * @param[in] cur the current pointer to the xml node being evaluated.
 * @param[in] path the full path of the XML file to be evaluated.
 * \return Object* - a pointer to the newly loaded object.
 * \par Purpose: 
 * To process all valid portions of an XML formatted file (i.e. gzx).
 * \note rudderSensorAngle and depthSounder are just NmeaSource types where gps and ais have their own respective classes.
 * \sa BasicVessel::LoadXML(),watchFor() (used to process key NMEA strings in file) 
 * \author Roland Arsenault (rja)
 * \date 2002
 * \par Important Comments: 
 *  BasicVessel::LoadXML() is called after XML info loaded from file and at the end of function. (This is no longer the case)
 */

Object * Vessel::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    int prog, lprog =0;
    Point3d gr;
    gr[2] = 0.0;
    DottedAddress *address;

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.anchor_offset"))
    {
        std::stringstream ss(*attribute);
        ss >> anchor_offset[0];
        ss >> anchor_offset[1];
        ss >> anchor_offset[2];
    }
    
    for(bpt::ptree::const_iterator node = xml.begin(); node != xml.end(); ++node)
        if (node->first == "Source")
        {
            std::string type = node->second.data();
            sources[type] = DataSource();

            if(attribute = node->second.get_optional<std::string>("<xmlattr>.filename"))
                sources[type].filename = *attribute;
            
            if(attribute = node->second.get_optional<std::string>("<xmlattr>.preload"))
                sources[type].preload = gz4d::IsTrue(*attribute);

            if(attribute = node->second.get_optional<std::string>("<xmlattr>.udp"))
                sources[type].udp = gz4d::IsTrue(*attribute);

            if(attribute = node->second.get_optional<std::string>("<xmlattr>.address"))
                sources[type].address = *attribute;

            sources[type].port = node->second.get("<xmlattr>.port",sources[type].port);

            if(attribute = node->second.get_optional<std::string>("<xmlattr>.logfile"))
                sources[type].logfile = fs::path(path)/(*attribute);

            if(attribute = node->second.get_optional<std::string>("<xmlattr>.offset"))
            {
                std::stringstream ss(*attribute);
                ss >> sources[type].offset[0];
                ss >> sources[type].offset[1];
                ss >> sources[type].offset[2];
            }

            if(attribute = node->second.get_optional<std::string>("<xmlattr>.filter_prefix"))
            {
                sources[type].prefix = *attribute;
                if(attribute->size() >= 3 && (*attribute)[2] == '~')
                    sources[type].filterIn = false;
            }
    }

    if(sources.count("rudder"))
    {
        if(!sources["rudder"].offset[X]&&!sources["rudder"].offset[Y]&&!sources["rudder"].offset[Z])
        {
            sources["rudder"].offset[Y] = -length[STERN];
        }
    }

    BasicVessel::LoadXML(xml, path, ncfile);

    if(sources.count("gps"))
    {
        if(!sources["gps"].address.empty() || sources["gps"].udp)
        {
            sources["gps"].nmea = new Gps(sources["gps"].address,sources["gps"].port,sources["gps"].udp,sources["gps"].logfile);
            generatingCMG = 0;
            if (!sources["gps"].prefix.empty())
            {
                sources["gps"].nmea->setFilterPrefix(sources["gps"].prefix);
                if (!sources["gps"].filterIn)
                    sources["gps"].nmea->setFilterType(NmeaSource::FilterOutPrefix);
            }
        }
    }

    for(SourceMap::iterator s = sources.begin(); s != sources.end(); ++s)
    {
        if(s->first != "gps")
        {
            if(!s->second.address.empty() || s->second.udp)
            {
                if(s->first == "ais")
                {
                    if(s->second.logfile.empty() &&  sources.count("gps") && sources["gps"].nmea)
                        s->second.nmea = new Ais(s->second.address, s->second.port, s->second.udp, sources["gps"].nmea);
                    else
                        s->second.nmea = new Ais(s->second.address, s->second.port, s->second.udp, s->second.logfile);
                }
                else
                {
                    if(s->second.logfile.empty() &&  sources.count("gps") && sources["gps"].nmea)
                        s->second.nmea = new NmeaSource(s->second.address, s->second.port, s->second.udp, sources["gps"].nmea);
                    else
                        s->second.nmea = new NmeaSource(s->second.address, s->second.port, s->second.udp, s->second.logfile);
                }
            }
        }
    }

    for(SourceMap::iterator s = sources.begin(); s != sources.end(); ++s)
    {
        if(!s->second.filename.empty())
        {
            if(s->first == "gps")
            {
                s->second.nmea = new Gps((fs::path(path)/s->second.filename).string());
                generatingCMG = 0;
            }
            else if(s->first == "ais")
            {
                s->second.nmea = new Ais((fs::path(path)/s->second.filename).string());
            }
            else
                s->second.nmea = new NmeaSource((fs::path(path)/s->second.filename).string());

            if(s->first == "sounder")
                s->second.nmea->watchFor('$', "DBS", NmeaSource::GENERIC_DATA, 2, NmeaSource::Float, &sounder_depth);

            if(s->first == "rudder")
                s->second.nmea->watchFor('$', "RSA", NmeaSource::GENERIC_DATA, 0, NmeaSource::Float, &rudder_angle);

            if (!s->second.prefix.empty())
            {
                s->second.nmea->setFilterPrefix(s->second.prefix);
                if (!s->second.filterIn)
                    s->second.nmea->setFilterType(NmeaSource::FilterOutPrefix);
            }
            for(int i=0;i<LoopCount;i++)
            {
                s->second.nmea->Idle();
            }

            if(s->first == "gps")
            {
                Gps *gps = dynamic_cast<Gps*>(s->second.nmea);
                autoProjection(gps->getLon(),gps->getLat());
                gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                gz4d::Point<double> geo = proj(gps->getLat(),gps->getLon());
                SetPosition(geo[0],geo[1],0.0);
            }

            if(s->second.preload)
            {
                createProgress(s->first+" file");
                while(s->second.nmea->isConnected())
                {
                    s->second.nmea->Idle();

                    if(s->first == "gps")
                    {
                        Gps *gps = dynamic_cast<Gps*>(s->second.nmea);
                        heading = gps->getHeading();
                        autoProjection(gps->getLon(),gps->getLat());
                        gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                        gz4d::Point<double> geo = proj(gps->getLat(),gps->getLon());

                        SetPositionOrientation(geo[0],geo[1],gps->getAltitude(),*(pose->getOrientation()),gps->GetCurrentTime());

                        //offsetToGPS->setPosition(gps_position);
                        //offsetToGPS->setOrientation(0.0, heading);
                    }

                    prog = (int)(s->second.nmea->getFileProgress()*100);

                    if(prog!=lprog)
                        updateProgress(prog);
                    lprog = prog;
                }
                killProgress();

                if(s->first == "gps")
                    hideOutsideTimeBounds = true;
            }
        }
    }

    address = getAddress();
    if (address)
    {
        if (sources.count("gps"))
        {
            sources["gps"].nmea->publishName("sourceGPS");
            sources["gps"].nmea->getAddress()->reparent(address);
        }
        if (sources.count("ais"))
        {
            sources["ais"].nmea->publishName("sourceAIS");
            sources["ais"].nmea->getAddress()->reparent(address);
        }
        if (sources.count("rudder"))
        {
            sources["rudder"].nmea->publishName("sourceRudder");
            sources["rudder"].nmea->getAddress()->reparent(address);
        }
        if (sources.count("depthSounder"))
        {
            sources["sounder"].nmea->publishName("sourceDepthSounder");
            sources["sounder"].nmea->getAddress()->reparent(address);
        }
    }

    if(sources.count("gps") && !sources["gps"].preload)
    {
        mainWindow->getScene3D()->timeControl->setRTsource(sources["gps"].nmea);
        mainWindow->getScene3D()->timeControl->useRT();
    }

    if (sources.count("rudder"))
        predictor = new VesselPredictor(this);
    return this;
}

int Vessel::readFrom(FILE *infile)
{
    //parse header
    if (BasicVessel::readFrom(infile))
        return 1;

/*    if(!gps_address.empty() || gps_udp)
    {
        gps=new Gps(gps_address,gps_port,gps_udp);
        if(gps)

            generatingCMG = 0;*/
/*      for(int i=0;i<5;i++)
        {
            //wait hopefully long enough to get a position to set a better georef
#ifndef WIN32
            sleep(1);
#else
            Sleep(1000);
#endif
            gps->Idle();
        }
        gr[0]=gps->getE();
        gr[1]=gps->getN();
        SetPosition(gr);*/
/*    }
    else if(!gps_filename.empty())
    {
        gps=new Gps(fs::path(GetFilename().parent_path()/gps_filename));
        if(gps)
            generatingCMG = 0;
        for(int i=0;i<5;i++)
        {
            gps->Idle();
        }
        gr[0]=gps->getE();
        gr[1]=gps->getN();
        SetPosition(gr);
    }*/
    

    return 0; //returns 0 if ok.
}

/** This function renders the object in the passed window.
 * @param[in] degradation -> A degradation of 0 indicates no degradation (high-res, for instance), while
 * a degradation of 1 indicates maximum degradation.   
 * @param[in] win -> can be used to determine information such as pixel-size-in-meters, etc.
 * \return nothing.
 * \note The degradation parameter informs the object to render itself more or less quickly.  
 * \sa BasicVessel::DrawLowRes(float,Window3D*), Object::DrawLowRes(float,Window3D*)
 * \author Matt Plumlee (mdp)
 * \date 2003
 * \par Important Comments: 
 * Reimplemented from BasicVessel
*/

void Vessel::DrawLowRes(float degradation, Window3D *win)
{
    PosableStateBase *p;
    BasicVessel::DrawLowRes(degradation, win);
    
    gz4d::glo::PushAttrib attrib(GL_POINT_BIT|GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    if (sources.count("sounder"))
    {
        float depth = newSounderDepth;
        float wid = depth;

        if (wid > width[HULL] / 3.0)
            wid = width[HULL] / 3.0;

        glColor3f(0.9f, 0.8f, 0.3f);
        depth *= 2;
        glBegin(GL_LINES);
            glVertex3fv(sources["sounder"].offset);
            glVertex3f(sources["sounder"].offset[X], sources["sounder"].offset[Y], sources["sounder"].offset[Z] - (depth * 5));

            glVertex3f(sources["sounder"].offset[X] - wid, sources["sounder"].offset[Y], sources["sounder"].offset[Z]);
            glVertex3f(sources["sounder"].offset[X] + wid, sources["sounder"].offset[Y], sources["sounder"].offset[Z] - depth);
            glVertex3f(sources["sounder"].offset[X] + wid, sources["sounder"].offset[Y], sources["sounder"].offset[Z]);
            glVertex3f(sources["sounder"].offset[X] - wid, sources["sounder"].offset[Y], sources["sounder"].offset[Z] - depth);

            glVertex3f(sources["sounder"].offset[X], sources["sounder"].offset[Y] - wid, sources["sounder"].offset[Z]);
            glVertex3f(sources["sounder"].offset[X], sources["sounder"].offset[Y] + wid, sources["sounder"].offset[Z] - depth);
            glVertex3f(sources["sounder"].offset[X], sources["sounder"].offset[Y] + wid, sources["sounder"].offset[Z]);
            glVertex3f(sources["sounder"].offset[X], sources["sounder"].offset[Y] - wid, sources["sounder"].offset[Z] - depth);
        glEnd();
        glPointSize(4.0f);

        if (win) //print the depth out near the vessel if the viewing parameters are appropriate
        {
            float scale, pitch;
            char depthString[50];

            p = win->getViewInfo()->getPosable();
            scale = p->getScale();
            pitch = p->getOrientationAxis()[Pitch];
            if (depth != 0.0 && scale > .002f && scale < .1f && pitch < 30)
            {
                sprintf(depthString, "Depth: %.1fm", depth);
                win->printAt(sources["sounder"].offset[X], sources["sounder"].offset[Y], sources["sounder"].offset[Z] - 3.0,depthString,1,GLUT_BITMAP_HELVETICA_18, 25, -20, GL_TRUE);
            }
        }
    }

    // to visualize the rudder angle if data is given. Added 5/23/05 - briana 
    if (sources.count("rudder"))
    {

        //glPointSize(5.0f);
        //glColor3f(1.0f, 1.0f, 0.0f);

        if (showPredictor)
            predictor->DrawLowRes(degradation, win);
        
        if (!toggle)
        {
            glLineWidth(2.0f);
            glColor3f(1.0f, 1.0f, 0.0f);
            glPushMatrix();
            glTranslatef(sources["rudder"].offset[X], sources["rudder"].offset[Y], sources["rudder"].offset[Z]);
            
            glRotatef(newRudderAngle, 0, 0, 1);
            glBegin(GL_LINE_STRIP);
                glVertex3f(0.0f, 0.0f, 0.0f);   
                //glVertex3fv(rudder_offset);
                //glVertex3f(rudder_offset[X], rudder_offset[Y]-length[HULL]/2, rudder_offset[Z]);
                glVertex3f(0.0f, -length[HULL]/2, 0.0f);
            glEnd();
            glPopMatrix();

            glLineWidth(8.0f);
            glColor3f(0.0f, 0.0f, 0.0f);
            glPushMatrix();
            glTranslatef(sources["rudder"].offset[X], sources["rudder"].offset[Y], sources["rudder"].offset[Z]);

            glRotatef(newRudderAngle, 0, 0, 1);
            glBegin(GL_LINE_STRIP);
                glVertex3f(0.0f, 0.0f, 0.0f);   
                //glVertex3fv(rudder_offset);
                //glVertex3f(rudder_offset[X], rudder_offset[Y]-length[HULL]/2, rudder_offset[Z]);
                glVertex3f(0.0f, -length[HULL]/2, 0.0f);
            glEnd();
            glLineWidth(1.0f);
            glPopMatrix();
        }
    }

    if (showCrosstrack && crosstrackPath)
    {
        glPushMatrix();
        glLoadIdentity();
        gz4d::glo::Translate(pose->getPosition());
        glLineWidth(2.0f);
        glColor3f(0.6f, 0.8f, 0.9f);
        glBegin(GL_LINE_STRIP);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(-xError, -yError, 0.0);
        glEnd();
        glPopMatrix();
    }
  /*    if (ais)
    {
        glDisable(GL_LINE_SMOOTH);
        glPopMatrix();//get back to scene coordinates
        ais->drawTargets(win, pose->getPosition()); 
        glPushMatrix();
    }*/
}

/*---------------------------------------------------------------------------*/
int Vessel::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    const char *message = argv[0];
    char retString[200];
    double val, hundredths;
    float minutes = 0.0;
    int deg = 0, intval;
    GLboolean needsFormatting = GL_FALSE;

    if (argc >= 1)
    {
        if (!strcmp(message, "getGPSinfo"))
        {
            if (sources.count("gps") == 0 || !sources["gps"].nmea)
            {
                Tcl_AppendResult(interp, getAddress()->getAddress().c_str(), " is not connected to a GPS source", NULL);
                return TCL_ERROR;
            }
            if (argc >= 2)
            {
                Gps * gps = dynamic_cast<Gps*>(sources["gps"].nmea);
                message = argv[1];
                if (!strcmp(message, "longitude"))
                {
                    val = gps->getLon();
                    if (argc >= 3)
                    {
                        deg = int(val);
                        minutes = val-deg;
                        needsFormatting = GL_TRUE;
                    }
                }
                else if (!strcmp(message, "latitude"))
                {
                    val = gps->getLat();
                    if (argc >= 3)
                    {
                        deg = int(val);
                        minutes = val-deg;
                        needsFormatting = GL_TRUE;
                    }
                }
                else if (!strcmp(message, "heading"))
                    val = gps->getHeading();
                else if (!strcmp(message, "altitude"))
                    val = gps->getAltitude();
                else if (!strcmp(message, "cog"))
                    val = gps->getCOG();
                else if (!strcmp(message, "sog"))
                    val = gps->getSOGinKnots();
                else
                    argc = 1;
            }
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "usage: getGPSinfo {longitude | latitude | heading | altitude | cog | sog} [dms | dmh | dd]", NULL);
                return TCL_ERROR;
            }
            if (needsFormatting)
            {
                if (!strcmp(argv[2], "dd"))
                    sprintf(retString, "%f", val);
                else
                {
                    if (!strcmp(argv[2], "dmh"))
                        sprintf(retString, "%d\xB0%02.1f'", deg, minutes);
                    else
                    {
                        hundredths = modf(minutes, &val);
                        sprintf(retString, "%d\xB0%02d'%02d\"", deg, (int)val, (int)(hundredths * 60.0));
                    }
                }
            }
            else
                sprintf(retString, "%.2f", val);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "predictor"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "array"))
                {
                    if (argc >= 3)
                        predictor->showRegularPredictors(atoi(argv[2]));
                    sprintf(retString, "%i", predictor->showRegularPredictors());
                    Tcl_SetResult(interp, retString, TCL_VOLATILE);
                    return TCL_OK;
                }
                else if (argv[1][0] == 'o' && argv[1][1] == 'n')
                    showPredictor = 1;
                else
                    showPredictor = atoi(argv[1]);
            }
            sprintf(retString, "%i", showPredictor);
            Tcl_SetResult(interp, retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "lookahead"))
        {
            if (!predictor)
            {
                Tcl_AppendResult(interp, "No predictor is available for this vessel to apply a lookahead to.", NULL);
                return TCL_ERROR;
            }

            if (argc >= 2)
                intval = predictor->setLookahead(atoi(argv[1]));
            else
                intval = predictor->setLookahead(); //just get the value
            sprintf(retString, "%i", intval);
            Tcl_SetResult(interp, retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (!strcmp(message, "current"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "direction"))
                {
                    if (argc >= 3)
                        currentDirection = atof(argv[2]);
                    sprintf(retString, "%f", currentDirection);
                    Tcl_SetResult(interp, retString, TCL_VOLATILE);
                    return TCL_OK;
                }
                else if (!strcmp(argv[1], "strength"))
                {
                    if (argc >= 3)
                        currentStrength = .51444 * atof(argv[2]);
                    sprintf(retString, "%f in knots (%f in m/s)", currentStrength / .51444, currentStrength);
                    Tcl_SetResult(interp, retString, TCL_VOLATILE);
                    return TCL_OK;
                }
/*
                else if (!strcmp(argv[1], "fudge"))
                {
                    if (argc >= 3)
                        predictor->fudge = atof(argv[2]);
                    sprintf(retString, "%f", predictor->fudge);
                    Tcl_SetResult(interp, retString, TCL_VOLATILE);
                    return TCL_OK;
                }
*/
            }
            Tcl_AppendResult(interp, "usage: current {direction | strength} [<val>]", NULL);
            return TCL_ERROR;
        }
        if (!strcmp(message, "logfile"))
        {
            NmeaSource *source = NULL;
            int fileArg = 2;

            if (argc >= 2)
            {
                source = sources[argv[1]].nmea;
                if (fileArg < argc)
                {
                    if (argv[fileArg][0] == '-')
                        source->changeLogfile();
                    else if (argv[fileArg][0] == '=')
                        source->shareLogfileWith(sources["gps"].nmea);
                    else
                        source->changeLogfile(argv[fileArg]);
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "usage: logfile [gps | ais | rudder | sounder] {<filename> | - | =}", NULL);
            return TCL_ERROR;
        }
        //Don't document dumplog, it's for the predictor experiments.
        if (!strcmp(message, "dumplog")) //where should we spit out the collated values (including crosstrack error)?
        {
            if (argc >= 2)
            {
                if (argv[1][0] == '-')
                {
                    if (dumplogFile)
                    {
                        fclose(dumplogFile);
                        dumplogFile = NULL;
                    }
                }
                else
                {
                    dumplogFile = fopen(argv[1], "a");
                    VesselSnapshotNode::writeDataHeader(dumplogFile);
                }
                return TCL_OK;
            }
            Tcl_AppendResult(interp, "usage: dumplog {<filename> | -}", NULL);
            return TCL_ERROR;
        }
        if (!strcmp(message, "infile"))
        {
            NmeaSource *source = NULL;
            int fileArg = 2;

            if (argc >= 2)
            {
                source = sources[argv[1]].nmea;
                if (fileArg < argc)
                {
                    if (argv[fileArg][0] == '-')
                        source->changeInputFile();
                    else
                        source->changeInputFile(argv[fileArg]);
                    detailHistory.reset();
                    poseHistory.reset();
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "usage: infile [gps | ais | rudder | sounder] {<filename> | -}", NULL);
            return TCL_ERROR;
        }
        if (!strcmp(message, "crosstrack"))
        {
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "show"))
                {
                    if (argc >= 3)
                        showCrosstrack = atoi(argv[2]);
                    sprintf(retString, "%d", showCrosstrack);
                    Tcl_SetResult(interp, retString, TCL_VOLATILE);
                    return TCL_OK;
                }
                else if (!strcmp(argv[1], "path"))
                {
                    Addressable *address;
                    Object *ob;
                    int ret;

                    if (argc >= 3)
                    {
                        ret = gzCommandInterface::findObject(argv[2], address, ob);
                        if (ret != TCL_OK)
                            return ret;
                        DYNAMIC_CAST(crosstrackPath, DataLines *, ob);
                        if (!crosstrackPath)
                        {
                            Tcl_AppendResult(interp, argv[2], " is not of type DataLines.  Discarding.", NULL);
                            return TCL_ERROR;
                        }
                        crosstrackIndex = 0;
                    }
                    if (crosstrackPath)
                        sprintf(retString, "%s", crosstrackPath->getAddress()->getAddress().c_str());
                    else
                        strcpy(retString, "<none>");
                    Tcl_SetResult(interp, retString, TCL_VOLATILE);
                    return TCL_OK;
                }
            }
            Tcl_AppendResult(interp, "usage: crosstrack {show [0|1] | path [<pathname>]}", NULL);
            return TCL_ERROR;
        }
    }

    return BasicVessel::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int Vessel::list(Tcl_Interp *interp)
{
    Tcl_AppendResult(interp, "getGPSinfo predictor lookahead current logfile infile crosstrack ", NULL);
    return BasicVessel::list(interp);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void Vessel::Idle(double time)
{
    //int i;
    float course = 0, speed = 0, XTEx, XTEy, headingDiff;
    OrientationInfo orient;

    if(sources.count("gps") && !sources["gps"].preload)
    {
        Gps * gps = dynamic_cast<Gps*>(sources["gps"].nmea);
        if(gps)
        {
            gps->Idle();
            
            if(gps->haveHeading())
            {
                //pose->setOrientation(0.0,gps->getHeading());
                orient.axis[Yaw] = heading = gps->getHeading();
                if (gps->haveCog())
                    setCourse(course = gps->getCOG());
                calculateCOG = GL_FALSE;
            }
            else if(gps->haveCog())
            {
                setCourse(course = gps->getCOG());
                orient.axis[Yaw] = gps->getCOG();
                calculateCOG = GL_FALSE;
            }
            if (gps->haveSog())
            {
                speed = gps->getSOG();
                if (priorSpeed > 1.0 && speed == 0.0) //we seem to have a problem with good sog data... (mdp 6/28/2005)
                    speed = priorSpeed;
                else
                    priorSpeed = speed;
                setSpeed(speed);
    //      cerr << speed << endl;
            }
            
            //* This sets the poseHistory position and orientation
            autoProjection(gps->getLon(),gps->getLat());
            gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
            gz4d::Point<double> en = proj(gps->getLat(),gps->getLon());

            SetVesselPositionOrientation(en[0],en[1],0.0,
                orient,gps->GetCurrentTime());

            if (latestVesselNode)
            {
                if (!calculateCOG)
                {
                    latestVesselNode->setCourseAndSpeed(course, speed);
                }
                if (crosstrackPath)
                {
                    calculateTrackLocation(latestVesselNode->lineNode->position[X], latestVesselNode->lineNode->position[Y], latestVesselNode->orientation.axis[Yaw], XTEx, XTEy, headingDiff);
                    latestVesselNode->setCrosstrackError(XTEx, XTEy, headingDiff);
                }
            }
            course = speed = 0;
        }
    }

    bool rmcTime = true;
    if(sources.count("gps") && sources["gps"].nmea)
        rmcTime = !dynamic_cast<Gps*>(sources["gps"].nmea)->rmc_time;

    if (sources.count("ais") && !sources["ais"].preload)
    {
        sources["ais"].nmea->Idle(rmcTime);
    }

    if (sources.count("sounder") && !sources["sounder"].preload)
    {
        sources["sounder"].nmea->Idle(rmcTime);
        //* This sets the latest VesselSnapshotNode's depthSounder in VesselHistory -briana
        if(latestVesselNode)
            latestVesselNode->setDepthSounder(sounder_depth, sources["sounder"].offset);
    }

    if (sources.count("rudder") && !sources["rudder"].preload)
    {
        sources["ais"].nmea->Idle(rmcTime);
        //* This sets the latest VesselSnapshotNode's rudder orientation in VesselHistory -briana           
        if(latestVesselNode)
        {
            latestVesselNode->setRudder(rudder_angle, sources["rudder"].offset);
        }
    }
    BasicVessel::Idle(time);

    //this section of idle figures out what time it is and sets the appropriate values from the detailHistory
    //saves accurate information for rendering
    if(detailHistory)
    {
        //add interpolate rudder angle if desired
        if(interpolatePositions)
        {
            OrientationInfo interpolatedRudderOrientation;
            double interpolatedSounderDepth;
            if(sources.count("rudder"))
            {
                if (detailHistory->getInterpolatedRudderOrientation(time, interpolatedRudderOrientation))
                {
                    //class variables to be used for rendering
                    newRudderAngle = interpolatedRudderOrientation.axis[Yaw];
                }
            }
            if (sources.count("sounder"))
            {
                if (detailHistory->getInterpolatedSounderDepth(time, interpolatedSounderDepth))
                {
                    //class variables to be used for rendering
                    newSounderDepth = interpolatedSounderDepth;
                }
            }
            if (crosstrackPath)
                detailHistory->getInterpolatedCrosstrackErrorXY(time, xError, yError);
            detailHistory->getInterpolatedCourseAndSpeed(time, course, speed);
        }
        else 
        {
            newRudderAngle = (detailHistory->getRudderOrientation())->axis[Yaw];
            //todo: add rudderOffset here

            // class variable to be used for rendering
            newSounderDepth = detailHistory->getSounderDepth();
            course = detailHistory->getCourse();
            speed = detailHistory->getSpeed();
            xError = detailHistory->getCrosstrackErrorX();
            yError = detailHistory->getCrosstrackErrorY();
        }
        setCourse(course);
        setSpeed(speed);
        if(sources.count("rudder"))
            predictor->setInfo(time, newRudderAngle, pose->getOrientationAxis()[Yaw], course, speed, currentDirection, currentStrength);

//      cerr << "time: " << (time - 1122985600.0) << ",\trudder: " << newRudderAngle << ",\tcourse: " << course << ",\tspeed: " << speed << endl;
    }
}

/*---------------------------------------------------------------------------*/
void Vessel::SetVesselPositionOrientation(double x, double y, double z, OrientationInfo o,double t)
{
    VesselSnapshotNode *previousNode;
    double prevt=0.0;

    if (!detailHistory)
        poseHistory = detailHistory = std::shared_ptr<VesselHistory>(new VesselHistory(this));

    if (dumplogFile && latestVesselNode)
    {
        previousNode = latestVesselNode;
        prevt = detailHistory->getTime();
    }
    else
        previousNode = NULL;

    detailHistory->insert(t, (float)x, (float)y, (float)z, o);

    // Save the last vessel node inserted to update specific vessel details (i.e. rudder, etc)
    latestVesselNode = detailHistory->getLast();
    if (dumplogFile && previousNode && previousNode != latestVesselNode) //if we're dumping the a log if the vessel info, do it now that we're moving on to the next one.
        previousNode->writeData(prevt, dumplogFile);

    expandBBoxTime(t);
}

/*---------------------------------------------------------------------------*/
/** This function calculates the distance between the passed position and the
 *  crosstrackPath, storing the x and y components in XTEx and XTEy and
 *  returning true.  The current index along the crosstrackPath is kept so that
 *  paths that cross themselves (or come close to it) don't confuse us.  This
 *  introduces a restriction on how the function can be called: it should not
 *  be called "at random" or "too far apart" with respect to the makeup of
 *  the path.  If you can, try to call this function enough so that the index
 *  on the crosstrackPath need be advanced no more than one leg per call.
 *  This will generally be fine if one is using real (real-time or recorded)
 *  data on a path that has at least a few meters between waypoints, and a call
 *  is made to this function for each new position.  If the crosstrackPath is
 *  NULL, sets XTEx and XTEy to 0.0 and returns false.  If the end of the
 *  crosstrackPath has been reached, returns false without changing XTEx or XTEy.
 *  Calculations of distance from point to line adapted from
 *  http://astronomy.swin.edu.au/~pbourke/geometry/pointline/
 */
bool Vessel::calculateTrackLocation(float posx, float posy, float h, float &XTEx, float &XTEy, float &headingDifference)
{
    float dx, dy, distsq = 0.0, distsq2=0.0, u, v, heading;
    bool haveTentativeAnswer = false;

    if (!crosstrackPath)
    {
        XTEx = XTEy = 0.0;
        return false;
    }

    while (crosstrackIndex < crosstrackPath->getPointArray().size()-1)
    {
        gz4d::Point<double> p1 = crosstrackPath->getPointArray()[crosstrackIndex];
        gz4d::Point<double> p2 = crosstrackPath->getPointArray()[crosstrackIndex + 1];
        dx = p2[X] - p1[X];
        dy = p2[Y] - p1[Y];
        v = dx * dx + dy * dy;
        //check if p1 and p2 are too close... if so, advance to p2.
        if (v < 0.0001)
        {
            crosstrackIndex++;
            continue;
        }
        //this line calculates how far along the line segment we are (from 0 to 1)
        u = (((posx - p1[X]) * (p2[X] - p1[X])) + ((posy - p1[Y]) * (p2[Y] - p1[Y]))) / v;
        if (u < 0) //we're "before" the segment... just calculate distance to p1
        {
            if (haveTentativeAnswer) //check distance difference first
            {
                dx = posx - p1[X];
                dy = posy - p1[Y];
                if ((dx * dx + dy * dy) < distsq2)
                {
                    XTEx = dx;
                    XTEy = dy;
                }
                else //otherwise, we already have the right answer in XTEx|y
                    crosstrackIndex--; //restore the right index
            }
            else
            {
                XTEx = posx - p1[X];
                XTEy = posy - p1[Y];
            }
            heading = atan2(XTEx, XTEy);
            headingDifference = h - heading;
            return true;
        }
        if (u < 1) //we're "along" the segment, find deltas to segment
        {
            dx = posx - (p1[X] + u * (p2[X] - p1[X]));
            dy = posy - (p1[Y] + u * (p2[Y] - p1[Y]));
            distsq = dx * dx + dy * dy;
            if (haveTentativeAnswer) //oops, overlap between segments
            {
                if (distsq < distsq2)
                {
                    XTEx = dx;
                    XTEy = dy;
                }
                else
                    crosstrackIndex--;//restore the right index
                heading = atan2(XTEx, XTEy);
                headingDifference = h - heading;
                return true;
            }
            else
            {
                XTEx = dx;
                XTEy = dy;
                distsq2 = distsq;
                haveTentativeAnswer = true; //good, but check next segment, too.
            }
        }
        crosstrackIndex++;
    }

    if (crosstrackIndex >= crosstrackPath->getPointArray().size() && !haveTentativeAnswer)
        return false;
    //if we have a tentative answer, back up for next time
    crosstrackIndex--;
    heading = atan2(XTEx, XTEy);
    headingDifference = h - heading;
    return true;
}
