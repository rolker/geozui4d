// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "EK60_Channel.h"
#include "EK60.h"
#include "engine/Scene3D.h"
#include <gz4d/vector/ios.h>
#include <gz4d/time/ios.h>
#include "engine/Window3D.h"

REGISTER_OBJECT(EK60_Channel, "");

#define EK60_RASTER_W 2048
#define EK60_RASTER_H 2048
#define EK60_DB_FACTOR 0.01175898420562426544

EK60_Channel::EK60_Channel(EK60 * ek60)
    :lastRaster(999999),ek60(ek60),displaySpan(180),fadeDelay(0.6),showFan(true),showRibbon(true)
{
    transparencyLevel = 1;
    if(ek60 && ek60->IsME70())
        showRibbon = false;
}

EK60_Channel::EK60_Channel(EK60 * ek60, std::string const &name)
    :lastRaster(999999),ek60(ek60),displaySpan(180),fadeDelay(0.6),showFan(true),showRibbon(true)
{
    transparencyLevel = 1;
    SetLabel(name);
    if(ek60 && ek60->IsME70())
        showRibbon = false;

}

EK60_Channel::EK60_Channel(EK60 * ek60, gz4d::Simrad::ConfigurationBeam const &config)
    :lastRaster(999999),ek60(ek60),displaySpan(180),fadeDelay(0.6),showFan(true),showRibbon(true)
{
    transparencyLevel = 1;
    SetLabel(config.ChannelId);
    if(ek60 && ek60->IsME70())
        showRibbon = false;

    f = config.Frequency;

    beamWidthX = config.BeamWidthAthwartship;
    beamWidthY = config.BeamWidthAlongship;
    beamDirectionX = config.BeamSteeringAngleAthwartship;
    beamDirectionY = config.BeamSteeringAngleAlongship;

    startAngle_sin = sin(M_PI*(beamDirectionX - (beamWidthX/2.0))/180.0);
    startAngle_cos = cos(M_PI*(beamDirectionX - (beamWidthX/2.0))/180.0);
    endAngle_sin = sin(M_PI*(beamDirectionX + (beamWidthX/2.0))/180.0);
    endAngle_cos = cos(M_PI*(beamDirectionX + (beamWidthX/2.0))/180.0);
    midAngle_sin = sin(M_PI*(beamDirectionX)/180.0);
    midAngle_cos = cos(M_PI*(beamDirectionX)/180.0);

    y = pow(10.0,config.EquivalentBeamAngle/10.0);
    G0 = config.Gain;
    if (G0 == 0.0)
    {
        for(int i = 0; i < 5; ++i)
        {
            gainTable[config.PulseLengthTable[i]].gain = pow(10.0,config.GainTable[i]/10.0);
            gainTable[config.PulseLengthTable[i]].SaCorrection = config.SaCorrectionTable[i];
        }
    }
    else
        G0 = pow(10.0,G0/10.0);
}

