#include "../boost_python.h"
#include <gz4d/Pixel.h>

using namespace boost::python;

template<typename T> void WrapPixel(char const *name)
{
    class_<gz4d::Pixel<T>, boost::python::bases<typename T::container_type> >(name)
        .def(init<>())
        .def(init<gz4d::Color>())
        .add_property("color", &gz4d::Pixel<T>::operator Color)
        .def(init<typename T::container_type const &>())
    ;
}

BOOST_PYTHON_MODULE(Pixel)
{
    WrapPixel<gz4d::RGB8>("PixelRGB8");
    WrapPixel<gz4d::BGR8>("PixelBGR8");
    WrapPixel<gz4d::RGBA8>("PixelRGBA8");
    WrapPixel<gz4d::YUV8>("PixelYUV8");
    WrapPixel<gz4d::L8>("PixelL8");
}
