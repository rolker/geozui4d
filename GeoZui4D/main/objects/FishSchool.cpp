/* Copyright (c) 2006 Data Visualization Research Lab,
//                    Center for Coastal and Ocean Mapping,
//                    University of New Hampshire.  All rights reserved.
//                    http://www.ccom.unh.edu/vislab
// Not to be copied or distributed without written agreement.
// Contact:  colinw@cisunix.unh.edu or roland@ccom.unh.edu
*/
/*---------------------------------------------------------------------------*/
// FishSchool.h
/*---------------------------------------------------------------------------*/

/** 
 * This class represents a collection of sonar targets.  I have named this
 * class FishSchool because its main purpose is to display fish.  In the
 * future some of the functionality in this class may be extracted to a base
 * class.
 *
 *	Created by blocke June 9, 2006
 */

#include "FishSchool.h"

#include <iostream>
#include <limits.h>
#include "objects/NetCDF.h"
#include <stdlib.h>
#include<gz4d/glo.h>
#include <GL/glut.h>

#include <filesystem>
#include "objects/gutm.h"
#include "engine/InputMediator.h"
#include "engine/Scene3D.h"
#include "utilities/Utilities.h"
#include "MainWindow.h"
#include "engine/TextureMaps/TextureMap.h"
#include "engine/TimeControl.h"
#include <gz4d/geo/Projection.h>

extern MainWindow *mainWindow;


REGISTER_OBJECT(FishSchool, "fishschool");

/**
 * This is the default time window length (s).
 */
#ifdef DBL_MAX
    static const double DEFAULT_TIME_WINDOW = DBL_MAX;
#else
    static const double DEFAULT_TIME_WINDOW = 100000.0;
#endif

/**
 * Default Constructor.
 */
FishSchool::FishSchool(void)
{
    init();
}

FishSchool::FishSchool(std::string const &fname)
:fishFilename(fname)
{
    init();
    initFishData();
}

void FishSchool::init()
{
    fishBeginIndex = 0;
    fishLimitIndex = 0;
    //baseTime = 0;
    idleTime = 0;
    timeWindow = DEFAULT_TIME_WINDOW;
    baseZone = 0;
    baseEasting = 0;
    baseNorthing = 0;
    enableLighting = true;
    useTextureColor = false;
    usePoly = false;
    baseSize = 1.0;
    sizeScale = 0.0;
    needListNames = true;
    displayLists = 0;
    listsNeedBuild = true;
    //listOffsets = NULL;
    extraFormat = COLOR;
    //socket.SetNonBlocking();
}

/**
 * Destructor.
 */
FishSchool::~FishSchool(void)
{
    // Delete the fish array.
/*    if (fishArray != NULL)
    {
            delete fishArray;
    }*/
    //delete[] listOffsets;
}

/**
 * @see Object
 * The NetCDF file that contains the fish must be specified in the XML file,
 * using the Object's XML attribute 'targetfile'.  The time window can also
 * be specified using the attribute 'timewindow'.
 */
Object * FishSchool::LoadXML(bpt::ptree const &xml, std::string const &path, gz4d::netcdf::File ncfile)
{
    Object::LoadXML(xml, path, ncfile);

    boost::optional<std::string> attribute;
    if(attribute = xml.get_optional<std::string>("<xmlattr>.targetfile"))
        fishFilename = fs::path(path)/(*attribute);

    timeWindow = xml.get("<xmlattr>.timewindow",timeWindow);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.lighting"))
        enableLighting = gz4d::IsTrue(*attribute);

    if(attribute = xml.get_optional<std::string>("<xmlattr>.extraFormat"))
        if (*attribute == "normal")
            extraFormat = NORMAL;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.color"))
        if (*attribute == "clut")
            useTextureColor = true;

    if(attribute = xml.get_optional<std::string>("<xmlattr>.usePoly"))
        usePoly = gz4d::IsTrue(*attribute);

    baseSize = xml.get("<xmlattr>.baseSize",baseSize);
    sizeScale = xml.get("<xmlattr>.sizeScale",sizeScale);

    if (timeWindow <= 0)
    {
        timeWindow = DEFAULT_TIME_WINDOW;
    }

    initFishData();
    return this;
}

/**
 * @see Object
 */
