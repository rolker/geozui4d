#ifndef SIMRADME70_H
#define SIMRADME70_H

#include "engine/ObjectCollection.h"
#include <gz4d/sonar/Simrad/Client.h>
#include <gz4d/sonar/Simrad/ServerManager.h>
#include <gz4d/sonar/Simrad/Navigation.h>
#include <gz4d/sonar/Simrad/BottomDetection.h>

class SimradME70: public ObjectCollection
{
    gz4d::Simrad::ServerManager manager;
    gz4d::Simrad::Client::Ptr client;

    class BaseHandler
    {
        public:
            typedef std::shared_ptr<BaseHandler> Ptr;
            BaseHandler(std::string target):targetObject(target){}
            ~BaseHandler();
            virtual void Idle()=0;
        protected:
            std::string targetObject;
//            std::vector<boost::signals::connection> connections;
            Object *GetTarget();
    };

    std::vector<BaseHandler::Ptr> handlers;

    class NavHandler: public BaseHandler
    {
        gz4d::Simrad::ParameterGroup::Map params;

        struct Position
        {
            double lat,lon,heading,time;
        };
        std::queue<Position> positions;
        boost::mutex positionsMutex;

        public:
            typedef std::shared_ptr<NavHandler> Ptr;

            NavHandler(gz4d::Simrad::ParameterGroup::Map const &p, std::string target);

            void Idle();
            void PositionUpdate(gz4d::Time t);
            void HeadingUpdate(gz4d::Time t);
            void AttitudeUpdate(gz4d::Time t);
    };

    class BottomHandler: public BaseHandler
    {
        std::queue<gz4d::Simrad::TargetSet> targets;
        boost::mutex targetsMutex;
        public:
            typedef std::shared_ptr<BottomHandler> Ptr;

            BottomHandler(gz4d::Simrad::Channel::Ptr const &c, std::string target);

            void Idle();
            void operator()(gz4d::Simrad::TargetSet &t);
    };

    class TargetHandler: public BaseHandler
    {
        std::queue<gz4d::Simrad::TargetSet> targets;
        boost::mutex targetsMutex;
        public:
            typedef std::shared_ptr<TargetHandler> Ptr;

            TargetHandler(gz4d::Simrad::Channel::Ptr const &c, std::string target);

            void Idle();
            void operator()(gz4d::Simrad::TargetSet &t);
    };

    public:
        SimradME70();
        DERIVE_OBJECT(BeamPattern);

        void Idle(double time);
        Object * LoadXML(bpt::ptree const &xml, std::string const &path, NcFile * ncfile);


};

#endif
