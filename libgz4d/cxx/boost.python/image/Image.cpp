#define GZ4D_BOOST_PYTHON
#include <gz4d/Image.h>
#include <gz4d/image/io.h>
#include "../boost_python.h"

using namespace boost::python;

template <typename T, typename OT> OT as(T &i)
{
    return OT(typename OT::raster_type(i.GetRaster()));
}

template <typename T> boost::python::str tostr(T &i)
{
    boost::python::str ret;
    int s = sizeof(typename T::pixel_type);
    int w = i.width();
    for(int r = i.height()-1; r >= 0; --r)
        ret += boost::python::str(&(reinterpret_cast<char *>(i.raw())[r*w*s]),w*s);
    return ret;
    //return boost::python::str(reinterpret_cast<char *>(i.raw()),i.raw_size());
}


template<typename T> inline void resize(T &i, std::size_t width, std::size_t height)
{
    i.Resize(width,height);
}

template<typename T> inline void WrapImage(char const *name)
{
//     gz4d::Raster<typename T::pixel_type> &(T::*GetRaster)() = &T::GetRaster;
//     gz4d::Raster<typename T::pixel_type> const &(T::*GetRasterC)() const = &T::GetRaster;

    class_<T>c(name,init<>());
        c.def(init<std::shared_ptr<gz4d::Raster<typename T::pixel_type> > &>());
        c.def("Load",&T::Load);
        c.def("Save",&T::Save);
        c.def("GetRaster",&T::GetRasterPtr,return_value_policy<copy_non_const_reference>());
        c.def("Resize",resize<T>);
        c.add_property("width",&T::width);
        c.add_property("height",&T::height);
        c.def("asImageRGB8",as<T,gz4d::ImageRGB8>);
        c.def("asImageBGR8",as<T,gz4d::ImageBGR8>);
        c.def("asImageL8",as<T,gz4d::ImageL8>);
        c.def("tostring",tostr<T>);
        
/*        .def("GetRaster",GetRaster,return_value_policy<copy_non_const_reference>())
        .def("GetRaster",GetRasterC,return_value_policy<copy_const_reference>())*/
    ;
}


BOOST_PYTHON_MODULE(Image)
{
    WrapImage<gz4d::ImageRGBA8>("ImageRGBA8");
    WrapImage<gz4d::ImageRGB8>("ImageRGB8");
    WrapImage<gz4d::ImageBGR8>("ImageBGR8");
    WrapImage<gz4d::ImageYUV8>("ImageYUV8");
    WrapImage<gz4d::ImageL8>("ImageL8");
}