void FishSchool::DrawLowRes(float degradation, Window3D *win)
{
    glPushAttrib(GL_POINT_BIT|GL_LIGHTING_BIT);

    if (!enableLighting)
        glDisable(GL_LIGHTING);

    glutSolidCube(1);
    glPointSize(1.0);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
    glDisable(GL_COLOR_MATERIAL);
    float black[4] = {0.0,0.0,0.0,1.0};
     float white[4] = {1.0,1.0,1.0,1.0};
     //glLightModelfv(GL_LIGHT_MODEL_AMBIENT,black);
     glMaterialfv(GL_BACK,GL_AMBIENT_AND_DIFFUSE,black);
     glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,white);
     //glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

    std::shared_ptr<TextureMap> texture;
    if(useTextureColor)
    {
        texture = GetCurrentTexture();
        glColor3f(1.0,1.0,1.0);
        EnableTexture(TEXTURE_COORD_NONE);
    }
    else if(extraFormat == NORMAL)
    {
        //glColor3f(1.0,1.0,1.0);
        EnableTexture();
    }


    // The fish (targets) that fall within the specified time window are
    // displayed.
    GLfloat size = baseSize;
    if(usePoly)
    {
        if (needListNames)
        {
            displayLists = glGenLists(fish.size());
            if (displayLists != 0)
            {
                //listOffsets = new GLuint[fishCount];
                listOffsets.resize(fish.size());
                for(GLuint i = 0; i < (GLuint)(fish.size()); i++)
                    listOffsets[i] = i+displayLists;
            }
            needListNames = false;
        }
        if(listsNeedBuild && displayLists != 0)
        {
            for (unsigned long i=0; i<fish.size(); i++)
            {
                glNewList(displayLists+i,GL_COMPILE);
                if(useTextureColor)
                    texture->SetHeight(fish[i].size);
                else if (extraFormat == COLOR)
                {
                    glColor3ubv(fish[i].extra);
                    glPushMatrix();
                    glTranslatef(fish[i].position[X],fish[i].position[Y],fish[i].position[Z]);
                    glutSolidSphere(baseSize+sizeScale*fish[i].size,3,3);
                    glPopMatrix();
                }
                else
                {
                    glPushMatrix();
                    glTranslatef(fish[i].position[X],fish[i].position[Y],fish[i].position[Z]);
                    glRotatef( acos(fish[i].extra[2]/127.0)*180.0/M_PI,-fish[i].extra[1]/127.0,fish[i].extra[0]/127.0,0.0);
                    glTranslatef(-fish[i].position[X],-fish[i].position[Y],-fish[i].position[Z]);
                    glNormal3f(0.0,0.0,1.0);
                    glBegin(GL_QUADS);
                    size = (baseSize+sizeScale*fish[i].size)/2.0;
                    glVertex3f(fish[i].position[X]-size,fish[i].position[Y]+size,fish[i].position[Z]);
                    glVertex3f(fish[i].position[X]-size,fish[i].position[Y]-size,fish[i].position[Z]);
                    glVertex3f(fish[i].position[X]+size,fish[i].position[Y]-size,fish[i].position[Z]);
                    glVertex3f(fish[i].position[X]+size,fish[i].position[Y]+size,fish[i].position[Z]);
                    glEnd();
                    glPopMatrix();
                }
                glEndList();
            }
            listsNeedBuild = false;
        }
    }
    else
    {
        glPointSize(baseSize);
        glBegin(GL_POINTS);
    }
    if(usePoly && displayLists != 0 && degradation < 0.5)
    {
        if (fishLimitIndex > fishBeginIndex)
            glCallLists(fishLimitIndex-fishBeginIndex,GL_UNSIGNED_INT,&listOffsets[fishBeginIndex]);
    }
    else
    {
        for (unsigned long i=fishBeginIndex; i<fishLimitIndex; i++)
        {
            if(sizeScale != 0.0)
            {
                if(usePoly)
                {
                    size = baseSize+sizeScale*fish[i].size;
                }
                else
                {
                    glPointSize(baseSize+sizeScale*fish[i].size);
                }
            }
            if(useTextureColor)
                texture->SetHeight(fish[i].size);
            else if (extraFormat == COLOR)
            {
                glColor3ubv(fish[i].extra);
            }
            else
            {
                glNormal3bv((GLbyte*)(fish[i].extra));
            }
            if(usePoly)
            {
                if (extraFormat == NORMAL)
                {
                    glPushMatrix();
                    glTranslatef(fish[i].position[X],fish[i].position[Y],fish[i].position[Z]);
                    glRotatef( acos(fish[i].extra[2]/127.0)*180.0/M_PI,-fish[i].extra[1]/127.0,fish[i].extra[0]/127.0,0.0);
                    glTranslatef(-fish[i].position[X],-fish[i].position[Y],-fish[i].position[Z]);
                    glNormal3f(0.0,0.0,1.0);
                    glBegin(GL_QUADS);
                    size = (baseSize+sizeScale*fish[i].size)/2.0;
                    glVertex3f(fish[i].position[X]-size,fish[i].position[Y]+size,fish[i].position[Z]);
                    glVertex3f(fish[i].position[X]-size,fish[i].position[Y]-size,fish[i].position[Z]);
                    glVertex3f(fish[i].position[X]+size,fish[i].position[Y]-size,fish[i].position[Z]);
                    glVertex3f(fish[i].position[X]+size,fish[i].position[Y]+size,fish[i].position[Z]);
                    glEnd();
                    glPopMatrix();
                }
                else
                {
                    glPushMatrix();
                    glTranslatef(fish[i].position[X],fish[i].position[Y],fish[i].position[Z]);
                    glutSolidSphere(size,5,5);
                    glPopMatrix();
                }
            }
            else
                glVertex3fv(fish[i].position);
        }
    }
    if(!usePoly)
        glEnd();

    if(useTextureColor || extraFormat == NORMAL)
        DisableTexture();

    if (!enableLighting)
        glEnable(GL_LIGHTING);

    glPopAttrib();
}

