// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0
/*---------------------------------------------------------------------------*/
// BasicVessel.cpp
/*---------------------------------------------------------------------------*/
/** 
*	This object is used to visualize ships, potentially with shadows.
*	
*
*	Adapted from Vessel by mdp 8/19/2004 (for vessels w/out real time input)
*/
/*---------------------------------------------------------------------------*/
#include "GZincludes.h"
#include "engine/ObjectPoseState.h"
#include "BasicVessel.h"
#include "widgets/Sv_Standards.h"
#include "engine/Window3D.h"
#include "utilities/DottedAddress.h"
#include "engine/PosableStateRelationships/PosableStateLock.h"
#include <string.h>
#include <stdlib.h>
#include <filesystem>
#include "utilities/Utilities.h"
#include "engine/gzCommandInterface.h"
#include "engine/ViewManager.h"
#include <gz4d/util/Utilities.h>
#include "utilities/ObjectHistory.h"
#include <gz4d/geo/Projection.h>
#include <boost/property_tree/xml_parser.hpp>

#ifndef WIN32
#include <unistd.h>
#endif

extern int dominantEye;

extern MainWindow *mainWindow;

REGISTER_OBJECT(BasicVessel, "bvessel");

static const char *VesselNames[] = {BASICVESSEL_NAMES, ""};

/*---------------------------------------------------------------------------*/
BasicVessel::BasicVessel()
: ObjectCollection(GL_FALSE),
tail(60, 0.5f),showDepth(false)
{
//	cerr << "BasicVessel::BasicVessel\n";
        SetTypeAndVersion("BASICVESSEL", 1, 0);
        SetKeywordTable(VesselNames);
        pose = new ObjectPoseState(this);

        //courseMadeGood is an absolute value in position, absolute in scale and orientation.
        courseMadeGood = new ObjectPoseState(this, "courseMadeGood");
        lockFromCMG = new PosableStateLock(this, pose);

        width[BOW] = 2.0f;
        width[BRIDGE] = 20.0f;
        width[HULL] = 25.0f;
        width[STERN] = 5.0f;

        length[BOW] = 30.0f;
        length[HULL] = 75.0f;
        length[STERN] = 20.0f;
        length[BRIDGE] = 40.0f;

        height[BOW] = 3.0f;
        height[HULL] = height[BRIDGE] = 18.0f;
        height[STERN] = 15.0f;

        generatingCMG = GL_TRUE;
        generatingSOG = GL_TRUE;

        drawOpaque = GL_FALSE;
        cogPitch = GL_FALSE;

        target_inner_size=3;
        target_outer_size=5;
        target_inner_color[0]=0.0;
        target_inner_color[1]=1.0;
        target_inner_color[2]=0.0;
        target_outer_color[0]=1.0;
        target_outer_color[1]=1.0;
        target_outer_color[2]=1.0;

        offsetToGPS = offsetToBridge = offsetToStern = NULL;
        lockToGPS = lockFromBridge = lockFromStern = NULL;

        hideOutsideTimeBounds = false;
        interpolatePositions = 1;

        vesselColor[0] = 0.5f;
        vesselColor[1] = 0.55f;
        vesselColor[2] = 0.6f;

        drawTail = GL_FALSE;
        drawCMG = GL_FALSE;
        drawChildren = GL_TRUE;
        onlyModelWin = NULL;
        shadowOver = NULL;

        toggle = 0;
    
    lastPos[0] = 0.0;
    lastPos[1] = 0.0;
    lastPos[2] = 0.0;

}

/*--------------------------------------------------------------------------*/
BasicVessel::~BasicVessel()
{
        delete courseMadeGood;
//	delete lockFromCMG;

        if (offsetToGPS)
                delete offsetToGPS;

        if (offsetToBridge)
                delete offsetToBridge;
        if (offsetToStern)
                delete offsetToStern;

        if (lockToGPS)
                delete lockToGPS;
        if (lockFromBridge)
                delete lockFromBridge;
        if (lockFromStern)
                delete lockFromStern;
}

/*--------------------------------------------------------------------------*/
void BasicVessel::initializingLabel()
{
        pose->getAddress()->addChild(courseMadeGood->getAddress());
        lockFromCMG->publishName("CMGlock");
}

/*--------------------------------------------------------------------------*/
int BasicVessel::processField(int option, char **fields)
{
    int i = 1;
    int sizeType = -1;

    switch(option)
    {
                case BOW_SIZE:
                case HULL_SIZE:
                case STERN_SIZE:
                case BRIDGE_SIZE:
                        sizeType = option - BOW_SIZE + BOW;
                        width[sizeType] = ConvertFloat(i);
                        length[sizeType] = ConvertFloat(i);
                        height[sizeType] = ConvertFloat(i);
                        break;
                case SHADOW:
//			shadow = Lookup(fields[i++]);
                        break;
                default:
                        return Object::processField(option, fields);
                        break;
    }
    return 0;
}

