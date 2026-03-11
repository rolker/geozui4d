// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../../boost_python.h"
#include <gz4d/data/NMEA.h>
#include <gz4d/thread/Producer.h>

using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(parse_overloads,Parse,1,2)

std::string const & __getitem__(gz4d::nmea::Sentence & s, int i)
{
    if (i<0)
        i += s.size();
    if (i<0 || i >= s.size())
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }
    return s[i];
}

void __setitem__(gz4d::nmea::Sentence & s, int i, std::string const & value)
{
    if (i<0)
        i += s.size();
    if (i<0 || i >= s.size())
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }
    s[i] = value;
}

BOOST_PYTHON_MODULE(Sentence)
{
    class_<gz4d::nmea::Sentence>("Sentence", init<std::string const&, optional<bool> >())
            .def(init<>())
            .def("__str__",&gz4d::nmea::Sentence::operator())
            .def("Parse",&gz4d::nmea::Sentence::Parse,parse_overloads())
            .def("IsType",&gz4d::nmea::Sentence::IsType)
            .def("__len__",&gz4d::nmea::Sentence::size)
            .def("__getitem__",__getitem__,return_value_policy<copy_const_reference>())
            .def("__setitem__",__setitem__)
            .def("resize",&gz4d::nmea::Sentence::resize)
            ;

    gz4d::WrapProducer<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >("TimeSentenceProducer");
    gz4d::WrapThreadedQueue<std::pair<gz4d::Time,gz4d::nmea::Sentence::Ptr> >("TimeSentenceThreadedQueue");
}
