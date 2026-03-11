// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "Sonar.h"
#include "engine/ObjectPoseState.h"
#include "engine/Scene3D.h"
#include "MainWindow.h"
#include "engine/TimeControl.h"
#include <gz4d/geo/Projection.h>
#include <gz4d/glo/Translate.h>
#include <gz4d/glo/Vertex.h>
#include <gz4d/glo/PushAttrib.h>
#include "utilities/Utilities.h"
#include "engine/TextureMaps/TextureMap.h"
#include <gz4d/time/ios.h>

extern MainWindow *mainWindow;

REGISTER_OBJECT(Sonar, "gz4d");

#define SONAR_RASTER_W 512
#define SONAR_RASTER_H 512

Sonar::Sonar()
    :renderFan(true),renderCurtains(true),positionSet(false),offsetsNeedUpdate(false),minDB(-80.0),maxDB(-33.0),alphaLimit(0.15),maxPingGap(10.0)
{
    binaryFile = true;
    projectionRequired = true;
    transparencyLevel = 1;

}

void Sonar::HandlePacket(std::shared_ptr<gz4d::Socket::Packet> &data)
{
    gz4d::binary::Wrapper<gz4d::binary::Sonar::Channel::Samples> cs(data);
    if(cs->IsValid())
    {
        int rasterNumber = 0;
        int rasterIndex = 0;
        if(!channels[cs->channelID].empty())
        {
            rasterNumber = channels[cs->channelID].rbegin()->second.rasterNumber;
            rasterIndex = channels[cs->channelID].rbegin()->second.rasterIndex + 1;
            if (rasterIndex == SONAR_RASTER_W)
            {
                rasterNumber += 1;
                rasterIndex = 0;
            }
        }
        channels[cs->channelID][cs->timestamp].samples = cs;
        channels[cs->channelID][cs->timestamp].rasterNumber = rasterNumber;
        channels[cs->channelID][cs->timestamp].rasterIndex = rasterIndex;
        if(rasterIndex == 0)
        {
            channelDetails[cs->channelID].rasters.push_back(gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> >());
            //std::shared_ptr<gz4d::glo::Texture> t(new gz4d::glo::Texture);
            //channelDetails[cs->channelID].textures.push_back(t);
            channelDetails[cs->channelID].rasterUpdated.push_back(true);
            channelDetails[cs->channelID].rasters.back().Resize(SONAR_RASTER_W,SONAR_RASTER_H);
        }
        int sampleSkip = 1;
        while(cs->samples.size/sampleSkip > SONAR_RASTER_H)
            ++sampleSkip;
        for(int i = 0; i < cs->samples.size/sampleSkip; ++i)
        {
            double value = 0.0;
            for (int j = 0; j < sampleSkip; ++j)
            {
                if (i*sampleSkip+j < cs->samples.size)
                    value += cs->samples.values[i*sampleSkip+j];
            }
            value /= (double)sampleSkip;
            int cIndex = ((value-minDB)/(maxDB-minDB))*12;
            gz4d::Pixel<gz4d::RGBA8> p;
            if(cIndex < 0)
            {
                p[0] = 0;
                p[1] = 0;
                p[2] = 0;
                p[3] = 0;
            }
            else
                switch(cIndex)
                {
                    case 0:
                        p[0] = 156;
                        p[1] = 138;
                        p[2] = 173;
                        p[3] = 128;
                        break;
                    case 1:
                        p[0] = 74;
                        p[1] = 73;
                        p[2] = 74;
                        p[3] = 138;
                        break;
                    case 2:
                        p[0] = 0;
                        p[1] = 109;
                        p[2] = 255;
                        p[3] = 148;
                        break;
                    case 3:
                        p[0] = 33;
                        p[1] = 12;
                        p[2] = 173;
                        p[3] = 158;
                        break;
                    case 4:
                        p[0] = 41;
                        p[1] = 211;
                        p[2] = 123;
                        p[3] = 168;
                        break;
                    case 5:
                        p[0] = 0;
                        p[1] = 134;
                        p[2] = 99;
                        p[3] = 178;
                        break;
                    case 6:
                        p[0] = 255;
                        p[1] = 255;
                        p[2] = 41;
                        p[3] = 188;
                        break;
                    case 7:
                        p[0] = 255;
                        p[1] = 121;
                        p[2] = 49;
                        p[3] = 198;
                        break;
                    case 8:
                        p[0] = 255;
                        p[1] = 89;
                        p[2] = 173;
                        p[3] = 208;
                        break;
                    case 9:
                        p[0] = 255;
                        p[1] = 24;
                        p[2] = 49;
                        p[3] = 218;
                        break;
                    case 10:
                        p[0] = 181;
                        p[1] = 60;
                        p[2] = 49;
                        p[3] = 228;
                        break;
                    default:
                        p[0] = 148;
                        p[1] = 40;
                        p[2] = 57;
                        p[3] = 255;
                    }
            channelDetails[cs->channelID].rasters[rasterNumber].GetRow(i)[rasterIndex]=p;
            channelDetails[cs->channelID].rasterUpdated[rasterNumber] = true;
        }
        channels[cs->channelID][cs->timestamp].textureCoord = double(cs->samples.size/sampleSkip)/double(SONAR_RASTER_H);
        expandBBoxTime(cs->timestamp.secondsSinceEpoch().asSeconds());
        offsetsNeedUpdate = true;
        return;
    }

    gz4d::binary::Wrapper<gz4d::binary::Navigation::Position6DOF::Geographic> nav(data);
    if(nav->IsValid())
    {
        navigation.Consume(nav);
        expandBBoxTime(nav->timestamp.secondsSinceEpoch().asSeconds());
        if(!positionSet)
        {
            gz4d::nav::Fix::Ptr fix = navigation(nav->timestamp);
            if (!gz4d::IsNan(fix->latitude) && !gz4d::IsNan(fix->longitude))
            {
                SetLatLon(fix->latitude.value(),fix->longitude.value());
                positionSet = true;
            }
        }
        offsetsNeedUpdate = true;
        return;
    }

    Object::HandlePacket(data);
}

