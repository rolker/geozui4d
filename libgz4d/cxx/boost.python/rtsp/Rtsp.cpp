#include "../boost_python.h"
#include <gz4d/rtsp/Rtsp.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Rtsp)
{
    class_<gz4d::rtsp::Rtsp, bases<>, gz4d::rtsp::Rtsp::Ptr >("Rtsp",init<std::string const &, std::string const &, std::string const &>())
        .def("save",&gz4d::rtsp::Rtsp::save)
        .def("stop",&gz4d::rtsp::Rtsp::stop)
        .add_property("bufferSize",&gz4d::rtsp::Rtsp::getBufferSize,&gz4d::rtsp::Rtsp::setBufferSize)
    ;
}