void FishSchool::HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data)
{
    gz4d::binary::Sonar::Channel::TargetDetection *t = reinterpret_cast<gz4d::binary::Sonar::Channel::TargetDetection*>(&data->front());
    if(t->IsValid())
    {
        gz4d::Point<float> p;
        p[0] = t->x;
        p[1] = t->y;
        p[2] = t->z;
        AddFish(t->latitude,t->longitude,p,t->timestamp,*std::max_element(t->samples.values,t->samples.values+t->samples.size));
    }
    else
    {
        Object::HandlePacket(data);
    }

}

/**
 * @see Object
 */
void FishSchool::Idle(double time)
{
    Object::Idle( time);

    if (fish.empty())
        return;

    double localTime = (gz4d::Time(time)-baseTime).asSeconds();

    unsigned long originalBeginIndex(fishBeginIndex);
    unsigned long originalLimitIndex(fishLimitIndex);

    // Adjust beginning index into fish array
    double lowerLimitTime = localTime - timeWindow;

    // short circuit when time loops
    if (fish[0].time > lowerLimitTime)
        fishBeginIndex = 0;

    if (fish[fish.size()-1].time < lowerLimitTime)
        fishBeginIndex = fish.size()-1;

    if (fish[0].time > localTime)
        fishLimitIndex = 0;

    if (fish[fish.size()-1].time < localTime)
        fishLimitIndex = fish.size()-1;

    // Move forward, if we should and if we can
    while (fish[fishBeginIndex].time < lowerLimitTime && fishBeginIndex < fish.size()-1 && fish[fishBeginIndex+1].time < lowerLimitTime)
        fishBeginIndex++;

    // Move backwards if necessary
    while (fish[fishBeginIndex].time > lowerLimitTime && fishBeginIndex > 0)
        fishBeginIndex--;

    // As we did for the begining time, adjust end time
    // Move forward, if we should and if we can
    while (fish[fishLimitIndex].time < localTime && fishLimitIndex < fish.size()-1 && fish[fishLimitIndex+1].time < localTime)
        fishLimitIndex++;

    // Move backwards if necessary
    while (fish[fishLimitIndex].time > localTime && fishLimitIndex > 0)
        fishLimitIndex--;

    if ((originalLimitIndex != fishLimitIndex) || (originalBeginIndex != fishBeginIndex))
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());

}

/**
 * This method initializes the fish data variables from a NetCDF file.
 * Presently, all of thetarget data in the NetCDF file is read into an array.
 * As you might imagine, this strategy will become unweildy as the number of
 * targets grow.  So, this will have to be reworked.
 */