void Sonar::Idle(double time)
{
    if(offsetsNeedUpdate)
        UpdateOffsets();
    
    if(time != lastIdleTime  && visible && renderFan)
    {
        bool needUpdate = false;
    
        for(std::map<int,PingSeq>::iterator c = channels.begin(); c != channels.end() && !needUpdate; ++c)
        {
            PingSeq::const_iterator s = c->second.GetLatest(gz4d::Time(time));
            if(s != c->second.end())
                if(s->first != channelDetails[c->first].renderTime)
                    needUpdate = true;
        }
        if(needUpdate)
            globalInputMediator->Trigger(Scene3D::objectChangedEvent());
    }
    Object::Idle(time);

}

void Sonar::UpdateOffsets()
{
    if(positionSet)
    {
        gz4d::ogr::SpatialReference wgs84;
        wgs84.SetWellKnownGeogCS("WGS84");
        gz4d::geo::Projection proj(wgs84, mainWindow->getScene3D()->getSpatialReference());

        for(std::map<int,PingSeq>::iterator c = channels.begin(); c != channels.end(); ++c)
        {
            for(PingSeq::const_iterator s = c->second.begin(); s != c->second.end(); ++s)
            {
                if(!offsets.count(s->first))
                {
                    gz4d::nav::Fix::Ptr fix = navigation(s->first);
                    if(!gz4d::IsNan(fix->latitude) && !gz4d::IsNan(fix->longitude))
                    {
                        offsets[s->first].position = proj(*fix) - pose->getPosition();

                        float heading = 0.0;
                        if(!gz4d::IsNan(fix->heading))
                            heading = fix->heading.value();
                        offsets[s->first].heading = heading;
                    }
                }
                if(offsets.count(s->first) && !offsets[s->first].channel.count(c->first))
                {
                    float heading = offsets[s->first].heading;
                    gz4d::Point<double> diff;
                    diff[0] = sin(gz4d::Radians(s->second.samples->geometry.direction.x))*cos(gz4d::Radians(heading))+sin(gz4d::Radians(s->second.samples->geometry.direction.y))*sin(gz4d::Radians(heading));
                    diff[1] = sin(gz4d::Radians(s->second.samples->geometry.direction.y))*cos(gz4d::Radians(heading))-sin(gz4d::Radians(s->second.samples->geometry.direction.x))*sin(gz4d::Radians(heading));
                    diff[2] = -sqrt(1.0-(diff[0]*diff[0]+diff[1]*diff[1]));
                    offsets[s->first].channel[c->first].diff = diff;
                    offsets[s->first].channel[c->first].start = diff * s->second.samples->offset*1500.0/2.0;
                    offsets[s->first].channel[c->first].step = s->second.samples->sampleInterval*1500.0/2.0;
                }
            }
        }
    }
    offsetsNeedUpdate = false;
}

