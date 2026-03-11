// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0

#include "../boost_python.h"
#include <gz4d/data/Raster.h>
#include <gz4d/Pixel.h>

using namespace boost::python;

template <typename T> inline typename T::value_type const &__getitem__(T &r, tuple t)
{
    int i = extract<int>(t[0]);
    int j = extract<int>(t[1]);
    
    if (i<0)
        i += r.GetColumns();
    if (i<0 || i >= r.GetColumns())
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }
    
    if (j<0)
        j += r.GetRows();
    if (j<0 || j >= r.GetRows())
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }
    
    return r.GetRow(j)[i];
}

template <typename T> inline void __setitem__(T &r, tuple t, typename T::value_type const & value)
{
    int i = extract<int>(t[0]);
    int j = extract<int>(t[1]);

    if (i<0)
        i += r.GetColumns();
    if (i<0 || i >= r.GetColumns())
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }

    if (j<0)
        j += r.GetRows();
    if (j<0 || j >= r.GetRows())
    {
        PyErr_SetString( PyExc_IndexError, "Index out of range" );
        throw_error_already_set();
    }

    r.GetRow(j)[i] = value;
}



template<typename T> inline void WrapRaster(char const *name)
{
    class_<T, bases<>, std::shared_ptr<T>, boost::noncopyable>(name,init<>())
            .def(init<std::size_t, std::size_t, typename T::value_type const &>())
            .def("GetColumns",&T::GetColumns)
            .def("GetRows",&T::GetRows)
            .add_property("columns",&T::GetColumns)
            .add_property("rows",&T::GetRows)
            .def("__getitem__", __getitem__<T>, return_value_policy<copy_const_reference>() )
            .def("__setitem__", __setitem__<T>)
    ;
}


BOOST_PYTHON_MODULE(Raster)
{
    WrapRaster<gz4d::Raster<gz4d::Pixel<gz4d::RGBA8> > >("RasterPixelRGBA8");
    WrapRaster<gz4d::Raster<gz4d::Pixel<gz4d::RGB8> > >("RasterPixelRGB8");
    WrapRaster<gz4d::Raster<gz4d::Pixel<gz4d::BGR8> > >("RasterPixelBGR8");
    WrapRaster<gz4d::Raster<gz4d::Pixel<gz4d::YUV8> > >("RasterPixelYUV8");
    WrapRaster<gz4d::Raster<gz4d::Pixel<gz4d::L8> > >("RasterPixelL8");
    WrapRaster<gz4d::Raster<gz4d::Color> >("RasterColor");
}
