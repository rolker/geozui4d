// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/Event.h>

using namespace boost::python;

struct EventHandlerWrap: gz4d::EventHandler, wrapper<gz4d::EventHandler>
{
    gz4d::EventGroup HandleEvent(gz4d::Event const &e)
    {
        return this->get_override("HandleEvent")(e);
    }
};

BOOST_PYTHON_MODULE(Event)
{
    enum_<gz4d::Event::Type>("EventType")
            .value("Null", gz4d::Event::Null)
            .value("Axis", gz4d::Event::Axis)
            .value("CaptureMouse", gz4d::Event::CaptureMouse)
            .value("Draw", gz4d::Event::Draw)
            .value("Idle", gz4d::Event::Idle)
            .value("Key", gz4d::Event::Key)
            .value("LED", gz4d::Event::LED)
            .value("Mouse", gz4d::Event::Mouse)
            .value("MouseButton", gz4d::Event::MouseButton)
            .value("MouseWheel", gz4d::Event::MouseWheel)
            .value("Position", gz4d::Event::Position)
            .value("ReleaseMouse", gz4d::Event::ReleaseMouse)
            .value("Rumble", gz4d::Event::Rumble)
            .value("Size", gz4d::Event::Size)
            ;

    class_<gz4d::Event>("Event")
            .def(init<gz4d::Event::Type>())
            .def(init<gz4d::Event::Type, int>())
            .def(init<gz4d::Event::Type, int, float>())
            .def(init<gz4d::Event::Type, int, float, float>())
            .def(init<gz4d::Event::Type, int, float, float, float>())
            .def_readwrite("timestamp", &gz4d::Event::timestamp)
            .def_readwrite("type", &gz4d::Event::type)
            .def_readwrite("id", &gz4d::Event::id)
            .def_readwrite("x", &gz4d::Event::x)
            .def_readwrite("y", &gz4d::Event::y)
            .def_readwrite("value", &gz4d::Event::value)
            .def("__str__",&gz4d::Event::String)
            ;

    class_<EventHandlerWrap, boost::noncopyable>("EventHandler")
            .def("HandleEvent", pure_virtual(&gz4d::EventHandler::HandleEvent))
            ;

    class_<std::vector<gz4d::Event> >("EventVector")
            .def(vector_indexing_suite<std::vector<gz4d::Event> >())
            ;

    class_<std::map<gz4d::Event::Type, std::vector<gz4d::Event> > >("EventVectorMap")
            .def(map_indexing_suite<std::map<gz4d::Event::Type, std::vector<gz4d::Event> > >())
            ;

    void (gz4d::EventGroup::*AddE)(gz4d::Event const &) = &gz4d::EventGroup::Add;
    void (gz4d::EventGroup::*AddEG)(gz4d::EventGroup const &) = &gz4d::EventGroup::Add;

    class_<gz4d::EventGroup, bases<std::map<gz4d::Event::Type, std::vector<gz4d::Event> > > >("EventGroup")
            .def("Add",AddE)
            .def("Add",AddEG)
            ;
}
