// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "EK60.h"
#include "EK60_Channel.h"
#include "engine/Scene3D.h"
#include <gz4d/data/NMEA.h>
#include "engine/InputMediator.h"
#include "MainWindow.h"
#include <gz4d/geo/Projection.h>

extern MainWindow * mainWindow;

REGISTER_OBJECT(EK60, "raw");


EK60::EK60()
{
    binaryFile = true;
    me70 = false;
}

EK60::~EK60()
{
}

// void EK60::DrawLowRes(float degradation, Window3D *win)
// {
//     glPushAttrib(GL_ALL_ATTRIB_BITS);
//     glDisable(GL_LIGHTING);
//     glColor3f(1.0,1.0,1.0);
//     glBegin(GL_LINE_STRIP);
//     for(std::map<gz4d::Time,Position>::iterator p = positions.begin(); p != positions.end(); ++p)
//     {
//         glVertex3f(p->second.x,p->second.y,0.0);
//     }
//     glEnd();
//     glPopAttrib();
//     ObjectCollection::DrawLowRes(degradation, win);
// }

bool EK60::GetPosition(gz4d::Time t, float &x, float &y)
{
    std::map<gz4d::Time,Position>::iterator start = positions.upper_bound(t);
    if (start != positions.end() && start != positions.begin())
    {
        --start;
        x = start->second.x;
        y = start->second.y;

        std::map<gz4d::Time,Position>::iterator end = positions.lower_bound(t);
        if(end != positions.end() && end != start)
        {
            double dt = (end->first - start->first).asSeconds();
            double r = ((end->first - t)/dt).asSeconds();
            x = r*start->second.x + (1.0-r)*end->second.x;
            y = r*start->second.y + (1.0-r)*end->second.y;
        }
        return true;
    }
    return false;
}

bool EK60::GetHeading(gz4d::Time t, float &h)
{
    std::map<gz4d::Time,float>::iterator start = headings.upper_bound(t);
    if (start != headings.end() && start != headings.begin())
    {
        --start;
        h = start->second;

        std::map<gz4d::Time,float>::iterator end = headings.lower_bound(t);
        if(end != headings.end() && end != start)
        {
            double dt = (end->first - start->first).asSeconds();
            double r = ((end->first - t)/dt).asSeconds();
            h = r*start->second + (1.0-r)*end->second;
        }
        return true;
    }
    return false;
}


int EK60::Load()
{
    int ret = Open();
    if (!ret)
    {
        while(ReadPacket())
        {}
    }
    //SetLabel( "ek60");
    return ret;
}

bool EK60::ReadPacket()
{
    uint32_t packetSize;
    if(fread(&packetSize,4,1,in)<1)
        return false;
    if (packetData.size() < packetSize)
        packetData.resize(packetSize);

    if(fread(&(packetData.front()),packetSize,1,in) < 1)
        return false;

    uint32_t checkSize;
    if (fread(&checkSize,4,1,in)<1)
        return false;

    if(checkSize != packetSize)
    {
        std::cerr << "EK60::ReadPacket Datagram size error" << std::endl;
        return false;
    }

    gz4d::Simrad::Datagram *header = reinterpret_cast<gz4d::Simrad::Datagram*>(&packetData.front());

    switch (header->DgHeader.DatagramType)
    {
        case 0x304e4f43:
        {
            if(channels.size() == 0)
            {
                gz4d::Simrad::ConfigurationDatagram * config = reinterpret_cast<gz4d::Simrad::ConfigurationDatagram*>(&packetData.front());
                std::cerr << config->ConfigHeader.SounderName << std::endl;
                if (std::string(config->ConfigHeader.SounderName) == "MBES")
                {
                    me70 = true;
                }
                SetLabel(config->ConfigHeader.SounderName);
                for(int32_t i = 0; i < config->ConfigHeader.TransducerCount; ++i)
                {
                    EK60_Channel * c = new EK60_Channel(this,config->Beam[i]);
                    channels.push_back(c);
                    AddObject(c);
                }
            }
            return true;
        }
        case 0x30574152:
        {
            gz4d::Simrad::SampleDatagram * sample = reinterpret_cast<gz4d::Simrad::SampleDatagram*>(&packetData.front());
            int c = sample->Channel-1;
            channels[c]->AddPing(*sample);
            return true;
        }
        case 0x30454d4e:
        {
            gz4d::Simrad::NMEADatagram * nmea = reinterpret_cast<gz4d::Simrad::NMEADatagram*>(&packetData.front());

            gz4d::nmea::Sentence n(nmea->Text,false);
            if (n.IsType("??GGA"))
            {
                gz4d::nmea::GGA gga(n);
                AddPosition(gz4d::time::FromNT(nmea->DgHeader.DateTime),gga.latitude,gga.longitude);
            }

            if (n.IsType("??HDT"))
            {
                gz4d::nmea::HDT hdt(n);
                AddHeading(gz4d::time::FromNT(nmea->DgHeader.DateTime),hdt.heading);
            }

            return true;
        }
        default:
            std::cerr << "EK60::ReadPacket - skipping packet type " << header->DgHeader.DatagramType << std::endl;
    }
    return true;
}

int EK60::perform(int argc, const char **argv, Tcl_Interp *interp)
{
    if (argc >= 1)
    {
        std::string command(argv[0]);
        if (command == "addChannel")
        {
            if (argc == 2)
            {
                EK60_Channel * c = new EK60_Channel(this,argv[1]);
                channels.push_back(c);
                AddObject(c);
                Tcl_SetResult(interp, const_cast<char *>(boost::lexical_cast<std::string>(c->GetId()).c_str()), TCL_VOLATILE);
                return TCL_OK;
            }
            Tcl_SetResult(interp, const_cast<char*>("usage: addChannel name"), TCL_VOLATILE);
            return TCL_ERROR;
        }
        if (command == "addPosition")
        {
            if (argc == 4)
            {
                double t = atof(argv[1]);
                double lon = atof(argv[2]);
                double lat = atof(argv[3]);
                AddPosition(gz4d::Time(t),lat,lon);
                currentTime = t;
                globalInputMediator->Trigger(Scene3D::objectChangedEvent());
                return TCL_OK;
            }
            Tcl_SetResult(interp, const_cast<char*>("usage: addPosition time lon lat"), TCL_VOLATILE);
            return TCL_ERROR;
        }
    }
    return Object::perform( argc, argv, interp);
}

void EK60::AddPosition(gz4d::Time t, double lat, double lon)
{
    autoProjection(lon,lat);
    gz4d::geo::Projection proj(spatialReference, mainWindow->getScene3D()->getSpatialReference());
    gz4d::Point<double> p = proj(lat,lon);
    if (positions.empty())
    {
        baseX = p[0];
        baseY = p[1];
        SetPosition(p);
    }
    Position pos;
    pos.x = p[0]-baseX;
    pos.y = p[1]-baseY;
    positions[t] = pos;
    expandBBoxTime(t.secondsSinceEpoch().asSeconds());
}

void EK60::AddHeading(gz4d::Time t, float h)
{
    headings[t] = h;
}