void Sonar::DrawLowRes(float degradation, Window3D *win)
{
    gz4d::glo::PushAttrib attrib(GL_ALL_ATTRIB_BITS);

    EnableTexture(TEXTURE_COORD_NONE);
    std::shared_ptr<TextureMap> currentTexture;
    if (!customtextureptr)
        currentTexture = *defaulttextureptr;
    else
        currentTexture = customtextureptr;

    glDisable(GL_LIGHTING);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, alphaLimit);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);

    for(std::map<int,PingSeq>::iterator c = channels.begin(); c != channels.end(); ++c)
    {
        while(channelDetails[c->first].rasters.size()>channelDetails[c->first].textures.size())
        {
            std::shared_ptr<gz4d::glo::Texture> t(new gz4d::glo::Texture(win->getGloContext()));
            channelDetails[c->first].textures.push_back(t);
        }        
        
        if(renderFan && channelDetails[c->first].visible)
        {
            PingSeq::const_iterator s = c->second.GetLatest(gz4d::Time(lastIdleTime));
            if(s != c->second.end())
            {
                channelDetails[c->first].renderTime = s->first;
                glPushMatrix();
                gz4d::TimeSequence<Offset>::const_iterator o = offsets.GetLatest(gz4d::Time(lastIdleTime));
                if(o != offsets.end())
                {
                    gz4d::glo::Translate(o->second.position);
                    glRotatef(o->second.heading,0.0,0.0,-1.0);
                }

                glRotatef(s->second.samples->geometry.direction.y,1.0,0.0,0.0);
                glRotatef(s->second.samples->geometry.direction.x,0.0,-1.0,0.0);
                if(s->first+ gz4d::seconds(6.0) > gz4d::Time(lastIdleTime))
                {
                    float tfactor = 1.0;
                    if(s->first+gz4d::seconds(1.0) < gz4d::Time(lastIdleTime))
                        tfactor = 1.0-(lastIdleTime - (s->first.secondsSinceEpoch().asSeconds()+1.0))/5.0;
                    glBegin(GL_QUAD_STRIP);
                    float dx = sin(gz4d::Radians(s->second.samples->geometry.width.x/2.0));
                    //int sampleSkip = std::max(1,channelDetails[c->first].sampleSkip*skipMultiplier);
                    for(unsigned int i = 0; i < s->second.samples->samples.size; ++i)//i+= sampleSkip)
                    {
                        float z = (s->second.samples->offset+i*s->second.samples->sampleInterval)*1500.0/2.0;
                        glColor4f(1.0,1.0,1.0,tfactor);
                        currentTexture->SetHeight(s->second.samples->samples.values[i]);
                        glVertex3f(-dx*z,0.0,-z);
                        glVertex3f(dx*z,0.0,-z);
                    }
                    glEnd();
                    glBegin(GL_QUAD_STRIP);
                    float dy = sin(gz4d::Radians(s->second.samples->geometry.width.y/2.0));
                    for(unsigned int i = 0; i < s->second.samples->samples.size; ++i)//i+=sampleSkip)
                    {
                        float z = (s->second.samples->offset+i*s->second.samples->sampleInterval)*1500.0/2.0;
                        glColor4f(1.0,1.0,1.0,tfactor);
                        currentTexture->SetHeight(s->second.samples->samples.values[i]);
                        glVertex3f(0.0,dy*z,-z);
                        glVertex3f(0.0,-dy*z,-z);
                    }
                    glEnd();
                }
                glPopMatrix();
            }
        }
    }

    glColor3f(1.0,1.0,1.0);
    DisableTexture();

    if(renderCurtains)
    {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_LIGHTING);
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GEQUAL, alphaLimit);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL);

        for(std::map<int,PingSeq>::iterator c = channels.begin(); c != channels.end(); ++c)
        {
            int activeTexture = -1;
            if(channelDetails[c->first].visible && gz4d::Time(lastIdleTime) >= c->second.begin()->first)
            {
                PingSeq::const_reverse_iterator s = PingSeq::const_reverse_iterator(c->second.GetLatest(gz4d::Time(lastIdleTime)));
                PingSeq::const_reverse_iterator last = PingSeq::const_reverse_iterator(c->second.GetLatest(std::max(gz4d::Time(lastIdleTime-channelDetails[c->first].renderHistory),c->second.begin()->first)));
                glBegin(GL_QUAD_STRIP);
    
                while(s!= c->second.rend())
                {
                    if(offsets.count(s->first))
                    {
                        if(s->second.rasterNumber != activeTexture)
                        {
                            glEnd();
                            if(!*channelDetails[c->first].textures[s->second.rasterNumber])
                            {
                                channelDetails[c->first].textures[s->second.rasterNumber]->Bind();
                                channelDetails[c->first].textures[s->second.rasterNumber]->Image(channelDetails[c->first].rasters[s->second.rasterNumber]);
                                channelDetails[c->first].textures[s->second.rasterNumber]->MinFilter(GL_LINEAR);
                                channelDetails[c->first].textures[s->second.rasterNumber]->MagFilter(GL_NEAREST);
                                channelDetails[c->first].rasterUpdated[s->second.rasterNumber]=false;
                            }
                            else
                            {
                                channelDetails[c->first].textures[s->second.rasterNumber]->Bind();
                                if(channelDetails[c->first].rasterUpdated[s->second.rasterNumber])
                                {
                                    channelDetails[c->first].textures[s->second.rasterNumber]->SubImage(channelDetails[c->first].rasters[s->second.rasterNumber]);
                                    channelDetails[c->first].rasterUpdated[s->second.rasterNumber]=false;
                                }
                            }

                            activeTexture = s->second.rasterNumber;
                            glBegin(GL_QUAD_STRIP);

                        }
                        gz4d::Point<double> a = offsets[s->first].position+offsets[s->first].channel[c->first].start;
                        glTexCoord2f(s->second.rasterIndex/float(SONAR_RASTER_W),0.0);
                        gz4d::glo::Vertex(a);
                        a +=  offsets[s->first].channel[c->first].diff*(offsets[s->first].channel[c->first].step*s->second.samples->samples.size);
                        glTexCoord2f(s->second.rasterIndex/float(SONAR_RASTER_W),s->second.textureCoord);
                        gz4d::glo::Vertex(a);
    
                    }
                    if(s == last)
                        break;
                    PingSeq::const_reverse_iterator next = s;
                    ++next;
                    if(next != c->second.rend() && s->first-next->first > maxPingGap)
                    {
                        glEnd();
                        glBegin(GL_QUAD_STRIP);
                    }
                    s = next;
                }
                glEnd();
            }
        }
        glDisable(GL_TEXTURE_2D);
    }
}

