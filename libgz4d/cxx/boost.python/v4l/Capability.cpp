#include "../boost_python.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <gz4d/v4l/Capability.h>
#include <gz4d/v4l/Device.h>

using namespace boost::python;

BOOST_PYTHON_MODULE(Capability)
{
    class_<gz4d::v4l::Capability::Format>("Format")
        .def_readwrite("description", &gz4d::v4l::Capability::Format::description)
        .def_readwrite("format", &gz4d::v4l::Capability::Format::format)
        .def_readwrite("compressed", &gz4d::v4l::Capability::Format::compressed)
    ;

    class_<std::vector<gz4d::v4l::Capability::Format> >("VectorFormat")
    .def(vector_indexing_suite<std::vector<gz4d::v4l::Capability::Format> >())
    ;

    class_<gz4d::v4l::Capability>("Capability",init<gz4d::v4l::Device const &>())
        .def("getDriver", &gz4d::v4l::Capability::getDriver, return_value_policy<copy_const_reference>())
        .def("getCard", &gz4d::v4l::Capability::getCard, return_value_policy<copy_const_reference>())
        .def("getBusInfo", &gz4d::v4l::Capability::getBusInfo, return_value_policy<copy_const_reference>())
        .def("isVideoCapture", &gz4d::v4l::Capability::isVideoCapture)
        .def("isReadWrite", &gz4d::v4l::Capability::isReadWrite)
        .def("isStreaming", &gz4d::v4l::Capability::isStreaming)
        .def("getBounds", &gz4d::v4l::Capability::getBounds, return_value_policy<copy_const_reference>())
        .def("getDefaultBounds", &gz4d::v4l::Capability::getDefaultBounds, return_value_policy<copy_const_reference>())
        .def("getPixelAspect", &gz4d::v4l::Capability::getPixelAspect)
        .def("getFormats", &gz4d::v4l::Capability::getFormats, return_value_policy<copy_const_reference>())
    ;
}