void EK60_Channel::DrawLowRes(float degradation, Window3D *win)
{
    if(pings.empty())
        return;

    if(pings.begin()->first > displayTime || pings.rbegin()->first < displayTime - displaySpan)
        return;
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_ALWAYS, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);

    glDisable(GL_LIGHTING);
    glColor3f(1.0,1.0,1.0);

    glEnable(GL_TEXTURE_2D);



    if(!texture && !rasters.empty())
    {
        texture = std::shared_ptr<gz4d::glo::Texture>(new gz4d::glo::Texture(win->getGloContext()));
        texture->Bind();
        texture->MinFilter(GL_LINEAR);
        texture->MagFilter(GL_LINEAR);
        texture->Image(rasters[0]);
        lastRaster = 0;
    }
    else
        texture->Bind();

    gz4d::time::Duration adjustedDisplaySpan = (displaySpan);// - degradation*0.5);
    gz4d::time::Duration startFade = adjustedDisplaySpan*fadeDelay;

    if(showFan)
    {
        std::map<gz4d::Time,Ping>::iterator p = pings.upper_bound(displayTime);
        float x,y,heading;
        if (p != pings.end() && ek60->GetPosition(p->first,x,y) && ek60->GetHeading(p->first,heading))
        {
            glPushMatrix();
            glTranslatef(x,y,0.0);
            glRotatef(heading,0.0,0.0,-1.0);

            unsigned int rasterNo = p->second.number/EK60_RASTER_W;
            if (rasterNo != lastRaster)
            {
                texture->SubImage(rasters[rasterNo]);
                lastRaster = rasterNo;
            }
            double u = p->second.number%EK60_RASTER_W/double(EK60_RASTER_W);
            glBegin(GL_QUADS);

            float dxs1 = -p->second.z1*startAngle_sin;
            float dzs1 = p->second.z1*startAngle_cos;
            float dxe1 = -p->second.z1*endAngle_sin;
            float dze1 = p->second.z1*endAngle_cos;
 
            float dxs2 = -p->second.z2*startAngle_sin;
            float dzs2 = p->second.z2*startAngle_cos;
            float dxe2 = -p->second.z2*endAngle_sin;
            float dze2 = p->second.z2*endAngle_cos;

            glTexCoord2d(u,0.0);
            glVertex3f(dxe1,0.0,dze1);
            glVertex3f(dxs1,0.0,dzs1);
            glTexCoord2d(u,1.0);
            glVertex3f(dxs2,0.0,dzs2);
            glVertex3f(dxe2,0.0,dze2);

            glEnd();
            glPopMatrix();

        }
    }
    if(showRibbon)
    {
        glBegin(GL_QUAD_STRIP);
        for(std::map<gz4d::Time,Ping>::iterator p = pings.begin(); p != pings.end();++p)
        {
            float x,y,heading;
            if(p->first > (displayTime - adjustedDisplaySpan) && p->first <= displayTime && ek60->GetPosition(p->first,x,y) && ek60->GetHeading(p->first,heading))
            {
                if(p->first < (displayTime - startFade))
                {
                    float a = (displayTime - startFade - p->first).asSeconds()/(adjustedDisplaySpan- startFade).asSeconds();
                    a = baseColor[3]*(1.0-a);
                    glColor4f(baseColor[0],baseColor[1],baseColor[2],a);
                }
                else
                    glColor4f(1.0,1.0,1.0,1.0);
    
                unsigned int rasterNo = p->second.number/EK60_RASTER_W;
    
                if (rasterNo != lastRaster)
                {
                    if (lastRaster != 999999)
                    {
                        glTexCoord2f(1.0,0.0);
                        glVertex3f(x,y,p->second.z1);
                        glTexCoord2f(1.0,1.0);
                        glVertex3f(x,y,p->second.z2);
                    }
                    glEnd();
                    texture->SubImage(rasters[rasterNo]);
                    lastRaster = rasterNo;
                    glBegin(GL_QUAD_STRIP);
                }

                float heading_cos = cos(M_PI*heading/180.0);
                float heading_sin = sin(M_PI*heading/180.0);

                float dx1 = -p->second.z1*midAngle_sin*heading_cos;
                float dy1 = p->second.z1*midAngle_sin*heading_sin;
                float dz1 = p->second.z1*midAngle_cos;

                float dx2 = -p->second.z2*midAngle_sin*heading_cos;
                float dy2 = p->second.z2*midAngle_sin*heading_sin;
                float dz2 = p->second.z2*midAngle_cos;

                double u = p->second.number%EK60_RASTER_W/double(EK60_RASTER_W);
                glTexCoord2d(u,0.0);
                glVertex3f(x+dx1,y+dy1,dz1);
                glTexCoord2d(u,1.0);
                glVertex3f(x+dx2,y+dy2,dz2);
            }
        }
        glEnd();
    }
    texture->Unbind();
    glDisable(GL_TEXTURE_2D);

    glPopAttrib();
}

void EK60_Channel::AddPing(gz4d::Simrad::SampleDatagram const &ping)
{
    Ping p;
    float T = ping.SampleInterval*ping.SoundVelocity/2.0; // Sample thickness
    p.z1 = -T*ping.Offset;
    p.z2 = p.z1-EK60_RASTER_H*T;

    gz4d::Time t = gz4d::time::FromNT(ping.DgHeader.DateTime);
    p.number = pings.size();
    pings[t] = p;

    unsigned int rasterNo = p.number/EK60_RASTER_W;
    if (rasters.size() <= rasterNo)
    {
        rasters.push_back(gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> >());
        rasters.back().Resize(EK60_RASTER_W,EK60_RASTER_H);
    }
    int col = p.number%EK60_RASTER_W;

    float Pt = ping.TransmitPower;
    float a = ping.AbsorptionCoefficient;
    float tau = ping.PulseLength;

    float G0_2 = G0*G0;
    float Sa = 0.0;

    if(!gainTable.empty())
    {
        G0_2 = gainTable[tau].gain*gainTable[tau].gain;
        Sa = gainTable[tau].SaCorrection;
    }
    float c = ping.SoundVelocity;
    float lambda_2 = c/f;
    lambda_2 *= lambda_2;
    float pi2x32 = 32.0*M_PI*M_PI;
    float g2lambda2ctauy = G0_2*lambda_2*c*tau*y;

    for(int i = 0; i < EK60_RASTER_H; ++i)
    {
        gz4d::Color color;
        if (i < ping.Count)
        {
            float Pr = ping.Power[i]*EK60_DB_FACTOR;
            float R = (i+ping.Offset- 2)*T;
            float Sv = Pr + 20.0*log10(R) + 2.0*a*R - 10.0*log10(Pt*g2lambda2ctauy/pi2x32) - 2.0*Sa; // http://www.echoview.com/WEBHELP/Reference/Algorithms/Echosounder/Simrad/EK60_Power_to_Sv_and_TS.htm
            if(Sv < -80.0)
            {
                color = gz4d::Color(0.0,0.0,0.0,0.0);
            }
            else if (Sv > -34.0)
            {
                color = gz4d::Color(1.0,1.0,1.0,1.0);
            }
            else 
            {
                float val = (Sv-(-80.0))/(-34.0-(-80.0));
                float r = 0.0;
                float g = 0.0;
                float b = 0.0;
                float a = 0.5+val/2.0;
                if (val > 0.25)
                {
                    r = (val -0.25)*1.3;
                }
                if (val < 0.75)
                {
                    b = 1.0 - (val*1.3);
                }
                g = fabs(val-0.5)*2.0;
                color[0] = r;
                color[1] = g;
                color[2] = b;
                color[3] = a;
            }
        }
        else
        {
            color = gz4d::Color(0.0,0.0,0.0,0.0);
        }
        rasters[rasterNo].GetRow(i)[col] = color;
    }
    expandBBoxTime(t.secondsSinceEpoch().asSeconds());
}