void FishSchool::initFishData()
{
    if (fishFilename.extension().string() == ".nc")
    {
        // Create and check the netCDF file object.
        gz4d::netcdf::File fishNcFile(fishFilename.string());
        if(fishNcFile)
        {
            gz4d::netcdf::Dim countd = fishNcFile.GetDim("count");
            gz4d::netcdf::Dim targetd = fishNcFile.GetDim("target");
            gz4d::netcdf::Dim timed = fishNcFile.GetDim("time");
            gz4d::netcdf::Dim positiond = fishNcFile.GetDim("position");

            if(countd && targetd && timed && positiond)
            {
                long fishCount = countd.Size();
                fishLimitIndex = 0;
                long targetSize = targetd.Size();

                // Check that the target size is what we expect.
                if (targetSize == sizeof(fish[0]))
                {
                    // Load the targets into the fish array.
                    fish.resize(fishCount);
                    gz4d::netcdf::Var targetVar = fishNcFile.GetVar("targets");
                    std::vector<long> cur(2,0);
                    targetVar.SetCur(cur);

                    // Load the base time and positions.
                    gz4d::netcdf::Var baseTimeVar = fishNcFile.GetVar("baseTime");
                    gz4d::netcdf::Var baseUtmZoneVar = fishNcFile.GetVar("baseUtmZone");
                    gz4d::netcdf::Var baseUtmEastingVar = fishNcFile.GetVar("baseUtmEasting");
                    gz4d::netcdf::Var baseUtmNorthingVar = fishNcFile.GetVar("baseUtmNorthing");

                    double t;
                    try
                    {
                        std::vector<long> counts(2);
                        counts[0] = fishCount;
                        counts[1] = targetSize;
                        targetVar.Get(reinterpret_cast<int8_t*>(&fish.front()), fishCount, targetSize);
                        counts.resize(1);
                        counts[0]=1;
                        baseTimeVar.Get(&t, 1);
                        baseUtmZoneVar.Get(&baseZone, 1);
                        baseUtmEastingVar.Get(&baseEasting, 1);
                        baseUtmNorthingVar.Get(&baseNorthing, 1);
                        // Set the object's position.
                        // We are assuming that the UTM zone is okay, which
                        // makes me uncomfortable.
                        baseTime = gz4d::Time(t);
                        SetPosition(baseEasting, baseNorthing, 0.0);
                        bbox.start = baseTime.secondsSinceEpoch().asSeconds();
                        bbox.end = (baseTime+gz4d::time::Duration(fish[fishCount-1].time)).secondsSinceEpoch().asSeconds();
                    }
                    catch (gz4d::netcdf::Exception)
                    {
                        fish.clear();
                        std::cerr << "Error loading fish from netcdf file." << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Target dimension (" << targetSize;
                    std::cerr << ") does not match size of fish (";
                    std::cerr << sizeof(fish[0]) << ")." << std::endl;
                }
            }
            else
            {
                std::cerr << "Error finding dimensions in netcdf file." << std::endl;
            }
        }
        else
        {
            std::cerr << "Error opening netcdf file" << std::endl;
        }
    }
}

void FishSchool::AddFish(double lat, double lon, gz4d::Point<float> const &p, gz4d::Time t, float value)
{

    autoProjection(lon,lat);
    gz4d::geo::Projection proj(spatialReference,mainWindow->getScene3D()->getSpatialReference());
    gz4d::Point<double> position = proj(lat,lon);
    position[0] += p[0];
    position[1] += p[1];
    position[2] = p[2];

    AddFish(position[0],position[1],position[2],t,255,255,255,255);
}

void FishSchool::AddFish(double e, double n, float z, gz4d::Time t, unsigned char e1, unsigned char e2, unsigned char e3, unsigned char size)
{
    if (fish.empty())
    {
        baseTime = t;
        baseEasting = e;
        baseNorthing = n;
        SetPosition(baseEasting, baseNorthing, 0.0);
        bbox.start = baseTime.secondsSinceEpoch().asSeconds();
    }
    Fish f;
    f.position[0] = e - baseEasting;
    f.position[1] = n - baseNorthing;
    f.position[2] = z;
    f.time = (t - baseTime).asSeconds();
    f.extra[0] = e1;
    f.extra[1] = e2;
    f.extra[2] = e3;
    f.size = size;
    fish.push_back(f);
    bbox.end = t.secondsSinceEpoch().asSeconds();

    if(!mainWindow->getScene3D()->timeControl->isPaused())
        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
}

/**
 * Allows an instance of FishSchool to respond to tcl gz commands.
 *
 */
int FishSchool::perform(int argc, const char** argv, Tcl_Interp* interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command ==  "add")
        {
            if(argc != 8)
            {
                Tcl_SetResult(interp, const_cast<char*>("usage: add lat long x y z time value"), TCL_VOLATILE);
                return TCL_ERROR;
            }
            double lat = boost::lexical_cast<double>(argv[1]);
            double lon = boost::lexical_cast<double>(argv[2]);
            float x = boost::lexical_cast<float>(argv[3]);
            float y = boost::lexical_cast<float>(argv[4]);
            float z = boost::lexical_cast<float>(argv[5]);
            double t = boost::lexical_cast<double>(argv[6]);
            float v = boost::lexical_cast<float>(argv[7]);
            AddFish(lat,lon,gz4d::Point<float>(x,y,z),gz4d::Time(t),v);
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            return TCL_OK;
        }
        if (command ==  "timewindow")
        {
            std::stringstream retString;
            switch (argc)
            {
                case 1:
                    retString << timeWindow;
                    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                case 2:
                    timeWindow = atof(argv[1]);
                    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    return TCL_OK;
                default:
                    Tcl_SetResult(interp, const_cast<char*>("Query or set time interval of displayed data.\nusage: gz timewindow [time]"), TCL_VOLATILE);
                    return TCL_ERROR;
            }
        }
        if (command ==  "baseSize")
        {
            std::stringstream retString;
            switch (argc)
            {
                case 1:
                    retString << baseSize;
                    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                case 2:
                    baseSize = atof(argv[1]);
                    listsNeedBuild = true;
                    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    return TCL_OK;
                default:
                    Tcl_SetResult(interp, const_cast<char*>("Query or set base size of displayed data.\nusage: gz tellObject foo baseSize [size]"), TCL_VOLATILE);
                    return TCL_ERROR;
            }
        }
        if (command ==  "sizeScale")
        {
            std::stringstream retString;
            switch (argc)
            {
                case 1:
                    retString << sizeScale;
                    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                case 2:
                    sizeScale = atof(argv[1]);
                    listsNeedBuild = true;
                    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    return TCL_OK;
                default:
                    Tcl_SetResult(interp, const_cast<char*>("Query or set scale applied to data's size attribute.\nusage: gz tellObject foo sizeScale [scale]"), TCL_VOLATILE);
                    return TCL_ERROR;
            }
        }
        if (command ==  "usePoly")
        {
            std::stringstream retString;
            switch (argc)
            {
                case 1:
                    retString << usePoly;
                    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                case 2:
                    usePoly = isTrue(std::string(argv[1]));
                    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    return TCL_OK;
                default:
                    Tcl_SetResult(interp, const_cast<char*>("Query or set flag indicating if points or polygons are used for rendering.\nusage: gz tellObject foo usePoly [true/false]"), TCL_VOLATILE);
                    return TCL_ERROR;
            }
        }
        if (command ==  "extraFormat")
        {
            std::stringstream retString;
            switch (argc)
            {
                case 1:
                    switch(extraFormat)
                    {
                        case COLOR:
                            retString << "color";
                            break;
                        case NORMAL:
                            retString << "normal";
                            break;
                        default:
                            retString << "invalid";
                    }
                    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                case 2:
                    if (std::string(argv[1])=="normal")
                        extraFormat = NORMAL;
                    else
                        extraFormat = COLOR;
                    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    return TCL_OK;
                default:
                    Tcl_SetResult(interp, const_cast<char*>("Query or set extra information format.\nusage: gz tellObject foo extraFormat [color|normal]"), TCL_VOLATILE);
                    return TCL_ERROR;
            }
        }
        if (command ==  "lighting")
        {
            std::stringstream retString;
            switch (argc)
            {
                case 1:
                    retString << enableLighting;
                    Tcl_SetResult(interp, (char *)(retString.str().c_str()), TCL_VOLATILE);
                    return TCL_OK;
                case 2:
                    enableLighting = isTrue(std::string(argv[1]));
                    listsNeedBuild = true;
                    globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    return TCL_OK;
                default:
                    Tcl_SetResult(interp, const_cast<char*>("Query or set flag indicating if lighting and shading is used for rendering.\nusage: gz tellObject foo lighting [true/false]"), TCL_VOLATILE);
                    return TCL_ERROR;
            }
        }
    }
    return Object::perform(argc, argv, interp);
}

void FishSchool::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">FishSchool</h" << level << ">";

    if (enableLighting)
        html << "Lighting is on, turn <a href=\"/command/gz/tellObject/" << GetId() << "/lighting/off\">off</a><br>";
    else
        html << "Lightin is off, turn <a href=\"/command/gz/tellObject/" << GetId() << "/lighting/on\">on</a><br>";

    if (usePoly)
        html << "Polygons are on, turn <a href=\"/command/gz/tellObject/" << GetId() << "/usePoly/off\">off</a><br>";
    else
        html << "Polygons are off, turn <a href=\"/command/gz/tellObject/" << GetId() << "/usePoly/on\">on</a><br>";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/timewindow/tValue\">Time window:<input type=\"text\" name=\"tValue\" value=\"" << timeWindow <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/baseSize/value\">Base size:<input type=\"text\" name=\"value\" value=\"" << baseSize <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/sizeScale/value\">Size scale:<input type=\"text\" name=\"value\" value=\"" << sizeScale <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    Object::getHtmlInfo(html,level);
}
