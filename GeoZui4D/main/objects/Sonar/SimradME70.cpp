// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "SimradME70.h"
#include "engine/gzCommandInterface.h"
#include "objects/XYZStream.h"
#include "objects/FishSchool.h"

REGISTER_OBJECT(SimradME70, "");

SimradME70::SimradME70()
{
}

Object * SimradME70::LoadXML(bpt::ptree const &xml, std::string const &path, NcFile * ncfile)
{
    ObjectCollection::LoadXML(xml.GetFirstNode(),path, ncfile);

    int port = 37655;
    std::string local_address = "ANY";
    int id=0;

    for(gz4d::xml::Attribute a = xml.GetFirstAttribute(); a; ++a)
    {
        if(a.Name() == "port")
            port = a.Value<int>();

        if(a.Name() == "local_address")
            local_address = a.Value();

        if(a.Name() == "id")
            id = a.Value<int>();
    }

    manager.LaunchSearch(port,local_address);

    gz4d::Simrad::Server s;

    gz4d::Time timeout = gz4d::Time::Now() + gz4d::Time(5.0);
    while(gz4d::Time::Now() < timeout && !s)
    {
        s = manager.Get(id);
        if(s)
            break;
        gz4d::Time::Sleep(.1);
    }

    if(s)
    {
        client = gz4d::Simrad::Client::Ptr(new gz4d::Simrad::Client(s));
        client->Connect();

        for(gz4d::xml::Node n = xml.GetFirstNode(); n; ++n)
        {
            if(n.Name() == "Platform")
            {
                gz4d::xml::Attribute a = n.GetFirstAttribute("target");
                if(a)
                {
                    BaseHandler::Ptr nav(new NavHandler(client->GetPlatform()->GetParameters(),a.Value()));
                    handlers.push_back(nav);
                }
            }
            if(n.Name() == "Channel")
            {
                gz4d::Simrad::Channel::List channels = client->GetTransducer()->GetChannels();
                for (gz4d::Simrad::Channel::List::iterator c = channels.begin(); c != channels.end(); ++c)
                {
                    gz4d::xml::Attribute a = n.GetFirstAttribute("name");
                    if(a)
                    {
                        if (a.Value() == "*" || a.Value() == (*c)->GetName())
                        {
                            for(gz4d::xml::Node sub = n.GetFirstNode(); sub; ++sub)
                            {
                                if(sub.Name() == "BottomDetection")
                                {
                                    gz4d::xml::Attribute a = sub.GetFirstAttribute("target");
                                    if(a)
                                    {
                                        BaseHandler::Ptr b(new BottomHandler(*c,a.Value()));
                                        handlers.push_back(b);
                                    }
                                }
                                if(sub.Name() == "TargetDetection")
                                {
                                    gz4d::xml::Attribute a = sub.GetFirstAttribute("target");
                                    if(a)
                                    {
                                        BaseHandler::Ptr t(new TargetHandler(*c,a.Value()));
                                        handlers.push_back(t);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return this;
}

void SimradME70::Idle( double time)
{
    for(std::vector<BaseHandler::Ptr>::iterator h = handlers.begin(); h != handlers.end(); ++h)
        (*h)->Idle();

    ObjectCollection::Idle( time);
}





SimradME70::BaseHandler::~BaseHandler()
{
/*    for(std::vector<boost::signals::connection>::iterator c = connections.begin(); c != connections.end(); ++c)
        if(c->connected())
            c->disconnect();*/
}

Object *SimradME70::BaseHandler::GetTarget()
{
    Addressable *address;
    Object *ob;
    int ret = gzCommandInterface::findObject(targetObject, address, ob);
    if (ret == TCL_OK)
        return ob;
    return NULL;
}



SimradME70::NavHandler::NavHandler(gz4d::Simrad::ParameterGroup::Map const &p, std::string target)
    :BaseHandler(target),params(p)
{
/*    connections.push_back(params["position"]->connect(boost::bind(&SimradME70::NavHandler::PositionUpdate,this,_1)));
    connections.push_back(params["heading"]->connect(boost::bind(&SimradME70::NavHandler::HeadingUpdate,this,_1)));
    connections.push_back(params["attitude"]->connect(boost::bind(&SimradME70::NavHandler::AttitudeUpdate,this,_1)));*/
}

void SimradME70::NavHandler::Idle()
{
    boost::mutex::scoped_lock lock(positionsMutex);
    if(!positions.empty())
    {
        Object *ob = GetTarget();
        if (ob)
            while(!positions.empty())
            {
                ob->SetLatLon(positions.front().lat,positions.front().lon,positions.front().heading,positions.front().time);
                positions.pop();
            }
    }
}

void SimradME70::NavHandler::PositionUpdate(gz4d::Time t)
{
    Position p;
    p.time = t;
    p.lat = (*params["position"])["lat"]->Get<double>(NAN,t);
    p.lon = (*params["position"])["lon"]->Get<double>(NAN,t);
    p.heading = (*params["heading"])["heading"]->Get<double>(NAN,t);

    if(!(isnan(p.lat) || isnan(p.lon)))
    {
        boost::mutex::scoped_lock lock(positionsMutex);
        positions.push(p);
    }
}

void SimradME70::NavHandler::HeadingUpdate(gz4d::Time t)
{
}

void SimradME70::NavHandler::AttitudeUpdate(gz4d::Time t)
{
}



SimradME70::BottomHandler::BottomHandler(gz4d::Simrad::Channel::Ptr const &c, std::string target)
    :BaseHandler(target)
{
//    connections.push_back(c->GetBottomDetection()->connect(boost::bind(&SimradME70::BottomHandler::operator(),this,_1)));
}


void SimradME70::BottomHandler::Idle()
{
    boost::mutex::scoped_lock lock(targetsMutex);
    if(!targets.empty())
    {
        XYZStream *s = dynamic_cast<XYZStream*>(GetTarget());
        if(s)
        {
            while(!targets.empty())
            {
                //s->AddPoint(targets.front().lat,targets.front().lon,targets.front().targets[0].position,targets.front().time,targets.front().targets[0].value);
                targets.pop();
            }
        }
    }
}

void SimradME70::BottomHandler::operator()(gz4d::Simrad::TargetSet &t)
{
/*    if(!(isnan(t.lat) || isnan(t.lon)))
    {
        boost::mutex::scoped_lock lock(targetsMutex);
        targets.push(t);
    }*/
}



SimradME70::TargetHandler::TargetHandler(gz4d::Simrad::Channel::Ptr const &c, std::string target)
    :BaseHandler(target)
{
//    connections.push_back(c->GetTargetDetection()->connect(boost::bind(&SimradME70::TargetHandler::operator(),this,_1)));
}


void SimradME70::TargetHandler::Idle()
{
    boost::mutex::scoped_lock lock(targetsMutex);
    if(!targets.empty())
    {
        FishSchool *s = dynamic_cast<FishSchool*>(GetTarget());
        if(s)
        {
            while(!targets.empty())
            {
/*                for(std::vector<gz4d::Simrad::Target>::iterator t = targets.front().targets.begin(); t != targets.front().targets.end(); ++t)
                    s->AddFish(targets.front().lat,targets.front().lon,t->position,targets.front().time,t->value);*/
                targets.pop();
            }
        }
    }
}

void SimradME70::TargetHandler::operator()(gz4d::Simrad::TargetSet &t)
{
/*    if(!(isnan(t.lat) || isnan(t.lon)))
    {
        boost::mutex::scoped_lock lock(targetsMutex);
        targets.push(t);
    }*/
}
