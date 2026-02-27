#include "../boost_python.h"
#include <gz4d/v4l/Tiler.h>

std::size_t (gz4d::v4l::Tiler::*width0)() const = &gz4d::v4l::Tiler::width;
std::size_t (gz4d::v4l::Tiler::*width1)(std::size_t) = &gz4d::v4l::Tiler::width;

std::size_t (gz4d::v4l::Tiler::*height0)() const = &gz4d::v4l::Tiler::height;
std::size_t (gz4d::v4l::Tiler::*height1)(std::size_t) = &gz4d::v4l::Tiler::height;

std::size_t (gz4d::v4l::Tiler::*innerBorder0)() const = &gz4d::v4l::Tiler::innerBorder;
void (gz4d::v4l::Tiler::*innerBorder1)(std::size_t) = &gz4d::v4l::Tiler::innerBorder;

std::size_t (gz4d::v4l::Tiler::*outerBorder0)() const = &gz4d::v4l::Tiler::outerBorder;
void (gz4d::v4l::Tiler::*outerBorder1)(std::size_t) = &gz4d::v4l::Tiler::outerBorder;

gz4d::Color const &(gz4d::v4l::Tiler::*backgroundColor0c)() const = &gz4d::v4l::Tiler::backgroundColor;
gz4d::Color &(gz4d::v4l::Tiler::*backgroundColor0)() = &gz4d::v4l::Tiler::backgroundColor;
gz4d::Color const &(gz4d::v4l::Tiler::*backgroundColor1)(gz4d::Color const &) = &gz4d::v4l::Tiler::backgroundColor;

using namespace boost::python;

BOOST_PYTHON_MODULE(Tiler)
{
    class_<gz4d::v4l::Tiler>("Tiler")
        .def("add", &gz4d::v4l::Tiler::add)
        .def("get", &gz4d::v4l::Tiler::get)
        .def("remove", &gz4d::v4l::Tiler::remove)

        .def("fixedWidth", &gz4d::v4l::Tiler::fixedWidth)
        .def("width", width0)
        .def("width", width1)

        .def("fixedHeight", &gz4d::v4l::Tiler::fixedHeight)
        .def("height", height0)
        .def("height", height1)

        .def("rows", &gz4d::v4l::Tiler::rows)
        .def("columns", &gz4d::v4l::Tiler::columns)
        .def("empty", &gz4d::v4l::Tiler::empty)

        .def("innerBorder", innerBorder0)
        .def("innerBorder", innerBorder1)

        .def("outerBorder", outerBorder0)
        .def("outerBorder", outerBorder1)

        .def("backgroundColor", backgroundColor0c, return_value_policy<copy_const_reference>())
        .def("backgroundColor", backgroundColor0, return_value_policy<copy_non_const_reference>())
        .def("backgroundColor", backgroundColor1, return_value_policy<copy_const_reference>())

        .def("framePeriod", &gz4d::v4l::Tiler::framePeriod)
        .def("Read", &gz4d::v4l::Tiler::Read)
    ;
}