/*--------------------------------------------------------------------------*/
void BasicVessel::establishOffset(PosableStateBase *&offsetPose, const char *offsetName, PosableStateLock *&lock, const char *lockName, const float *offset)
{

        offsetPose = new ObjectPoseState(NULL, offsetName);
        offsetPose->getAddress()->reparent(pose->getAddress());
        gz4d::Point<double> pos = pose->getPosition();
        offsetPose->setPosition(pos[X] + offset[X], pos[Y] + offset[Y], pos[Z] + offset[Z]);
        if (offsetName[1] == 'P') //GPS
                lock = new PosableStateLock(this, offsetPose, pose);
        else
                lock = new PosableStateLock(this, pose, offsetPose);
        lock->publishName(lockName);
        lock->setDeleteThreshold(0);
        lock->getAddress()->reparent(pose->getAddress());
        lock->lock(PosableStateLock::LockLocalized | PosableStateLock::LockDeltaTranslate | PosableStateLock::LockRotateY | PosableStateLock::LockFromOwner);
}

/*--------------------------------------------------------------------------*/
Object * BasicVessel::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    int i;
    const std::string partNames[] = {"bow_size", "hull_size", "stern_size", "bridge_size"};
    Point3d gr;

    gr[2] = 0.0;
    Point3f item_offset;
    double t = 0.0;
    OrientationInfo orient;
    float bridgeNudge = 0.0;
    float altitude;


    Object::LoadXML(xml,path, ncfile);

    boost::optional<std::string> attribute;

    for (i = BOW; i <= BRIDGE; i++)
    {
        if(attribute = xml.get_optional<std::string>("<xmlattr>." + partNames[i]))
        {
            std::stringstream ss(*attribute);
            ss >> width[i] >> length[i] >> height[i];
        }
    }

    
    bridgeNudge = xml.get("<xmlattr>.bridge_nudge",bridgeNudge);
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.gps_offset"))
    {
        std::stringstream ss(*attribute);
        ss >> item_offset[0] >> item_offset[1] >> item_offset[2];
        ESTABLISH_OFFSET(GPS, To, item_offset);
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.bridge_offset"))
    {
        std::stringstream ss(*attribute);
        ss >> item_offset[0] >> item_offset[1] >> item_offset[2];
        ESTABLISH_OFFSET(Bridge, From, item_offset);
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.stern_offset"))
    {
        std::stringstream ss(*attribute);
        ss >> item_offset[0] >> item_offset[1] >> item_offset[2];
        ESTABLISH_OFFSET(Stern, From, item_offset);
    }
    
    boost::optional<int> int_attrib;
    if(int_attrib = xml.get_optional<int>("<xmlattr>.telemetryPort"))
    {
        telemetrySocket = std::shared_ptr<gz4d::UDPSocket>(new gz4d::UDPSocket);
        telemetrySocket->SetNonBlocking();
        telemetrySocket->Bind(gz4d::IPSocket::Address("ANY",*int_attrib));
    }

    target_inner_size = xml.get("<xmlattr>.target_inner_size",target_inner_size);
    target_outer_size = xml.get("<xmlattr>.target_outer_size",target_outer_size);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.target_inner_color"))
    {
        std::stringstream ss(*attribute);
        ss >> target_inner_color[0] >> target_inner_color[1] >> target_inner_color[2];
    }

    if(attribute = xml.get_optional<std::string>("<xmlattr>.target_outer_color"))
    {
        std::stringstream ss(*attribute);
        ss >> target_outer_color[0] >> target_outer_color[1] >> target_outer_color[2];
    }
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.draw_tail"))
        drawTail = gz4d::IsTrue(*attribute);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.draw_cmg"))
        drawCMG = gz4d::IsTrue(*attribute);
    
    if(attribute = xml.get_optional<std::string>("<xmlattr>.show_depth"))
        showDepth = gz4d::IsTrue(*attribute);

    speedUnits = xml.get("<xmlattr>.speed_units",speedUnits);
        
    if(attribute = xml.get_optional<std::string>("<xmlattr>.projection"))
        if (*attribute == "Geographic")
        {
            projectionRequired = true;
        }
        else // assume UTM if not geo
        {
            int zone;
            if (sscanf(attribute->c_str(),"UTM %i",&zone))
            {
                //ProjectionSpec * gp = mainWindow->getScene3D()->getProjection();
                //if (gp)
            //{
                //if(gp->getZone() != zone)
                //{
                    //proj = this should maybe create a local projection to go to geo, then use scene proj to reproject to scene utm 
                    //}
                    //}
            }
        }

    calculateOffsets(bridgeNudge);

    ///\todo Positions in time could be implemented in a netcdf file for better loadtime and less wasted mem by loading only what is needed. An index of time would get loaded in memory

    orient.axis[Roll] = 0.0;
    orient.axis[Pitch] = 0.0;

    gz4d::geo::Projection proj;

    double timeOffset = xml.get<double>("<xmlattr>.timeOffset", 0.0);

    for(bpt::ptree::const_iterator n = xml.begin(); n != xml.end(); ++n)
        if(n->first == "Position")
        {
            hideOutsideTimeBounds = true;
            if(attribute = n->second.get_optional<std::string>("<xmlattr>.position"))
            {
                std::stringstream ss(*attribute);
                ss >> gr[0] >> gr[1] >> gr[2];
                if(projectionRequired)
                {
                        autoProjection(gr[X],gr[Y]);
                        if(!proj)
                            proj = gz4d::geo::Projection(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                        gz4d::Point<double> p = proj(gr[Y],gr[X]);
                        gr[X] = p[0];
                        gr[Y] = p[1];
                }
            }
            boost::optional<double> double_attrib;
            if(double_attrib = n->second.get_optional<double>("<xmlattr>.time"))
                t = *double_attrib+timeOffset;

            altitude = n->second.get<float>("<xmlattr>.altitude", -1.0);

            if(attribute = n->second.get_optional<std::string>("<xmlattr>.orientation"))
            {
                std::stringstream ss(*attribute);
                ss >> orient.axis[Roll] >> orient.axis[Pitch] >> orient.axis[Yaw];
            }
            else
            {
                if(double_attrib = n->second.get_optional<double>("<xmlattr>.heading"))
                    orient.axis[Yaw] = *double_attrib;
                else
                    orient.axis[Yaw] = RAD2DEG(atan2(gr[0]-lastPos[0],gr[1]-lastPos[1]));

                orient.axis[Pitch] = n->second.get("<xmlattr>.pitch",orient.axis[Pitch]);
                orient.axis[Roll] = n->second.get("<xmlattr>.roll",orient.axis[Roll]);
            }

            boost::optional<float> speed;
            if(speed = n->second.get_optional<float>("<xmlattr>.speed"))
                speeds[gz4d::Time(t)] = *speed;

            COPY_POINT(lastPos,gr);
            SetPositionOrientation(gr[0],gr[1],gr[2],orient,t,altitude);
            calculateCOG=0;
        }


    ObjectCollection::LoadXML(xml,path,ncfile);
    return this;
}

/*---------------------------------------------------------------------------*/
// documented in Object
int BasicVessel::readFrom(FILE *infile)
{
    //parse header
    if (Object::readFrom(infile))
        return 1;

        calculateOffsets();

        return 0; //returns 0 if ok.
}

/*---------------------------------------------------------------------------*/
/** This function renders the object in the passed window.  The degradation
*	parameter informs the object to render itself more or less quickly.  A
*	degradation of 0 indicates no degradation (high-res, for instance), while
*  a degradation of 1 indicates maximum degradation.  The Window3D parameter
*	can be used to determine information such as pixel-size-in-meters, etc.
*/
void BasicVessel::DrawLowRes(float degradation, Window3D *win)
{
        float scale;

        glPushAttrib(GL_LIGHTING_BIT|GL_POLYGON_BIT|GL_POINT_BIT);
        GLint df;
        glGetIntegerv(GL_DEPTH_FUNC,&df);
        glDepthFunc(GL_ALWAYS);
        glDisable(GL_LIGHTING);
        if (toggle)
        {
                glPointSize(target_outer_size*2);
                glBegin(GL_POINTS);
                        glColor3fv(target_outer_color);
                        glVertex3f(0.0,0.0,0.0);
                glEnd();
                glPointSize(target_inner_size*2);
                glBegin(GL_POINTS);
                        glColor3fv(target_inner_color);
                        glVertex3f(0.0,0.0,0.0);
                glEnd();
        }
        else
        {
                glPointSize(target_outer_size);
                glBegin(GL_POINTS);
                        glColor3fv(target_outer_color);
                        glVertex3f(0.0,0.0,0.0);
                glEnd();
                glPointSize(target_inner_size);
                glBegin(GL_POINTS);
                        glColor3fv(target_inner_color);
                        glVertex3f(0.0,0.0,0.0);
                glEnd();
        }
        glDepthFunc(df);
        glEnable(GL_LIGHTING);

        //First, draw shadow if it is desired
        if (shadowOver)
        {
                float h;
                h = shadowOver->GetHeightNear(pose->getPosition()[X], pose->getPosition()[Y], referenceLength);
                if (h > invalidHeight)
                {
                        h -= pose->getPosition()[Z];
                        glShadeModel(GL_FLAT);

                        //draw a flat ship
                        glDisable(GL_LIGHTING);        
                        glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
                        /*
                        glLineWidth(3.0);
                        glBegin(GL_LINES);
                                glNormal3f(1.0, 0.0, 0.0);
                                glVertex3f(0.0, 0.0, 0.0);
                                glVertex3f(0.0, 0.0, h);
                        glEnd();
                        glLineWidth(1.0);
                        */
                        glPushMatrix();
                        glScalef(0.5f, 1.0f, 1.0f);
                        glTranslatef(0.0f, -offset[HULL], h);
                                glBegin(GL_QUAD_STRIP); //my graphics card has trouble with GL_QUADS in line-mode (wireframe)
                                        glNormal3f(0.0, 0.0, 1.0);
                                        glVertex3f(-width[BOW], offset[BOW] + length[BOW], 0.0);
                                        glVertex3f(width[BOW], offset[BOW] + length[BOW], 0.0);
                                        glVertex3f(-width[HULL], offset[BOW], 0.0);
                                        glVertex3f(width[HULL], offset[BOW], 0.0);

                                        glVertex3f(-width[HULL], offset[HULL], 0.0);
                                        glVertex3f(width[HULL], offset[HULL], 0.0);

                                        glVertex3f(-width[STERN], 0.0, 0.0);
                                        glVertex3f(width[STERN], 0.0, 0.0);
                                        glVertex3f(-width[STERN], length[STERN], 0.0);
                                        glVertex3f(width[STERN], length[STERN], 0.0);
                                glEnd();
                                //(no front panel)
                        glPopMatrix();
                        glEnable(GL_LIGHTING);
                }
        }

        scale = 0.002f / win->getPoseInfo()->getScale(); //make the boat bigger in x & y if it would show up too small
        glPushMatrix(); //outer scale
        if (scale > 1.0 && MainWindow::getInterfaceType() == GeoNav3D)
                glScalef(scale, scale, 1.0);

        if(!toggle)
        {
            if(!IsEmpty() && drawChildren && (!onlyModelWin || (onlyModelWin == win)))
            {
                    ObjectCollection::DrawLowRes(degradation,win);
            }
            else
            {
                doneFlag = 0;
                float heightBow = height[HULL] - height[BOW];
                float heightStern = height[HULL] - height[STERN];
                float dWidthBowHull = (width[HULL] - width[BOW]) / width[HULL];
                float dHeightBowHull = (height[HULL] - heightBow) / height[HULL];
        //	float dWidthSternHull = (width[HULL] - width[STERN]) / width[HULL];
                float dHeightSternHull = (height[HULL] - heightStern) / height[HULL];

                if (drawOpaque)
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                glShadeModel(GL_FLAT);

                //draw the ship
                glEnable(GL_LIGHTING);        
                glColor3f(vesselColor[0], vesselColor[1], vesselColor[2]);
                glPushMatrix();
                glScalef(0.5, 1.0, 1.0);
                glTranslatef(0.0f, -offset[HULL], 0.0f);
                //  scale = 10 / sqrt(winScale);
                //create the bow
                glPushMatrix();
                        glTranslatef(0.0, offset[BOW], 0.0);
                        glBegin(GL_QUAD_STRIP); //my graphics card has trouble with GL_QUADS in line-mode (wireframe)
                                //right-hand panel
                                glNormal3f(0.0, 0.0, 1.0);
                                glVertex3f(-width[HULL], 0.0, height[HULL]);
                                glVertex3f(-width[BOW], length[BOW], height[HULL]);
                                glNormal3f(dWidthBowHull - 1.0, dWidthBowHull, 0.0);
                                glVertex3f(-width[HULL], 0.0, 0.0);
                                glVertex3f(-width[BOW], length[BOW], heightBow);
                                //bottom panel
                                glNormal3f(0.0, dHeightBowHull, dHeightBowHull - 1.0);
                                glVertex3f(width[HULL], 0.0, 0.0);
                                glVertex3f(width[BOW], length[BOW], heightBow);
                                //left-hand panel
                                glNormal3f(1.0 - dWidthBowHull, dWidthBowHull, 0.0);
                                glVertex3f(width[HULL], 0.0, height[HULL]);
                                glVertex3f(width[BOW], length[BOW], height[HULL]);
                                //top panel
                                glNormal3f(0.0, 0.0, 1.0);
                                glVertex3f(-width[HULL], 0.0, height[HULL]);
                                glVertex3f(-width[BOW], length[BOW], height[HULL]);
                        glEnd();
                                //front panel
                        glBegin(GL_QUAD_STRIP);
                                glNormal3f(0.0, 1.0, 0.0);
                                glVertex3f(-width[BOW], length[BOW], heightBow);
                                glVertex3f(width[BOW], length[BOW], heightBow);
                                glVertex3f(-width[BOW], length[BOW], height[HULL]);
                                glVertex3f(width[BOW], length[BOW], height[HULL]);
                        glEnd();
                        //optional center panel
                        if (drawOpaque)
                        {
                                glBegin(GL_QUAD_STRIP);
                                        glNormal3f(1.0, 0.0, 0.0);
                                        glVertex3f(0.0, 0.0, 0.0);
                                        glVertex3f(0.0, length[BOW], heightBow);
                                        glVertex3f(0.0, 0.0, height[HULL]);
                                        glVertex3f(0.0, length[BOW], height[HULL]);
                                glEnd();
                        }
                        //(no back panel)
                glPopMatrix();
                //create the hull
                glPushMatrix();
                        glTranslatef(0.0, offset[HULL], 0.0);
                        glBegin(GL_QUAD_STRIP);
                                //right-hand panel
                                glNormal3f(0.0, 0.0, -1.0);
                                glVertex3f(-width[HULL], 0.0, 0.0);
                                glVertex3f(-width[HULL], length[HULL], 0.0);
                                glNormal3f(-1.0, 0.0, 0.0);
                                glVertex3f(-width[HULL], 0.0, height[HULL]);
                                glVertex3f(-width[HULL], length[HULL], height[HULL]);
                                //top panel
                                glNormal3f(0.0, 0.0, 1.0);
                                glVertex3f(width[HULL], 0.0, height[HULL]);
                                glVertex3f(width[HULL], length[HULL], height[HULL]);
                                //left-hand panel
                                glNormal3f(1.0, 0.0, 0.0);
                                glVertex3f(width[HULL], 0.0, 0.0);
                                glVertex3f(width[HULL], length[HULL], 0.0);
                                //bottom panel
                                glNormal3f(0.0, 0.0, -1.0);
                                glVertex3f(-width[HULL], 0.0, 0.0);
                                glVertex3f(-width[HULL], length[HULL], 0.0);
                        glEnd();
                                //optional center panel
                        if (drawOpaque)
                        {
                                glBegin(GL_QUAD_STRIP);
                                        glNormal3f(1.0, 0.0, 0.0);
                                        glVertex3f(0.0, 0.0, 0.0);
                                        glVertex3f(0.0, 0.0, height[HULL]);
                                        glVertex3f(0.0, length[HULL], 0.0);
                                        glVertex3f(0.0, length[HULL], height[HULL]);
                                glEnd();
                        }
                        //(no front or back panel)
                glPopMatrix();
                //create the stern
                glPushMatrix();
                        glTranslatef(0.0, offset[STERN], 0.0);
                        glBegin(GL_QUAD_STRIP);
                                //right-hand panel
                                glNormal3f(dWidthBowHull - 1.0, -dWidthBowHull, 0.0);
                                glVertex3f(-width[HULL], length[STERN], height[HULL]);
                                glVertex3f(-width[STERN], 0.0, height[HULL]);
                                glVertex3f(-width[HULL], length[STERN], 0.0);
                                glVertex3f(-width[STERN], 0.0, heightStern);
                                //bottom panel
                                glNormal3f(0.0, -dHeightSternHull, dHeightBowHull - 1.0);
                                glVertex3f(width[HULL], length[STERN], 0.0);
                                glVertex3f(width[STERN], 0.0, heightStern);
                                //left-hand panel
                                glNormal3f(1.0 - dWidthBowHull, -dWidthBowHull, 0.0);
                                glVertex3f(width[HULL], length[STERN], height[HULL]);
                                glVertex3f(width[STERN], 0.0, height[HULL]);
                                //top panel
                                glNormal3f(0.0, 0.0, 1.0);
                                glVertex3f(-width[HULL], length[STERN], height[HULL]);
                                glVertex3f(-width[STERN], 0.0, height[HULL]);
                        glEnd();
                        glBegin(GL_QUAD_STRIP);
                                //back panel
                                glNormal3f(0.0, -1.0, 0.0);
                                glVertex3f(-width[STERN], 0.0, height[HULL]);
                                glVertex3f(width[STERN], 0.0, height[HULL]);
                                glVertex3f(-width[STERN], 0.0, heightStern);
                                glVertex3f(width[STERN], 0.0, heightStern);
                        glEnd();
                        //optional center panel
                        if (drawOpaque)
                        {
                                glBegin(GL_QUAD_STRIP);
                                        glNormal3f(1.0, 0.0, 0.0);
                                        glVertex3f(0.0, length[STERN], height[HULL]);
                                        glVertex3f(0.0, 0.0, height[HULL]);
                                        glVertex3f(0.0, length[STERN], 0.0);
                                        glVertex3f(0.0, 0.0, heightStern);
                                glEnd();
                        }
                        //(no front panel)
                glPopMatrix();
                //create the bridge
                glPushMatrix();
                        glTranslatef(0.0, offset[BRIDGE], height[HULL]);
                        glBegin(GL_QUAD_STRIP);
                                //right-hand panel
                                glNormal3f(0.0, -1.0, 0.0);
                                glVertex3f(-width[BRIDGE], 0.0, 0.0);
                                glVertex3f(-width[BRIDGE], 0.0, height[BRIDGE]);
                                glNormal3f(-1.0, 0.0, 0.0);
                                glVertex3f(-width[BRIDGE], length[BRIDGE], 0.0);
                                glVertex3f(-width[BRIDGE], length[BRIDGE], height[BRIDGE]);
                                //front panel
                                glNormal3f(0.0, 1.0, 0.0);
                                glVertex3f(width[BRIDGE], length[BRIDGE], 0.0);
                                glVertex3f(width[BRIDGE], length[BRIDGE], height[BRIDGE]);
                                //left-hand panel
                                glNormal3f(1.0, 0.0, 0.0);
                                glVertex3f(width[BRIDGE], 0.0, 0.0);
                                glVertex3f(width[BRIDGE], 0.0, height[BRIDGE]);
                                //back panel
                                glNormal3f(0.0, -1.0, 0.0);
                                glVertex3f(-width[BRIDGE], 0.0, 0.0);
                                glVertex3f(-width[BRIDGE], 0.0, height[BRIDGE]);
                        glEnd();
                                //top panel
                        glBegin(GL_QUAD_STRIP);
                                glNormal3f(0.0, 0.0, 1.0);
                                glVertex3f(-width[BRIDGE], length[BRIDGE], height[BRIDGE]);
                                glVertex3f(width[BRIDGE], length[BRIDGE], height[BRIDGE]);
                                glVertex3f(-width[BRIDGE], 0.0, height[BRIDGE]);
                                glVertex3f(width[BRIDGE], 0.0, height[BRIDGE]);
                        glEnd();
                        //optional center panel
                        if (drawOpaque)
                        {
                                glBegin(GL_QUAD_STRIP);
                                        glNormal3f(1.0, 0.0, 0.0);
                                        glVertex3f(0.0, 0.0, 0.0);
                                        glVertex3f(0.0, 0.0, height[BRIDGE]);
                                        glVertex3f(0.0, length[BRIDGE], 0.0);
                                        glVertex3f(0.0, length[BRIDGE], height[BRIDGE]);
                                glEnd();
                        }
                        //(no bottom panel)
                glPopMatrix();
                glPopMatrix();
                glPopMatrix(); //outer scale

                //glDisable(GL_LINE_SMOOTH);
                glPushMatrix();
                glLoadIdentity();
                        //draw the tail
                        if (drawTail)
                                tail.DrawIn(win);

                        if (drawCMG)
                        {
                                //draw the CMG line
                                glDisable(GL_LIGHTING);
                                glLineWidth(2);
                                //glPointSize(4);
                                glPushMatrix();
                                courseMadeGood->applyTransforms();
                                glColor3f(0.2f, 0.2f, 0.2f);
                                //glBegin(GL_POINTS);
                                //	glVertex3f(0.0, 0.0, 0.0);
                                //glEnd();
                                //glPointSize(10);
                                //glColor3f(0.5f, 0.8f, 0.7f);
                                //glBegin(GL_POINTS);
                        //		glVertex3f(0.0, 0.0, 0.0);
                        //	glEnd();
                                glBegin(GL_LINES);
                                        glVertex3f(0.0, 0.0, 0.0);
                                        glVertex3f(0.0, 360.0, 0.0);
                                glEnd();
                                glLineWidth(4);
                                glColor3f(0.8f, 0.8f, 0.8f);
                                glBegin(GL_LINES);
                                        glVertex3f(0.0, 0.0, 0.0);
                                        glVertex3f(0.0, 360.0, 0.0);
                                glEnd();
                                glPopMatrix();
                        //	glPointSize(1);
                                glLineWidth(1);
                        }
            }
            glPopMatrix();
        }

        gz4d::Point<double> pos = pose->getPosition();
        double x,y;
        gz4d::Point<double> wpos = win->getViewInfo()->getPosable()->getPosition();
        x = pos[X] - wpos[X];
        y = pos[Y] - wpos[Y];

        if (win->getCursorPosOnPlane(x, y, pos[Z])) //see if the cursor is nearby.  If so, show label.
        {
            x -= pos[X];
            y -= pos[Y];
            if ((x * x + y * y) < (900.0 * scale * scale)) // are we within 30m of the ship?
            {
                glColor4f(1.0,1.0,1.0,0.7);
                win->printAt(0.0, 0.0, pos[Z] + 3.0, aisName);
            }
        }

        std::stringstream label_ss;
        if(showDepth)
        {
            if (poseHistory)
            {
                OrientationInfo o;
                gz4d::Point<double> p;
                double alt;
                poseHistory->getPositionOrientation(lastIdleTime,p,o,alt);
                label_ss << std::setprecision(4) << p[2] << " m";
            }
            else
                label_ss << std::setprecision(4) << GetPosition(2) << " m";
        }
        if(!speeds.empty())
        {
            if(showDepth)
                label_ss << ", ";
            double toffset = 0.0;
            if(poseHistory)
                toffset = poseHistory->getTimeOffset();
            label_ss << std::setprecision(4) << speeds.GetLatest(gz4d::Time(lastIdleTime+toffset))->second << " " << speedUnits;
        }
        if(!label_ss.str().empty())
        {
            glPushAttrib(GL_DEPTH_BUFFER_BIT|GL_LIGHTING_BIT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glColor3f(1.0,1.0,1.0);
            win->printAt(0.0, 0.0, 0.0, label_ss.str(), true, GLUT_BITMAP_HELVETICA_18,10,5);
            glPopAttrib();
        }

        glPopAttrib();
}

/*---------------------------------------------------------------------------*/
int BasicVessel::DrawHighRes(int eye, Window3D *win)
{
        DrawLowRes(0, win);

    if (eye == dominantEye || eye == MONO_EYE) //go ahead and set the doneFlag if this will be our last time through
            doneFlag = 1;

        return 1;
}

/*---------------------------------------------------------------------------*/
void BasicVessel::DrawAsFlatIcon(Window3D *win, float opaqueness, GLboolean resizeWithScale)
{
        float scale;

        if (resizeWithScale)
        {
                scale = 0.002f / win->getPoseInfo()->getScale(); //make the boat bigger in x & y if it would show up too small
                glPushMatrix(); //outer scale
                if (scale > 1.0)
                        glScalef(scale, scale, 1.0);
        }

        //float dWidthBowHull = (width[HULL] - width[BOW]) / width[HULL];

        glShadeModel(GL_FLAT);

        //draw the ship
        glDisable(GL_LIGHTING);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(vesselColor[0], vesselColor[1], vesselColor[2], opaqueness);
        glPushMatrix();
        glScalef(0.5, 1.0, 1.0);
        glTranslatef(0.0f, -offset[HULL], 0.0f);
        glPushMatrix();
                glBegin(GL_QUAD_STRIP); //my graphics card has trouble with GL_QUADS in line-mode (wireframe)
                        glNormal3f(0.0, 0.0, 1.0);
                        glVertex3f(-width[BOW], offset[BOW] + length[BOW], 0.0);
                        glVertex3f(width[BOW], offset[BOW] + length[BOW], 0.0);
                        glVertex3f(-width[HULL], offset[BOW], 0.0);
                        glVertex3f(width[HULL], offset[BOW], 0.0);

                        glVertex3f(-width[HULL], offset[HULL], 0.0);
                        glVertex3f(width[HULL], offset[HULL], 0.0);

                        glVertex3f(-width[STERN], 0.0, 0.0);
                        glVertex3f(width[STERN], 0.0, 0.0);
                glEnd();
                //(no front panel)
        glPopMatrix();
        glPopMatrix();
        if (resizeWithScale)
                glPopMatrix(); //outer scale

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
}


/*---------------------------------------------------------------------------*/
int BasicVessel::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    char retString[2048];
    bool badSyntax;
    int returnval;

    if (argc >= 1)
    {
        std::string message = argv[0];
        if (message == "truncateTail")
        {
            if (argc < 2)
            {
                Tcl_AppendResult(interp, "usage: truncateTail <index>", NULL);
                return TCL_ERROR;
            }
            tail.truncateTailAt(atoi(argv[1]));
            return TCL_OK;
        }
        if (message == "opaque")
        {
            if (argc >= 2)
                drawOpaque = atoi(argv[1]);
            sprintf(retString, "%d", drawOpaque);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "callsign")
        {
            if (argc == 2)
                callsign = argv[1];
            Tcl_SetResult(interp, const_cast<char *>(callsign.c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "aisName")
        {
            if (argc == 2)
                aisName = argv[1];
            Tcl_SetResult(interp, const_cast<char *>(aisName.c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "destination")
        {
            if (argc == 2)
                destination = argv[1];
            Tcl_SetResult(interp, const_cast<char *>(destination.c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "cogPitch")
        {
            if (argc >= 2)
                cogPitch = atoi(argv[1]);
            sprintf(retString, "%d", cogPitch);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "drawCMG")
        {
            if (argc >= 2)
                drawCMG = atoi(argv[1]);
            sprintf(retString, "%d", drawCMG);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "drawTail")
        {
            if (argc >= 2)
                drawTail = isTrue(argv[1]);
            sprintf(retString, "%d", drawTail);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "color")
        {
            if (argc >= 2)
                vesselColor[0] = atof(argv[1]);
            if (argc >= 3)
                vesselColor[1] = atof(argv[2]);
            if (argc >= 4)
                vesselColor[2] = atof(argv[3]);
            sprintf(retString, "%f %f %f", vesselColor[0],vesselColor[1],vesselColor[2]);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "toggle")
        {
            if (argc >= 2)
            {	
                toggle = atoi(argv[1]);
            }
            sprintf(retString, "%d", toggle);
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
        if (message == "modelWindows")
        {
            Window3D *win = NULL;
            Addressable *address = NULL;

            badSyntax = GL_TRUE;
            if (argc >= 2)
            {
                if (!strcmp(argv[1], "all"))
                {
                    setModelWindow(NULL);
                    return TCL_OK;
                }
                if (argc >= 3)
                {
                    if (!strcmp(argv[1], "only"))
                    {
                        returnval = gzCommandInterface::findWindow(argv[2], address, win);
                        if (returnval != TCL_OK)
                            return returnval;
                        setModelWindow(win);
                        return TCL_OK;
                    }
                }
            }
            if (badSyntax)
            {
                Tcl_AppendResult(interp, "Usage: renderWindows {all | only <window>}", NULL);
                return TCL_ERROR;
            }

        }
        if (message == "shadowOver")
        {
            Object *ob = NULL;
            Addressable *address = NULL;

            if (argc >= 2)
            {
                if (argv[1][0] == '-')
                        shadowOver = NULL;
                else
                {
                    returnval = gzCommandInterface::findObject(argv[1], address, ob);
                    if (returnval != TCL_OK)
                            return returnval;
                    shadowOver = ob;
                }
            }
            sprintf(retString, "%s", shadowOver? shadowOver->getAddress()->getAddress().c_str() : "-");
            Tcl_SetResult(interp, (char *) retString, TCL_VOLATILE);
            return TCL_OK;
        }
    }
    return ObjectCollection::perform(argc, argv, interp);
}

/*---------------------------------------------------------------------------*/
int BasicVessel::list(Tcl_Interp *interp)
{
        Tcl_AppendResult(interp, "truncateTail opaque cogPitch drawTail drawCMG color toggle modelWindows shadowOver ", NULL);
        return Object::list(interp);
}

/*---------------------------------------------------------------------------*/
void BasicVessel::setModelWindow(Window3D *win)
{
    if (win != onlyModelWin)
    {
        onlyModelWin = win;
        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void BasicVessel::PoseChanged(PosableStateBase *whichPose, long whatChanged)
{
    gz4d::Point<double> newPosition;
    gz4d::Point<double> newPos(0.0f, 0.0f, 0.0f);
        double scale;

        if ((whatChanged & ChangeTranslateMask) && pose == whichPose)
        {
                newPosition = pose->getPosition();
                if (cogPitch)
                        tail.newPosition(newPosition);
                else
                {
                        newPos[X] = newPosition[X];
                        newPos[Y] = newPosition[Y];
                        tail.newPosition(newPos);
                }
                if (generatingCMG)
                {
                        scale = pose->getScale();
                        courseMadeGood->AccumulateNotifications();
                        courseMadeGood->setScale(scale * scale * tail.getAverageCourse(3, courseMadeGood)); // was 10, courseMadeGood, changed by mdp 2/18/2004
                        courseMadeGood->ResumeSingularNotification();
                }
                else if (generatingSOG)
                        courseMadeGood->setScale(tail.getAverageCourse(3, NULL));
        }
        Object::PoseChanged(whichPose, whatChanged);
}

/*---------------------------------------------------------------------------*/
void BasicVessel::calculateOffsets(float bridge_nudge)
{
    gz4d::Point<double> pos;

    offset[STERN] = 0.0f;
    offset[HULL] = offset[STERN] + length[STERN];
    offset[BOW] = length[HULL] + offset[HULL];
    offset[BRIDGE] = offset[BOW] / 4.0f + bridge_nudge;
    pos = pose->getPosition();
    courseMadeGood->setPosition(pos[X], pos[Y] + length[HULL] + length[BOW], pos[Z]);
    lockFromCMG->lock(PosableStateLock::LockLocalized | PosableStateLock::LockDeltaTranslate | PosableStateLock::LockRotateY | PosableStateLock::LockFromOwner, courseMadeGood);
    referenceLength = length[HULL] + length[BOW];
}

/*---------------------------------------------------------------------------*/
void BasicVessel::establishGPSoffset(float *offset_amount)
{
        Point3f item_offset;

        ESTABLISH_OFFSET(GPS, To, offset_amount);
        item_offset[X] = 0;
        item_offset[Y] = offset[BRIDGE] + length[BRIDGE] + 6.0f;
        item_offset[Z] = height[HULL] + height[BRIDGE] * 0.5f;
        ESTABLISH_OFFSET(Bridge, From, item_offset);
        item_offset[Y] = -length[STERN] * 2.0f;
        item_offset[Z] = 0;
        ESTABLISH_OFFSET(Stern, From, item_offset);
}

/*---------------------------------------------------------------------------*/
void BasicVessel::setSpeed(float speed)
{
        courseMadeGood->setScale(speed * 10.0f);
        generatingSOG = GL_FALSE;
}

/*---------------------------------------------------------------------------*/
void BasicVessel::setCourse(float heading, GLboolean andHeadingToo)
{
        if (andHeadingToo)
                pose->setOrientation(0.0f, heading);
        courseMadeGood->setOrientation(0.0f, heading);
}

/*---------------------------------------------------------------------------*/
// void BasicVessel::setStringVal(char *&string, const char *val)
// {
//         int slen, vlen;
// 
//         if (!val || (string && (strcmp(string, val) == 0)))
//                 return; //don't do anything if they match or if val is NULL
// 
//         //if val is smaller than string, just copy it in.  Otherwise, reallocate.
//         vlen = strlen(val);
//         if (string)
//                 slen = strlen(string);
//         else
//                 slen = -1;
// 
//         if (vlen > slen)
//         {
//                 delete string;
//                 string = new char[vlen + 1];
//         }
//         strcpy(string, val);
// }

/*---------------------------------------------------------------------------*/
/**	Display info about this object in the info window on the GUI.
*/
void BasicVessel::ShowInfo(unsigned int *names, int numNames)
{
        char infoLine[512];
        Scene3D *scene;

        scene = mainWindow->getScene3D();

        sprintf(infoLine, "Type:\t%s\n", getRegistrationEntry()->className.c_str());
        scene->appendInfo(infoLine);
        if (!aisName.empty())
        {
                sprintf(infoLine, "AIS Nam:%s\n", aisName.c_str());
                scene->appendInfo(infoLine);
                sprintf(infoLine, "MMSI:\t%s\n", GetLabel().c_str());
                scene->appendInfo(infoLine);
        }
        else
        {
                sprintf(infoLine, "Name:\t%s\n", GetLabel().c_str());
                scene->appendInfo(infoLine);
        }
        if (!callsign.empty())
        {
                sprintf(infoLine, "Callsgn:%s\n", callsign.c_str());
                scene->appendInfo(infoLine);
        }
        if (!destination.empty())
        {
                sprintf(infoLine, "Dest.:\t%s\n", destination.c_str());
                scene->appendInfo(infoLine);
        }

        sprintf(infoLine, "SOG:\t%f\n", courseMadeGood->getScale());
        scene->appendInfo(infoLine);
        sprintf(infoLine, "COG:\t%f\n", courseMadeGood->getOrientationAxis()[Yaw]);
        scene->appendInfo(infoLine);

        if (!GetFilename().empty())
        {
                sprintf(infoLine, "File:\t%s\n", GetFilename().c_str());
                scene->appendInfo(infoLine);
        }

        sprintf(infoLine, "Last Pos:%f, %f, %f\n", GetPosition(X), GetPosition(Y), GetPosition(Z));
        scene->appendInfo(infoLine);
        scene->appendInfo("--------------------\n");
}

void BasicVessel::Idle( double time)
{

    if(telemetrySocket)
    {
        std::string data(1500,0);
        gz4d::IPSocket::Address a;
        try
        {
            telemetrySocket->RecvFrom(data,a);
            bpt::ptree doc;
            std::istringstream datass(data);
            read_xml(datass,doc);

            for(bpt::ptree::const_iterator n = doc.begin(); n != doc.end(); ++n)
                if(n->first == "Position")
                {
                    hideOutsideTimeBounds = true;
                    Point3d gr;
                    boost::optional<std::string> attribute;
                    if(attribute = n->second.get_optional<std::string>("<xmlattr>.position"))
                    {
                        std::stringstream ss(*attribute);
                        ss >> gr[0] >> gr[1] >> gr[2];
                    }
                    if(projectionRequired)
                    {
                        autoProjection(gr[X],gr[Y]);
                        gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
                        gz4d::Point<double> p = proj(gr[Y],gr[X]);
                        gr[X] = p[0];
                        gr[Y] = p[1];
                    }
        
                    double t = n->second.get<double>("<xmlattr>.time",0.0);
        
                    float altitude = n->second.get<float>("<xmlattr>.altitude",-1.0);
        
                    OrientationInfo orient;
                    if(attribute = n->second.get_optional<std::string>("<xmlattr>.orientation"))
                    {
                        std::stringstream ss(*attribute);
                        ss >> orient.axis[Roll] >> orient.axis[Pitch] >> orient.axis[Yaw];
                    }
                    else
                    {
                        orient.axis[Yaw] = RAD2DEG(atan2(gr[0]-lastPos[0],gr[1]-lastPos[1]));
                        COPY_POINT(lastPos,gr);
                    }
                    SetPositionOrientation(gr[0],gr[1],gr[2],orient,t,altitude);
                    calculateCOG=0;
                }
        }
        catch(gz4d::Socket::WouldBlockException){}
    }
    ObjectCollection::Idle( time);
}