void EK60_Channel::Idle(double time)
{
    displayTime = gz4d::Time(time);

/*    if(socket)
    {
        try
        {
            gz4d::Socket::Packet data(5000,0);
            socket->Recv(data);
    #pragma pack(1)
            struct SampleSetHeader
            {
                gz4d::Time time;
                float z1;
                float sampleInterval;
                float soundSpeed;
            };
    #pragma pack()
            //SampleSetHeader* sh = reinterpret_cast<SampleSetHeader*>(data.data());
    
            //int sampleCount = (data.size()-sizeof(SampleSetHeader))/2;
            //int16_t *samples = reinterpret_cast<int16_t *>(&(data.data()[sizeof(SampleSetHeader)]));
            //AddPing(sh->time,sh->z1,sh->sampleInterval,sh->soundSpeed,sampleCount,samples);
        }
        catch(gz4d::Socket::WouldBlockException)
        {}
    }

    //if (!pings.empty() && displayTime >= pings.begin()->first && displayTime < pings.end()->first+1200.0)
        globalInputMediator->Trigger(Scene3D::sceneChangedEvent());*/
}

int EK60_Channel::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command == "displaySpan")
        {
            if (argc == 2)
            {
                displaySpan = gz4d::time::Duration(argv[1]);
            }
            Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(displaySpan).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command == "fadeDelay")
        {
            if (argc == 2)
            {
                fadeDelay = atof(argv[1]);
            }
            Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(fadeDelay).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command == "color")
        {
            if (argc == 5)
            {
                baseColor[0] = atof(argv[1]);
                baseColor[1] = atof(argv[2]);
                baseColor[2] = atof(argv[3]);
                baseColor[3] = atof(argv[4]);
            }
            Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(baseColor).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command == "showFan")
        {
            if (argc == 2)
            {
                showFan = gz4d::IsTrue(argv[1]);
            }
            Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(showFan).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }
        if (command == "showRibbon")
        {
            if (argc == 2)
            {
                showRibbon = gz4d::IsTrue(argv[1]);
            }
            Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(showRibbon).c_str()), TCL_VOLATILE);
            return TCL_OK;
        }

    }
    return Object::perform( argc, argv, interp);
}

void EK60_Channel::getHtmlInfo(std::stringstream &html, short level)
{
    html << "<h" << level << ">Line</h" << level << ">";

    if (showRibbon)
        html << "Ribbon is on, turn <a href=\"/command/gz/tellObject/" << GetId() << "/showRibbon/off\">off</a><br>";
    else
        html << "Ribbon is off, turn <a href=\"/command/gz/tellObject/" << GetId() << "/showRibbon/on\">on</a><br>";

    if (showFan)
        html << "Fan is on, turn <a href=\"/command/gz/tellObject/" << GetId() << "/showFan/off\">off</a><br>";
    else
        html << "Fan is off, turn <a href=\"/command/gz/tellObject/" << GetId() << "/showFan/on\">on</a><br>";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/color/rValue/gValue/bValue/aValue\">Color r:<input type=\"text\" name=\"rValue\" value=\"" << baseColor[0] <<"\"/> g:<input type=\"text\" name=\"gValue\" value=\"" << baseColor[1] <<"\"/> b:<input type=\"text\" name=\"bValue\" value=\"" << baseColor[2] <<"\"/><input type=\"text\" name=\"aValue\" value=\"" << baseColor[3] <<"\"/><input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/displaySpan/value\">Display time:<input type=\"text\" name=\"value\" value=\"" << displaySpan <<"\"/>Time in seconds channel is visible<input type=\"submit\" value=\"Modify\"/></form><br>";

    html << "<form action=\"/command/gz/tellObject/" << GetId() << "/fadeDelay/value\">Fade delay:<input type=\"text\" name=\"value\" value=\"" << fadeDelay <<"\"/>(0.0 to 1.0) Ratio of Display time to wait before starting fade.<input type=\"submit\" value=\"Modify\"/></form><br>";

    Object::getHtmlInfo(html,level);
}
