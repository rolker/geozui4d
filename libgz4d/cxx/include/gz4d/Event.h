// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#ifndef GZ4D_EVENT_H
#define GZ4D_EVENT_H

#include <gz4d/gz4d.h>
#include <gz4d/time.h>
#include <gz4d/time/ios.h>
#include <vector>
#include <sstream>
#include <memory>

namespace gz4d
{
    /// User interface events.
    /// Designed to be independent of windowing system, and easy to use even across languages (easily swigable)

    struct Event
    {
        enum Type 
        {
            Null = 0,
            Axis,
            CaptureMouse,
            Draw,
            Idle,
            Key,
            LED,
            Mouse,
            MouseButton,
            MouseWheel,
            Position,
            ReleaseMouse,
            Rumble,
            Size
        };

        gz4d::time::Point timestamp;
        Type type;
        int id;
        float x,y;
        float value;

        Event()
            :timestamp(gz4d::time::Now()),type(Null),id(0),x(0.0),y(0.0),value(0.0){}

        Event(Event const &e)
            :timestamp(e.timestamp),type(e.type),id(e.id),x(e.x),y(e.y),value(e.value){}

        Event(Type type)
            :timestamp(gz4d::time::Now()),type(type),id(0),x(0.0),y(0.0),value(0.0){}
        Event(Type type, int id)
            :timestamp(gz4d::time::Now()),type(type),id(id),x(0.0),y(0.0),value(0.0){}
        Event(Type type, int id, float value)
            :timestamp(gz4d::time::Now()),type(type),id(id),x(0.0),y(0.0),value(value){}
        Event(Type type, int id, float x, float y, float value)
            :timestamp(gz4d::time::Now()),type(type),id(id),x(x),y(y),value(value){}
        Event(Type type, int id, float x, float y)
            :timestamp(gz4d::time::Now()),type(type),id(id),x(x),y(y),value(0.0){}

        bool operator==(Event const &e) const
        {
            return timestamp == e.timestamp && type == e.type && id == e.id && x == e.x && y == e.y && value == e.value;
        }

        std::string String()
        {
            std::stringstream ret;
            ret << "Event:\t";// << timestamp << "\t";
            switch(type)
            {
                case Null:
                    ret << "Null";
                    break;
                case Draw:
                    ret << "Draw";
                    break;
                case Axis:
                    ret << "Axis";
                    break;
                case MouseButton:
                    ret << "MouseButton";
                    break;
                default:
                    ret << "Unknown";
            }
            ret << "\tid: " << id << " x, y, value: " << x << ", " << y << ", " << value;
            return ret.str();
        }
    };

    class EventGroup: public std::map<Event::Type,std::vector<Event> >
    {
        public:
            void Add(EventGroup const &eg)
            {
                for (EventGroup::const_iterator i = eg.begin(); i != eg.end(); ++i)
                {
                    if(!i->second.empty())
                        operator[](i->first).insert(operator[](i->first).end(),i->second.begin(),i->second.end());
                }
                insert(eg.begin(),eg.end());
            }
            void Add(Event const &e)
            {
                operator[](e.type).push_back(e);
            }
    };

    class EventHandler
    {
        public:
            typedef std::shared_ptr<EventHandler> Ptr;

            virtual EventGroup HandleEvent(Event const &e)=0;
    };
}

#endif