int Sonar::Load()
{
    int ret = Open();
    if (!ret)
    {
        while (true)
        {
            char header[5];
            if(fread(header,1,4,in)<4)
                break;
            header[4] = 0;
            if(std::string(header) != "gz4d")
                break;
            uint8_t version[2];
            if(fread(version,1,2,in)<2)
                break;
            uint64_t size;
            if(fread(&size,sizeof(size),1,in)<1)
                break;
            fseek(in,-(6+sizeof(size)),SEEK_CUR);
            std::shared_ptr<std::vector<char> > data(new std::vector<char>(size));
            if(fread(&data->front(),1,size,in) < size)
                break;
            HandlePacket(data);
        }
    }
    return ret;
}

int Sonar::perform(int argc, const char** argv, Tcl_Interp* interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command ==  "minDB")
        {
            if(argc == 2)
            {
                minDB = atof(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(minDB).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command ==  "maxDB")
        {
            if(argc == 2)
            {
                maxDB = atof(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(maxDB).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command ==  "alphaLimit")
        {
            if(argc == 2)
            {
                alphaLimit = atof(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(alphaLimit).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command ==  "maxPingGap")
        {
            if(argc == 2)
            {
                maxPingGap = gz4d::time::Duration(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(maxPingGap).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command ==  "fan")
        {
            if(argc == 2)
            {
                renderFan = isTrue(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(renderFan).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command ==  "curtains")
        {
            if(argc == 2)
            {
                renderCurtains = isTrue(argv[1]);
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
            }
            Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(renderCurtains).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command ==  "channel")
        {
            if (argc > 2)
            {
                int channelNumber = atoi(argv[1]);
                std::string channelCmd(argv[2]);
                if(channelCmd == "visible")
                {
                    if(argc == 4)
                    {
                        channelDetails[channelNumber].visible = isTrue(argv[3]);
                        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    }
                    Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(channelDetails[channelNumber].visible).c_str()), TCL_VOLATILE);
                    return TCL_OK;
                }
                if(channelCmd == "renderHistory")
                {
                    if(argc == 4)
                    {
                        channelDetails[channelNumber].renderHistory = atof(argv[3]);
                        globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                    }
                    Tcl_SetResult(interp, (char *)(boost::lexical_cast<std::string>(channelDetails[channelNumber].renderHistory).c_str()), TCL_VOLATILE);
                    return TCL_OK;
                }
            }
        }
    }
    return Object::perform(argc, argv, interp);
}

void Sonar::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">Sonar</h" << level << ">";
    if (renderFan)
        html << "Fan visible, <a href=\"/command/gz/tellObject/" << GetId() << "/fan/off\">(set invisible)</a><br>";
    else
        html << "Fan invisible, <a href=\"/command/gz/tellObject/" << GetId() << "/fan/on\">(set visible)</a><br>";

    if (renderCurtains)
        html << "Curtains visible, <a href=\"/command/gz/tellObject/" << GetId() << "/curtains/off\">(set invisible)</a><br>";
    else
        html << "Curtains invisible, <a href=\"/command/gz/tellObject/" << GetId() << "/curtains/on\">(set visible)</a><br>";
    
    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/minDB/value\">Minimum dB:<input type=\"text\" name=\"value\" value=\"" << minDB <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";
    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/maxDB/value\">Maximum dB:<input type=\"text\" name=\"value\" value=\"" << maxDB <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";
    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/alphaLimit/value\">Alpha limit (0.0-1.0):<input type=\"text\" name=\"value\" value=\"" << alphaLimit <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";
    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/maxPingGap/value\">Maximum gap between pings (s):<input type=\"text\" name=\"value\" value=\"" << maxPingGap <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    for(std::map<int,PingSeq>::iterator c = channels.begin(); c != channels.end(); ++c)
    {
        html << "<h" << level+1 << ">Channel " << c->first << "</h" << level + 1 << ">";
        if (channelDetails[c->first].visible)
            html << "Visible, <a href=\"/command/gz/tellObject/" << GetId() << "/channel/" << c->first << "/visible/off\">(set invisible)</a><br>";
        else
            html << "Invisible, <a href=\"/command/gz/tellObject/" << GetId() << "/channel/" << c->first << "/visible/on\">(set visible)</a><br>";
        html << "<form action=\"/command/gz/tellObject/" << GetId() << "/channel/" << c->first << "/renderHistory/value\">Render history (s):<input type=\"text\" name=\"value\" value=\"" << channelDetails[c->first].renderHistory <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";
    }
    Object::getHtmlInfo(html,level);
}
